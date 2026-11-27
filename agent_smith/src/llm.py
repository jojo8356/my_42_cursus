"""LLM providers, key rotation and model fallback.

Three provider implementations share one interface (``complete``):

  * :class:`ScriptedProvider` — deterministic, offline, no API key. It replays a
    list of responses, which makes the whole agent loop testable and reproducible
    (and is the reason the test suite never needs the network).
  * :class:`OpenAICompatibleProvider` — a ``/chat/completions`` client built on
    ``urllib`` so the core stays dependency-free.
  * :class:`EchoProvider` — a tiny last-resort provider used by demos.

Key rotation: several comma-separated keys may be configured for one provider; on
HTTP 429 the next key is used. When a provider is exhausted, the loop falls back to
the next model of the registry, strongest first.
"""

from __future__ import annotations

import json
import os
import time
import urllib.error
import urllib.request
from dataclasses import dataclass, field
from typing import Any, Callable, Dict, List, Optional, Sequence

# Status codes that mean "try the next model".
FALLBACK_STATUSES = frozenset({404, 408, 413, 429, 500, 502, 503, 504})


class ProviderError(RuntimeError):
    """A provider failed in a way the loop may recover from."""

    def __init__(self, message: str, status: Optional[int] = None,
                 retryable: bool = False) -> None:
        super().__init__(message)
        self.status = status
        self.retryable = retryable

    @property
    def should_fallback(self) -> bool:
        return self.status in FALLBACK_STATUSES or (self.status is None and self.retryable)


@dataclass
class Completion:
    """One model response."""

    text: str
    input_tokens: int = 0
    output_tokens: int = 0
    latency_ms: float = 0.0
    model: str = ""
    url: str = ""

    def to_dict(self) -> Dict[str, Any]:
        return {
            "text": self.text,
            "input_tokens": self.input_tokens,
            "output_tokens": self.output_tokens,
            "latency_ms": self.latency_ms,
            "model": self.model,
            "url": self.url,
        }


def estimate_tokens(text: str) -> int:
    """Cheap, deterministic token estimate (≈4 chars per token)."""
    if not text:
        return 0
    return max(1, (len(text) + 3) // 4)


class BaseProvider:
    """Provider interface."""

    name = "base"
    url = ""

    def complete(self, messages: List[Dict[str, str]], *,
                 stop: Optional[Sequence[str]] = None,
                 max_tokens: Optional[int] = None) -> Completion:
        raise NotImplementedError

    def set_key(self, key: str) -> None:
        """Set the active API key (no-op for offline providers)."""


class ScriptedProvider(BaseProvider):
    """Deterministic provider replaying a script — the offline model.

    Each entry may be a plain string or a dict with ``text`` and optional
    ``raise`` / ``status`` keys (to simulate failures).
    """

    name = "scripted"

    def __init__(self, script: Sequence[Any], model: str = "scripted-model",
                 repeat_last: bool = False) -> None:
        self.script = list(script)
        self.model = model
        self.repeat_last = repeat_last
        self.calls: List[List[Dict[str, str]]] = []
        self._index = 0

    def complete(self, messages, *, stop=None, max_tokens=None) -> Completion:
        self.calls.append(list(messages))
        prompt = "".join(m.get("content", "") for m in messages)
        if self._index >= len(self.script):
            if not self.repeat_last or not self.script:
                raise ProviderError(
                    f"scripted provider exhausted after {len(self.script)} responses",
                    retryable=False,
                )
            entry = self.script[-1]
        else:
            entry = self.script[self._index]
            self._index += 1

        if isinstance(entry, dict) and "raise" in entry:
            raise ProviderError(str(entry["raise"]), status=entry.get("status"),
                                retryable=bool(entry.get("retryable", True)))
        text = entry if isinstance(entry, str) else str(entry.get("text", ""))
        return Completion(
            text=text,
            input_tokens=estimate_tokens(prompt),
            output_tokens=estimate_tokens(text),
            latency_ms=0.0,
            model=self.model,
            url="scripted://local",
        )

    @property
    def exhausted(self) -> bool:
        return self._index >= len(self.script)


class OpenAICompatibleProvider(BaseProvider):
    """Minimal ``/chat/completions`` client (stdlib ``urllib`` only)."""

    name = "openai-compatible"

    def __init__(self, url: str, model: str, api_key: str = "",
                 key_variable: str = "", timeout: float = 120.0) -> None:
        self.url = normalize_url(url)
        self.model = model
        self.api_key = api_key or (os.environ.get(key_variable, "") if key_variable else "")
        self.key_variable = key_variable
        self.timeout = timeout

    def set_key(self, key: str) -> None:
        self.api_key = key

    def complete(self, messages, *, stop=None, max_tokens=None) -> Completion:
        payload: Dict[str, Any] = {"model": self.model, "messages": list(messages)}
        if stop:
            payload["stop"] = list(stop)
        if max_tokens:
            payload["max_tokens"] = max_tokens
        data = json.dumps(payload).encode("utf-8")
        request = urllib.request.Request(
            self.url,
            data=data,
            headers={
                "Content-Type": "application/json",
                "Authorization": f"Bearer {self.api_key}" if self.api_key else "",
            },
            method="POST",
        )
        started = time.monotonic()
        try:
            with urllib.request.urlopen(request, timeout=self.timeout) as response:
                body = response.read().decode("utf-8")
        except urllib.error.HTTPError as exc:
            status = exc.code
            detail = ""
            try:
                detail = exc.read().decode("utf-8", "replace")[:500]
            except Exception:  # noqa: BLE001
                pass
            raise ProviderError(
                f"HTTP {status} from {self.url}: {detail}", status=status,
                retryable=status in FALLBACK_STATUSES,
            ) from None
        except (urllib.error.URLError, TimeoutError) as exc:
            raise ProviderError(f"network error talking to {self.url}: {exc}",
                                retryable=True) from None

        latency = (time.monotonic() - started) * 1000
        try:
            parsed = json.loads(body)
        except json.JSONDecodeError:
            raise ProviderError(f"the provider returned invalid JSON: {body[:300]}") from None
        text = _extract_text(parsed)
        usage = parsed.get("usage") or {}
        return Completion(
            text=text,
            input_tokens=int(usage.get("prompt_tokens") or estimate_tokens(str(messages))),
            output_tokens=int(usage.get("completion_tokens") or estimate_tokens(text)),
            latency_ms=latency,
            model=self.model,
            url=self.url,
        )


class EchoProvider(BaseProvider):
    """Last-resort provider: repeats the task as a code-less answer."""

    name = "echo"
    url = "echo://local"

    def __init__(self, model: str = "echo-model") -> None:
        self.model = model

    def complete(self, messages, *, stop=None, max_tokens=None) -> Completion:
        prompt = messages[-1].get("content", "") if messages else ""
        text = (
            "Thought: I have no model configured, so I will submit what I know.\n"
            "```python\nfinal_answer('echo: ' + " + repr(prompt[:80]) + ")\n```"
        )
        return Completion(text=text, input_tokens=estimate_tokens(prompt),
                          output_tokens=estimate_tokens(text), latency_ms=0.0,
                          model=self.model, url=self.url)


def _extract_text(parsed: Dict[str, Any]) -> str:
    """Pull the assistant text out of an OpenAI-shaped response."""
    choices = parsed.get("choices") or []
    if not choices:
        raise ProviderError("the provider returned no choice")
    message = choices[0].get("message") or {}
    content = message.get("content")
    if isinstance(content, list):  # some providers return content parts
        return "".join(part.get("text", "") for part in content if isinstance(part, dict))
    if content is None:
        return choices[0].get("text", "") or ""
    return str(content)


def normalize_url(url: str) -> str:
    """Append ``/chat/completions`` when the base URL does not carry it."""
    if not url:
        raise ValueError("a provider URL is required")
    cleaned = url.rstrip("/")
    if cleaned.endswith("/chat/completions"):
        return cleaned
    return cleaned + "/chat/completions"


# ---------------------------------------------------------------------------
# Registry and rotation
# ---------------------------------------------------------------------------
@dataclass
class ModelEntry:
    """One model, owned by one provider."""

    model: str
    provider: str
    url: str = ""
    key_variable: str = ""
    strength: int = 0


class TokenRotator:
    """Rotate through several API keys, switching on 429."""

    def __init__(self, keys: Sequence[str]) -> None:
        self.keys = [k for k in keys if k]
        self._index = 0

    def __len__(self) -> int:
        return len(self.keys)

    def current(self) -> str:
        return self.keys[self._index] if self.keys else ""

    def rotate(self) -> bool:
        """Move to the next key; False when there is none left."""
        if self._index + 1 >= len(self.keys):
            return False
        self._index += 1
        return True

    def reset(self) -> None:
        self._index = 0


class ProviderRegistry:
    """Model registry: name -> provider, with key rotation and ordered fallback."""

    def __init__(self, entries: Optional[List[ModelEntry]] = None,
                 env: Optional[Dict[str, str]] = None,
                 factory: Optional[Callable[[ModelEntry], BaseProvider]] = None) -> None:
        self.entries: List[ModelEntry] = list(entries or [])
        self.env = dict(os.environ if env is None else env)
        self.factory = factory or self._default_factory
        self._instances: Dict[str, BaseProvider] = {}

    # -- construction ----------------------------------------------------
    @classmethod
    def from_dict(cls, data: Dict[str, Any], env: Optional[Dict[str, str]] = None,
                  factory: Optional[Callable[[ModelEntry], BaseProvider]] = None
                  ) -> "ProviderRegistry":
        """Build a registry from a ``models.json``-shaped mapping.

        Shape: ``{"providers": {"groq": {"url": "...", "models": ["a", "b"]}}}``.
        """
        providers = data.get("providers", data)
        if not isinstance(providers, dict):
            raise ValueError("the models file must contain a `providers` object")
        entries: List[ModelEntry] = []
        seen = set()
        for position, (provider_name, spec) in enumerate(providers.items()):
            if not isinstance(spec, dict):
                raise ValueError(f"provider {provider_name!r} must be an object")
            url = spec.get("url", "")
            models = spec.get("models") or []
            if url and not models:
                raise ValueError(f"provider {provider_name!r} has a URL but no model")
            if not models:
                raise ValueError(f"provider {provider_name!r} has an empty model list")
            for model in models:
                if model in seen:
                    raise ValueError(f"model {model!r} is served twice")
                seen.add(model)
                entries.append(ModelEntry(
                    model=model,
                    provider=provider_name,
                    url=url,
                    key_variable=f"{provider_name.upper()}_API_KEY",
                    strength=-position,
                ))
        return cls(entries, env=env, factory=factory)

    def _default_factory(self, entry: ModelEntry) -> BaseProvider:
        keys = self._keys_for(entry.key_variable)
        key = keys[0] if keys else ""
        return OpenAICompatibleProvider(entry.url, entry.model, api_key=key,
                                        key_variable=entry.key_variable)

    def _keys_for(self, variable: str) -> List[str]:
        if not variable:
            return []
        candidates = [variable + "S"]
        if variable.endswith("Y"):
            candidates.append(variable[:-1] + "IES")
        raw = ""
        for candidate in candidates:
            raw = self.env.get(candidate) or ""
            if raw:
                break
        raw = raw or self.env.get(variable) or ""
        return [part.strip() for part in raw.split(",") if part.strip()]

    # -- access ----------------------------------------------------------
    def names(self) -> List[str]:
        return [entry.model for entry in self.entries]

    def ordered(self) -> List[ModelEntry]:
        return sorted(self.entries, key=lambda e: (-e.strength, e.model))

    def get(self, model: str) -> BaseProvider:
        for entry in self.entries:
            if entry.model == model:
                if model not in self._instances:
                    self._instances[model] = self.factory(entry)
                return self._instances[model]
        raise ProviderError(f"unknown model: {model!r} (available: {', '.join(self.names())})")

    def entry_for(self, model: str) -> ModelEntry:
        for entry in self.entries:
            if entry.model == model:
                return entry
        raise ProviderError(f"unknown model: {model!r}")

    def has_key(self, model: str) -> bool:
        try:
            entry = self.entry_for(model)
        except ProviderError:
            return False
        return bool(self._keys_for(entry.key_variable)) or not entry.key_variable

    def default_model(self) -> str:
        """First authorized model whose provider actually has a key."""
        ordered = self.ordered()
        for entry in ordered:
            if self.has_key(entry.model):
                return entry.model
        if ordered:
            return ordered[0].model
        raise ProviderError("no model is configured")

    def register(self, model: str, provider: BaseProvider) -> None:
        """Attach an instance directly (used for the scripted/offline model)."""
        if not any(e.model == model for e in self.entries):
            self.entries.append(ModelEntry(model=model, provider=provider.name,
                                           url=provider.url))
        self._instances[model] = provider


def make_rotator(registry: ProviderRegistry, model: str) -> TokenRotator:
    entry = registry.entry_for(model)
    return TokenRotator(registry._keys_for(entry.key_variable))


def call_with_rotation(provider: BaseProvider, rotator: Optional[TokenRotator],
                       messages: List[Dict[str, str]], *, stop=None,
                       max_tokens=None) -> tuple:
    """Call ``provider``, rotating keys on 429. Returns ``(completion, retries)``."""
    retries = 0
    while True:
        try:
            return provider.complete(messages, stop=stop, max_tokens=max_tokens), retries
        except ProviderError as exc:
            if exc.status == 429 and rotator is not None and rotator.rotate():
                provider.set_key(rotator.current())
                retries += 1
                continue
            raise
