"""Restricted execution environment for generated code.

Design (mirrors the subject):

  * a dedicated WORKER PROCESS per session, so a hard timeout and a memory limit
    can really be enforced (SIGTERM + ``resource.setrlimit``);
  * persistence: variables and functions defined in one block stay available in
    the next one — implemented by replaying the previous successful blocks into a
    fresh worker after a timeout;
  * security layers, all stdlib only: import allowlist, filesystem allowlist on
    ``open`` (realpath-resolved), network ban, restricted builtins, AST rejection
    of dunder / frame attributes at parse time, and a ``sys.addaudithook``
    backstop that sees what the interpreter actually does;
  * ``final_answer(value)`` injected in every namespace — it is not a tool;
  * MCP-style tools become plain Python functions through a proxy that crosses the
    pipe to the parent, which is why a tool call is not bound by the sandbox
    timeout.

Only three functions are public: ``execute``, ``get_manual`` and ``close``.
"""

from __future__ import annotations

import ast
import io
import multiprocessing as mp
import os
import signal
import sys
import time
import traceback
from contextlib import redirect_stderr, redirect_stdout
from typing import Any, Dict, List, Optional

from config import SandboxConfig

# Builtins kept available inside the sandbox.
SAFE_BUILTINS = (
    "abs", "all", "any", "ascii", "bin", "bool", "bytearray", "bytes", "callable",
    "chr", "classmethod", "complex", "dict", "dir", "divmod", "enumerate", "filter",
    "float", "format", "frozenset", "getattr", "hasattr", "hash", "hex", "id",
    "int", "isinstance", "issubclass", "iter", "len", "list", "map", "max",
    "min", "next", "object", "oct", "ord", "pow", "print", "property", "range",
    "repr", "reversed", "round", "set", "setattr", "slice", "sorted", "staticmethod",
    "str", "sum", "tuple", "type", "zip", "None", "True", "False", "Exception",
    # `open` is allowed, but only through the restricted wrapper installed in the
    # namespace: it enforces the directory allowlist.
    "open",
    "BaseException", "ValueError", "TypeError", "KeyError", "IndexError",
    "AttributeError", "RuntimeError", "ZeroDivisionError", "StopIteration",
    "ArithmeticError", "AssertionError", "LookupError", "NameError", "OSError",
    "NotImplementedError", "OverflowError", "RecursionError", "UnicodeError",
    "MemoryError", "SystemExit", "KeyboardInterrupt", "NotImplemented", "Ellipsis",
    "__build_class__", "__name__",
)

FORBIDDEN_AUDIT_EVENTS = (
    "socket.connect", "socket.bind", "socket.getaddrinfo", "socket.gethostbyname",
    "os.system", "os.exec", "os.posix_spawn", "os.spawn", "os.fork", "os.forkpty",
    "subprocess.Popen", "ctypes.dlopen", "ctypes.dlsym", "ctypes.call_function",
)


class SandboxError(RuntimeError):
    """Raised when the sandbox itself cannot run (not when user code fails)."""


class _FinalAnswer(BaseException):
    """Internal signal raised by final_answer() to stop the block."""

    def __init__(self, value: Any) -> None:
        super().__init__("final_answer")
        self.value = value


# ---------------------------------------------------------------------------
# Worker side
# ---------------------------------------------------------------------------
def _safe_path(path: Any) -> Optional[str]:
    """Return a comparable absolute real path, or ``None`` when unusable."""
    if isinstance(path, int):
        return None
    try:
        if isinstance(path, bytes):
            path = path.decode("utf-8", "replace")
        if not isinstance(path, str):
            path = os.fspath(path)
            if isinstance(path, bytes):
                path = path.decode("utf-8", "replace")
    except (TypeError, ValueError):
        return None
    try:
        return os.path.realpath(os.path.abspath(path))
    except (OSError, ValueError):
        return None


def _install_audit_hook(config: SandboxConfig) -> None:
    """Install the backstop hook. It cannot be removed afterwards."""
    allowed = list(config.allowed_directories)
    python_paths = [os.path.realpath(p) for p in
                    {sys.prefix, sys.base_prefix, sys.exec_prefix} if p]
    allow_network = config.allow_network
    allow_subprocess = config.allow_subprocess

    def hook(event: str, args: tuple) -> None:
        if event == "open":
            path = args[0] if args else None
            real = _safe_path(path)
            if real is None:
                return
            for root in python_paths + allowed:
                if real == root or real.startswith(root.rstrip(os.sep) + os.sep):
                    return
            if real.endswith(".so") or real.endswith(".pyd"):
                return
            raise PermissionError(
                f"filesystem access denied by the sandbox audit hook: {path!r}"
            )
        if event in ("socket.connect", "socket.bind", "socket.getaddrinfo",
                     "socket.gethostbyname") and not allow_network:
            raise PermissionError(f"network access denied by the sandbox audit hook: {event}")
        if event in ("os.system", "os.exec", "os.posix_spawn", "os.spawn",
                     "os.fork", "os.forkpty", "subprocess.Popen") and not allow_subprocess:
            raise PermissionError(f"process spawning denied by the sandbox audit hook: {event}")
        if event.startswith("ctypes.") and not allow_subprocess:
            raise PermissionError(f"ctypes use denied by the sandbox audit hook: {event}")

    sys.addaudithook(hook)


def _make_restricted_import(config: SandboxConfig):
    real_import = __builtins__["__import__"] if isinstance(__builtins__, dict) else __builtins__.__import__

    def restricted_import(name, globals=None, locals=None, fromlist=(), level=0):  # noqa: A002
        if level != 0:
            raise ImportError(f"relative imports are not allowed in the sandbox: {name!r}")
        if not config.import_allowed(name):
            raise ImportError(
                f"module {name!r} is not in the sandbox import allowlist "
                f"({', '.join(config.authorized_imports[:6])}...)"
            )
        module = real_import(name, globals, locals, fromlist, level)
        # Importing `os.path` would also expose `os`: strip it from the namespace.
        for item in list(fromlist or ()):
            if not config.import_allowed(f"{name}.{item}") and hasattr(module, item):
                try:
                    delattr(module, item)
                except (AttributeError, TypeError):
                    pass
        return module

    return restricted_import


def _make_restricted_open(config: SandboxConfig):
    real_open = io.open

    def restricted_open(file, mode="r", *args, **kwargs):
        real = _safe_path(file)
        if real is None:
            raise PermissionError(f"invalid path for open(): {file!r}")
        if not config.directory_allowed(real):
            raise PermissionError(
                "filesystem access denied: "
                f"{file!r} is outside the allowed directories {config.allowed_directories}"
            )
        if any(c in mode for c in "wax+"):
            pass  # writing allowed inside the allowlist
        return real_open(file, mode, *args, **kwargs)

    return restricted_open


def _make_blocked_socket(config: SandboxConfig):
    if config.allow_network:
        import socket as _socket

        return _socket.socket

    def blocked(*args, **kwargs):
        raise PermissionError("network access is disabled in the sandbox")

    return blocked


def _build_namespace(config: SandboxConfig) -> Dict[str, Any]:
    """Namespace containing only the allowed builtins plus sandbox helpers."""
    import builtins as real_builtins

    safe: Dict[str, Any] = {}
    for name in SAFE_BUILTINS:
        value = getattr(real_builtins, name, None)
        if value is not None:
            safe[name] = value
    safe["__import__"] = _make_restricted_import(config)
    safe["open"] = _make_restricted_open(config)
    for name in ("eval", "exec", "compile", "input", "breakpoint", "globals",
                 "locals", "vars", "help", "exit", "quit"):
        safe.pop(name, None)
    safe["__name__"] = "__sandbox__"
    safe["__builtins__"] = safe

    namespace: Dict[str, Any] = {"__builtins__": safe, "__name__": "__sandbox__"}

    # The network is disabled by swapping socket.socket out of the namespace.
    import socket as _socket
    namespace["socket"] = type(sys)("socket")
    namespace["socket"].socket = _make_blocked_socket(config)

    def final_answer(value: Any = None) -> None:  # noqa: D401
        """Submit the final answer and stop the current block."""
        raise _FinalAnswer(value)

    namespace["final_answer"] = final_answer
    return namespace


def _inject_tools(namespace: Dict[str, Any], names: List[str], conn) -> None:
    """Expose every MCP tool as a plain Python function in the namespace."""

    def make_proxy(tool_name: str):
        def proxy(*args, **kwargs):
            payload = {"type": "tool_call", "name": tool_name,
                       "args": list(args), "kwargs": kwargs}
            conn.send(payload)
            reply = conn.recv()
            if reply.get("status") != "ok":
                raise RuntimeError(
                    f"tool {tool_name!r} failed: {reply.get('error') or reply.get('output')}"
                )
            return reply.get("result")

        proxy.__name__ = tool_name
        proxy.__qualname__ = tool_name
        proxy.__doc__ = f"MCP tool {tool_name!r} (proxy)."
        return proxy

    for tool_name in names:
        namespace[tool_name] = make_proxy(tool_name)


def _check_ast(code: str, config: SandboxConfig) -> None:
    """Reject dunder / frame attributes and forbidden builtins at parse time."""
    tree = ast.parse(code)
    forbidden_attrs = set(config.forbidden_attributes)
    forbidden_builtins = set(config.forbidden_builtins)
    for node in ast.walk(tree):
        if isinstance(node, ast.Attribute):
            attr = node.attr
            if attr.startswith("__") and attr.endswith("__"):
                raise SecurityError(
                    f"access to dunder attribute {attr!r} is forbidden in the sandbox"
                )
            if attr in forbidden_attrs:
                raise SecurityError(
                    f"access to attribute {attr!r} is forbidden in the sandbox"
                )
            if attr.startswith("f_") or attr.startswith("tb_") or attr.startswith("gi_"):
                raise SecurityError(
                    f"access to frame attribute {attr!r} is forbidden in the sandbox"
                )
        elif isinstance(node, ast.Name):
            if node.id in forbidden_builtins and node.id not in SAFE_BUILTINS:
                # `open` is in SAFE_BUILTINS: the namespace provides the restricted
                # wrapper, so it stays usable inside the allowed directories.
                raise SecurityError(
                    f"use of {node.id!r} is forbidden in the sandbox"
                )


class SecurityError(Exception):
    """Raised when generated code violates a static sandbox rule."""


def _worker_main(conn, config: SandboxConfig, tool_names: List[str]) -> None:
    """Worker process: read blocks, run them, reply. Never returns normally."""
    try:
        signal.signal(signal.SIGINT, signal.SIG_IGN)
    except (ValueError, OSError):
        pass
    try:
        import resource

        limit = int(config.max_memory_mb) * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (limit, limit))
        cpu = int(config.max_cpu_seconds) + 5
        resource.setrlimit(resource.RLIMIT_CPU, (cpu, cpu))
    except Exception:  # pragma: no cover - platform dependent
        pass

    namespace = _build_namespace(config)
    _inject_tools(namespace, tool_names, conn)
    hook_installed = False

    while True:
        try:
            message = conn.recv()
        except (EOFError, OSError):
            return
        if message.get("cmd") == "close":
            return
        if message.get("cmd") != "exec":
            continue

        code = message.get("code", "")
        if not hook_installed:
            _install_audit_hook(config)
            hook_installed = True

        stdout, stderr = io.StringIO(), io.StringIO()
        result: Dict[str, Any] = {
            "type": "result", "stdout": "", "stderr": "", "error": None,
            "final_answer": None,
        }
        try:
            _check_ast(code, config)
            compiled = compile(code, "<agent>", "exec")
        except SecurityError as exc:
            result["error"] = f"SecurityError: {exc}"
            conn.send(result)
            continue
        except SyntaxError as exc:
            result["error"] = f"SyntaxError: {exc.msg} (line {exc.lineno})"
            conn.send(result)
            continue

        try:
            with redirect_stdout(stdout), redirect_stderr(stderr):
                exec(compiled, namespace)  # noqa: S102 - that is the point
        except _FinalAnswer as signal_exc:
            result["final_answer"] = signal_exc.value
        except SystemExit as exc:
            result["error"] = f"SystemExit({exc.code}) was raised by the sandboxed code"
        except KeyboardInterrupt:
            result["error"] = "KeyboardInterrupt was raised by the sandboxed code"
        except MemoryError:
            result["error"] = "MemoryError: the sandbox memory limit was exceeded"
        except BaseException as exc:  # noqa: BLE001 - report everything to the model
            result["error"] = "".join(
                traceback.format_exception_only(type(exc), exc)
            ).strip()

        result["stdout"] = stdout.getvalue()
        result["stderr"] = stderr.getvalue()
        try:
            conn.send(result)
        except (BrokenPipeError, OSError):
            return


# ---------------------------------------------------------------------------
# Parent side
# ---------------------------------------------------------------------------
class Sandbox:
    """Restricted Python executor satisfying the shared SandboxProtocol."""

    def __init__(self, config: Optional[SandboxConfig] = None, tools: Any = None) -> None:
        self.config = config or SandboxConfig()
        self.tools = tools
        self._process: Optional[mp.Process] = None
        self._conn = None
        self._history: List[str] = []
        self._closed = False
        self._restarts = 0
        self._tool_calls: List[Any] = []

    # -- lifecycle -------------------------------------------------------
    def _start_worker(self) -> None:
        parent_conn, child_conn = mp.Pipe()
        context = mp.get_context("fork") if "fork" in mp.get_all_start_methods() else mp.get_context()
        process = context.Process(
            target=_worker_main,
            args=(child_conn, self.config, self._tool_names()),
            daemon=True,
        )
        process.start()
        child_conn.close()
        self._process = process
        self._conn = parent_conn

    def _tool_names(self) -> List[str]:
        if self.tools is None:
            return []
        if hasattr(self.tools, "names"):
            return list(self.tools.names())
        return list(getattr(self.tools, "tools", {}) or {})

    def _kill_worker(self) -> None:
        if self._process is not None:
            try:
                if self._process.is_alive():
                    self._process.terminate()
                    self._process.join(timeout=2)
                if self._process.is_alive():
                    self._process.kill()
                    self._process.join(timeout=2)
            except (OSError, ValueError):
                pass
        if self._conn is not None:
            try:
                self._conn.close()
            except (OSError, ValueError):
                pass
        self._process = None
        self._conn = None

    def close(self) -> None:
        """Stop the worker process."""
        if self._closed:
            return
        self._closed = True
        if self._conn is not None and self._process is not None and self._process.is_alive():
            try:
                self._conn.send({"cmd": "close"})
                self._process.join(timeout=1)
            except (OSError, ValueError, EOFError):
                pass
        self._kill_worker()

    def __enter__(self) -> "Sandbox":
        return self

    def __exit__(self, *exc_info) -> None:
        self.close()

    def get_manual(self) -> str:
        """Sandbox manual injected into the system prompt."""
        if self.tools is None:
            tool_section = "No MCP server is connected: no tools are available."
        else:
            tool_section = self.tools.manual()
        directories = ", ".join(self.config.allowed_directories) or "(none)"
        return (
            "## Sandbox manual\n\n"
            "You write Python code. Each block runs in a restricted worker process and its\n"
            "stdout, stderr and errors come back as the Observation.\n\n"
            f"- Memory limit: {self.config.max_memory_mb} MB\n"
            f"- Time limit per block: {self.config.max_cpu_seconds} s\n"
            f"- Allowed directories: {directories}\n"
            f"- Network: {'allowed' if self.config.allow_network else 'forbidden'}\n"
            f"- Authorized imports: {', '.join(self.config.authorized_imports)}\n"
            "- Variables and functions persist between blocks.\n"
            "- Dunder attributes and frame attributes are rejected before execution.\n"
            "- Call `final_answer(value)` to submit your answer and stop the loop.\n"
            "- `final_answer` is not a tool: it is always available.\n\n"
            f"{tool_section}"
        )

    # -- execution -------------------------------------------------------
    def execute(self, code: str, timeout: Optional[float] = None) -> Any:
        """Run ``code`` in the worker and return an :class:`ExecutionResult`."""
        from models import ExecutionResult, ToolCall

        if self._closed:
            raise SandboxError("sandbox is closed")
        timeout = float(timeout if timeout is not None else self.config.max_cpu_seconds)
        started = time.monotonic()
        self._tool_calls = []

        if self._process is None or not self._process.is_alive():
            self._start_worker()

        try:
            self._conn.send({"cmd": "exec", "code": code})
        except (BrokenPipeError, OSError, ValueError):
            self._restart()
            try:
                self._conn.send({"cmd": "exec", "code": code})
            except (BrokenPipeError, OSError, ValueError) as exc:
                return ExecutionResult(
                    error=f"sandbox worker unavailable: {exc}",
                    duration_s=time.monotonic() - started,
                )

        deadline = started + timeout
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                self._kill_worker()
                self._restarts += 1
                result = ExecutionResult(
                    timed_out=True,
                    error=f"TimeoutError: execution exceeded {timeout:g}s",
                    duration_s=time.monotonic() - started,
                    tool_calls=list(self._tool_calls),
                )
                # The worker is gone: replay what worked so the session keeps
                # its state (variables, functions) for the next block.
                self._replay()
                return result
            try:
                ready = self._conn.poll(min(remaining, 0.5))
            except (OSError, ValueError):
                ready = False
            if not ready:
                if self._process is None or not self._process.is_alive():
                    self._kill_worker()
                    return ExecutionResult(
                        error="sandbox worker died unexpectedly (crash or memory limit)",
                        duration_s=time.monotonic() - started,
                        tool_calls=list(self._tool_calls),
                    )
                continue

            try:
                message = self._conn.recv()
            except (EOFError, OSError):
                self._kill_worker()
                return ExecutionResult(
                    error="sandbox worker died unexpectedly (crash or memory limit)",
                    duration_s=time.monotonic() - started,
                    tool_calls=list(self._tool_calls),
                )

            if message.get("type") == "tool_call":
                tool_started = time.monotonic()
                reply = self._dispatch_tool(message)
                deadline += time.monotonic() - tool_started  # tools are not timed out
                try:
                    self._conn.send(reply)
                except (BrokenPipeError, OSError):
                    self._kill_worker()
                    return ExecutionResult(
                        error="sandbox worker died while a tool was running",
                        duration_s=time.monotonic() - started,
                        tool_calls=list(self._tool_calls),
                    )
                continue

            result = ExecutionResult(
                stdout=message.get("stdout", ""),
                stderr=message.get("stderr", ""),
                error=message.get("error"),
                final_answer=message.get("final_answer"),
                duration_s=time.monotonic() - started,
                tool_calls=list(self._tool_calls),
            )
            if len(result.stdout) > self.config.max_output_chars:
                result.stdout = result.stdout[: self.config.max_output_chars]
                result.truncated = True
            if result.success:
                self._history.append(code)
            return result

    def _dispatch_tool(self, message: Dict[str, Any]) -> Dict[str, Any]:
        """Run one tool request coming from the worker."""
        from models import ToolCall

        name = message.get("name", "")
        args = list(message.get("args") or [])
        kwargs = dict(message.get("kwargs") or {})
        call = ToolCall(name=name, arguments={**kwargs, "_args": args})
        self._tool_calls.append(call)
        if self.tools is None or name not in self._tool_names():
            call.status = "error"
            call.error = f"unknown tool: {name}"
            return {"status": "error", "error": call.error}
        try:
            result = self.tools.call(name, *args, **kwargs)
        except Exception as exc:  # noqa: BLE001 - reported to the model
            call.status = "error"
            call.error = f"{type(exc).__name__}: {exc}"
            return {"status": "error", "error": call.error}
        return {"status": "ok", "result": result}

    def _restart(self) -> None:
        """Start a fresh worker and replay the successful blocks into it."""
        self._kill_worker()
        self._start_worker()
        self._restarts += 1
        self._replay()

    def _replay(self) -> None:
        """Re-execute the recorded history in the current worker."""
        replay = list(self._history)
        self._history = []
        for code in replay:
            result = self.execute(code)
            if result.final_answer is not None or result.error:
                # A block that no longer works is skipped: the session continues
                # with whatever still succeeds.
                continue

    def replay_history(self) -> None:
        """Re-execute the recorded history (used after a timeout)."""
        self._restart()

    @property
    def restarts(self) -> int:
        return self._restarts

    @property
    def history(self) -> List[str]:
        return list(self._history)
