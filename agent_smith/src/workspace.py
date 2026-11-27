"""Repository backends used by the SWE-bench tools.

Both backends expose the same interface — ``exec``, ``read``, ``replace_file``,
``workdir``, ``cleanup`` — so the tools never know whether they run on a local
checkout or inside a task container.

  * :class:`LocalWorkspace` runs commands with ``subprocess`` directly on the host
    checkout (the mode used to test the tools in isolation).
  * :class:`DockerWorkspace` would drive one long-lived container per task; it is
    provided as an optional shim that requires the Docker SDK, and degrades with an
    explicit error when it is unavailable.
"""

from __future__ import annotations

import os
import shutil
import subprocess
import tempfile
from dataclasses import dataclass
from typing import Optional


class WorkspaceError(RuntimeError):
    """Raised when a backend command cannot be executed at all."""


@dataclass
class CommandResult:
    """Outcome of one backend command."""

    stdout: str = ""
    stderr: str = ""
    exit_code: int = 0
    timed_out: bool = False


class LocalWorkspace:
    """Run commands on a local checkout."""

    def __init__(self, root: str, timeout: int = 600) -> None:
        if not root:
            raise WorkspaceError("a repository path is required")
        self.root = os.path.realpath(os.path.abspath(root))
        if not os.path.isdir(self.root):
            raise WorkspaceError(f"repository path is not a directory: {root}")
        self.timeout = timeout

    # -- interface -------------------------------------------------------
    def workdir(self) -> str:
        return self.root

    def exec(self, command, workdir: Optional[str] = None, timeout: Optional[float] = None,
             check: bool = False) -> CommandResult:
        cwd = self._resolve(workdir)
        timeout = self.timeout if timeout is None else timeout
        shell = isinstance(command, str)
        try:
            completed = subprocess.run(
                command,
                shell=shell,
                cwd=cwd,
                capture_output=True,
                text=True,
                timeout=timeout,
            )
        except subprocess.TimeoutExpired as exc:
            stdout = exc.stdout or ""
            stderr = exc.stderr or ""
            if isinstance(stdout, bytes):
                stdout = stdout.decode("utf-8", "replace")
            if isinstance(stderr, bytes):
                stderr = stderr.decode("utf-8", "replace")
            return CommandResult(stdout=stdout, stderr=stderr, exit_code=124, timed_out=True)
        except FileNotFoundError as exc:
            raise WorkspaceError(f"command not found: {exc}") from exc
        result = CommandResult(
            stdout=completed.stdout or "",
            stderr=completed.stderr or "",
            exit_code=completed.returncode,
        )
        if check and result.exit_code != 0:
            raise WorkspaceError(result.stderr or f"command failed with {result.exit_code}")
        return result

    def read(self, path: str) -> str:
        real = self._resolve(path)
        with open(real, encoding="utf-8", errors="replace") as fh:
            return fh.read()

    def write(self, path: str, content: str) -> None:
        real = self._resolve(path)
        os.makedirs(os.path.dirname(real) or ".", exist_ok=True)
        _atomic_write(real, content)

    def replace_file(self, path: str, content: str) -> None:
        """Atomic replacement (temp file + rename), never a shell command."""
        self.write(path, content)

    def exists(self, path: str) -> bool:
        return os.path.exists(self._resolve(path))

    def cleanup(self) -> None:
        """Nothing to clean up for a host checkout."""

    # -- helpers ---------------------------------------------------------
    def _resolve(self, path: Optional[str]) -> str:
        if not path:
            return self.root
        candidate = path if os.path.isabs(path) else os.path.join(self.root, path)
        real = os.path.realpath(candidate)
        if real != self.root and not real.startswith(self.root + os.sep):
            raise WorkspaceError(f"path escapes the repository: {path}")
        return real

    def relative(self, path: str) -> str:
        return os.path.relpath(self._resolve(path), self.root)


def _atomic_write(path: str, content: str) -> None:
    directory = os.path.dirname(path) or "."
    handle, tmp_path = tempfile.mkstemp(dir=directory, suffix=".tmp")
    try:
        with os.fdopen(handle, "w", encoding="utf-8") as fh:
            fh.write(content)
        shutil.copymode(path, tmp_path) if os.path.exists(path) else None
        os.replace(tmp_path, path)
    except BaseException:
        if os.path.exists(tmp_path):
            os.unlink(tmp_path)
        raise


class DockerWorkspace:
    """Drive one long-lived container from the task's ``docker_image``.

    State (file edits, git history) persists for the whole task, the way it would
    inside a real terminal session. Requires the optional Docker SDK.
    """

    def __init__(self, image: str, testbed_path: str = "/testbed",
                 timeout: int = 600) -> None:
        try:
            import docker  # noqa: F401 - optional dependency
        except ImportError as exc:  # pragma: no cover - optional path
            raise WorkspaceError(
                "the Docker backend requires the `docker` package "
                "(pip install docker); use the local backend instead"
            ) from exc
        import docker

        self.client = docker.from_env()
        self.container = self.client.containers.run(
            image, command="tail -f /dev/null", detach=True, init=True
        )
        self.testbed_path = testbed_path
        self.timeout = timeout

    def workdir(self) -> str:
        return self.testbed_path

    def exec(self, command, workdir: Optional[str] = None, timeout: Optional[float] = None,
             check: bool = False) -> CommandResult:
        shell = isinstance(command, str)
        if shell:
            argv = ["bash", "-lc", command]
        else:
            argv = list(command)
        try:
            exit_code, output = self.container.exec_run(
                argv, workdir=workdir or self.testbed_path, demux=False
            )
        except Exception as exc:  # pragma: no cover - optional path
            raise WorkspaceError(f"container exec failed: {exc}") from exc
        text = output.decode("utf-8", "replace") if isinstance(output, bytes) else str(output)
        result = CommandResult(stdout=text, stderr="", exit_code=exit_code)
        if check and exit_code != 0:
            raise WorkspaceError(text or f"command failed with {exit_code}")
        return result

    def read(self, path: str) -> str:
        code, data = self.container.get_archive(path)
        import io
        import tarfile

        archive = tarfile.open(fileobj=io.BytesIO(b"".join(data)))
        member = archive.next()
        handle = archive.extractfile(member)
        return handle.read().decode("utf-8", "replace") if handle else ""

    def replace_file(self, path: str, content: str) -> None:  # pragma: no cover - optional
        """Write through a tar archive, never a shell command."""
        import io
        import posixpath
        import tarfile

        data = content.encode("utf-8")
        info = tarfile.TarInfo(name=posixpath.basename(path))
        info.size = len(data)
        info.mode = 0o644
        buffer = io.BytesIO()
        with tarfile.open(fileobj=buffer, mode="w") as archive:
            archive.addfile(info, io.BytesIO(data))
        buffer.seek(0)
        self.container.put_archive(posixpath.dirname(path), buffer.read())

    def write(self, path: str, content: str) -> None:
        self.replace_file(path, content)

    def exists(self, path: str) -> bool:
        return self.exec(["test", "-e", path]).exit_code == 0

    def cleanup(self) -> None:
        try:
            self.container.remove(force=True)
        except Exception:  # pragma: no cover - optional path
            pass
