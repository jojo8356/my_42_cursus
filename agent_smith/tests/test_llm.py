"""Tests for the LLM layer: scripted provider, registry, key rotation, fallback."""

from __future__ import annotations

import json

import pytest

from llm import (
    Completion,
    EchoProvider,
    ModelEntry,
    OpenAICompatibleProvider,
    ProviderError,
    ProviderRegistry,
    ScriptedProvider,
    TokenRotator,
    call_with_rotation,
    estimate_tokens,
    normalize_url,
)


# -- ScriptedProvider ------------------------------------------------------
def test_scripted_provider_replays_the_script():
    provider = ScriptedProvider(["one", "two"])
    assert provider.complete([{"role": "user", "content": "x"}]).text == "one"
    assert provider.complete([{"role": "user", "content": "x"}]).text == "two"


def test_scripted_provider_exhaustion_is_an_error():
    provider = ScriptedProvider(["only"])
    provider.complete([{"role": "user", "content": "x"}])
    with pytest.raises(ProviderError, match="exhausted"):
        provider.complete([{"role": "user", "content": "x"}])


def test_scripted_provider_can_repeat_last():
    provider = ScriptedProvider(["only"], repeat_last=True)
    provider.complete([{"role": "user", "content": "x"}])
    assert provider.complete([{"role": "user", "content": "x"}]).text == "only"


def test_scripted_provider_simulates_failures():
    provider = ScriptedProvider([{"raise": "boom", "status": 500}])
    with pytest.raises(ProviderError) as captured:
        provider.complete([{"role": "user", "content": "x"}])
    assert captured.value.status == 500
    assert captured.value.should_fallback


def test_scripted_provider_records_the_conversation():
    provider = ScriptedProvider(["ok"])
    provider.complete([{"role": "system", "content": "s"}, {"role": "user", "content": "u"}])
    assert provider.calls[0][0]["content"] == "s"


def test_scripted_provider_counts_tokens():
    provider = ScriptedProvider(["a" * 400])
    completion = provider.complete([{"role": "user", "content": "b" * 400}])
    assert completion.input_tokens > 0
    assert completion.output_tokens > 0


def test_scripted_provider_exhausted_property():
    provider = ScriptedProvider([])
    assert provider.exhausted


# -- EchoProvider ----------------------------------------------------------
def test_echo_provider_submits_an_answer():
    completion = EchoProvider().complete([{"role": "user", "content": "hello"}])
    assert "final_answer" in completion.text


# -- normalize_url ---------------------------------------------------------
def test_normalize_url_appends_suffix():
    assert normalize_url("https://api.example.com/v1").endswith("/chat/completions")


def test_normalize_url_keeps_full_url():
    url = "https://api.example.com/v1/chat/completions"
    assert normalize_url(url) == url


def test_normalize_url_rejects_empty():
    with pytest.raises(ValueError):
        normalize_url("")


def test_estimate_tokens_is_monotonic():
    assert estimate_tokens("") == 0
    assert estimate_tokens("abcd") >= 1
    assert estimate_tokens("a" * 400) > estimate_tokens("a" * 40)


# -- TokenRotator ----------------------------------------------------------
def test_rotator_cycles_and_stops():
    rotator = TokenRotator(["k1", "k2"])
    assert rotator.current() == "k1"
    assert rotator.rotate() is True
    assert rotator.current() == "k2"
    assert rotator.rotate() is False


def test_rotator_ignores_empty_keys():
    assert len(TokenRotator(["", "k2", ""])) == 1


def test_rotator_reset():
    rotator = TokenRotator(["a", "b"])
    rotator.rotate()
    rotator.reset()
    assert rotator.current() == "a"


# -- call_with_rotation ----------------------------------------------------
class FlakyProvider:
    """Fails with 429 ``failures`` times, then succeeds."""

    def __init__(self, failures: int) -> None:
        self.failures = failures
        self.keys = []

    def set_key(self, key: str) -> None:
        self.keys.append(key)

    def complete(self, messages, *, stop=None, max_tokens=None):
        if self.failures:
            self.failures -= 1
            raise ProviderError("rate limited", status=429)
        return Completion(text="ok")


def test_rotation_switches_key_on_429():
    provider = FlakyProvider(failures=1)
    completion, retries = call_with_rotation(provider, TokenRotator(["k1", "k2"]),
                                             [{"role": "user", "content": "x"}])
    assert completion.text == "ok"
    assert retries == 1
    assert provider.keys == ["k2"]


def test_rotation_gives_up_when_no_key_left():
    provider = FlakyProvider(failures=5)
    with pytest.raises(ProviderError):
        call_with_rotation(provider, TokenRotator(["only"]),
                           [{"role": "user", "content": "x"}])


def test_non_429_is_not_rotated():
    provider = FlakyProvider(failures=1)
    with pytest.raises(ProviderError):
        call_with_rotation(provider, None, [{"role": "user", "content": "x"}])


# -- ProviderRegistry ------------------------------------------------------
MODELS = {
    "providers": {
        "groq": {"url": "https://groq.example/v1", "models": ["a", "b"]},
        "gemini": {"url": "https://gemini.example/v1", "models": ["c"]},
    }
}


def test_registry_from_dict_lists_models():
    registry = ProviderRegistry.from_dict(MODELS, env={})
    assert registry.names() == ["a", "b", "c"]


def test_registry_orders_by_strength():
    registry = ProviderRegistry.from_dict(MODELS, env={})
    assert [entry.model for entry in registry.ordered()][0] == "a"


def test_registry_rejects_provider_without_models():
    with pytest.raises(ValueError, match="no model"):
        ProviderRegistry.from_dict({"providers": {"p": {"url": "u", "models": []}}}, env={})


def test_registry_rejects_duplicate_model():
    data = {"providers": {"p1": {"url": "u1", "models": ["m"]},
                          "p2": {"url": "u2", "models": ["m"]}}}
    with pytest.raises(ValueError, match="served twice"):
        ProviderRegistry.from_dict(data, env={})


def test_registry_rejects_non_object_providers():
    with pytest.raises(ValueError, match="providers"):
        ProviderRegistry.from_dict({"providers": []}, env={})


def test_registry_reads_plural_keys_first():
    registry = ProviderRegistry.from_dict(MODELS, env={"GROQ_API_KEYS": "k1,k2"})
    assert registry._keys_for("GROQ_API_KEY") == ["k1", "k2"]


def test_registry_reads_singular_key():
    registry = ProviderRegistry.from_dict(MODELS, env={"GROQ_API_KEY": "solo"})
    assert registry._keys_for("GROQ_API_KEY") == ["solo"]


def test_default_model_prefers_a_provider_with_a_key():
    registry = ProviderRegistry.from_dict(MODELS, env={"GEMINI_API_KEY": "k"})
    assert registry.default_model() == "c"


def test_default_model_falls_back_to_the_first():
    registry = ProviderRegistry.from_dict(MODELS, env={})
    assert registry.default_model() == "a"


def test_unknown_model_raises():
    registry = ProviderRegistry.from_dict(MODELS, env={})
    with pytest.raises(ProviderError, match="unknown model"):
        registry.get("nope")


def test_registry_factory_is_used_lazily():
    created = []

    def factory(entry):
        created.append(entry.model)
        return ScriptedProvider(["x"], model=entry.model)

    registry = ProviderRegistry.from_dict(MODELS, env={}, factory=factory)
    assert created == []
    registry.get("a")
    assert created == ["a"]
    registry.get("a")  # cached
    assert created == ["a"]


def test_registry_register_attaches_an_instance():
    registry = ProviderRegistry([])
    provider = ScriptedProvider(["hi"], model="offline")
    registry.register("offline", provider)
    assert registry.names() == ["offline"]
    assert registry.get("offline") is provider


def test_openai_provider_endpoint(monkeypatch):
    provider = OpenAICompatibleProvider("https://api.example.com/v1", "m", api_key="k")
    assert provider.url == "https://api.example.com/v1/chat/completions"


def test_openai_provider_reads_the_environment(monkeypatch):
    monkeypatch.setenv("MY_KEY", "from-env")
    provider = OpenAICompatibleProvider("https://x/v1", "m", key_variable="MY_KEY")
    assert provider.api_key == "from-env"


def test_provider_error_should_fallback():
    assert ProviderError("x", status=503).should_fallback
    assert ProviderError("x", status=429).should_fallback
    assert not ProviderError("x", status=401).should_fallback
    assert ProviderError("x", retryable=True).should_fallback


def test_completion_to_dict():
    data = Completion("t", 1, 2, 3.0, "m", "u").to_dict()
    assert data["text"] == "t"
    assert data["input_tokens"] == 1
