#!/usr/bin/env python3
"""The Answer Protocol command-line entry point."""

from __future__ import annotations

import argparse
import json
import socket
import sys
import time
from pathlib import Path
from typing import Optional

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "src"
if str(SRC) not in sys.path:
    sys.path.insert(0, str(SRC))

import protocol  # noqa: E402
from client_cli import run_client  # noqa: E402
from client_gui import serve_gui  # noqa: E402
from logging_json import JsonLogger, NullLogger  # noqa: E402
from server import TapServer, serve  # noqa: E402
from world import World, WorldError, default_world_path  # noqa: E402


class DemoWire:
    """Small synchronous client used by ``tap.py demo``."""

    def __init__(self, label: str, host: str, port: int) -> None:
        self.label = label
        self.sock = socket.create_connection((host, port), timeout=3)
        self.sock.settimeout(0.15)
        self.buffer = bytearray()
        greeting = self.read_line(3)
        self.show(greeting)

    def show(self, line: str) -> None:
        print(f"{self.label:>6} ← {line}")

    def read_line(self, timeout: float = 2.0) -> str:
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            newline = self.buffer.find(b"\n")
            if newline >= 0:
                raw = bytes(self.buffer[:newline])
                del self.buffer[:newline + 1]
                return raw.decode("utf-8", "replace").rstrip("\r")
            try:
                chunk = self.sock.recv(4096)
            except socket.timeout:
                continue
            if not chunk:
                return ""
            self.buffer.extend(chunk)
        return ""

    def command(self, command: str, timeout: float = 2.0) -> str:
        print(f"{self.label:>6} → {command}")
        self.sock.sendall(protocol.frame(command))
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            line = self.read_line(max(0.01, deadline - time.monotonic()))
            if not line:
                continue
            self.show(line)
            if line.startswith(("OK", "ERR ")):
                return line
        raise TimeoutError(f"no reply for {command!r} ({self.label})")

    def close(self) -> None:
        try:
            self.sock.close()
        except OSError:
            pass


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="A shared-world text adventure over RFC 42TAP.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    subparsers = parser.add_subparsers(dest="action", required=True)

    server_parser = subparsers.add_parser("server", help="start the authoritative TCP server")
    server_parser.add_argument("--host", default="127.0.0.1")
    server_parser.add_argument("--port", type=int, default=4242)
    server_parser.add_argument("--world", default=default_world_path())
    server_parser.add_argument("--log-level", default="INFO",
                                choices=("DEBUG", "INFO", "WARN", "ERROR"))

    client_parser = subparsers.add_parser("client", help="start the interactive CLI client")
    client_parser.add_argument("--host", default="127.0.0.1")
    client_parser.add_argument("--port", type=int, default=4242)

    gui_parser = subparsers.add_parser("gui", help="serve the HTTP/WebSocket GUI client")
    gui_parser.add_argument("--host", default="0.0.0.0")
    gui_parser.add_argument("--port", type=int, default=8080)
    gui_parser.add_argument("--tap-host", default="127.0.0.1")
    gui_parser.add_argument("--tap-port", type=int, default=4242)
    gui_parser.add_argument("--log-level", default="INFO",
                             choices=("DEBUG", "INFO", "WARN", "ERROR"))

    world_parser = subparsers.add_parser("check-world", help="validate world references")
    world_parser.add_argument("--world", default=default_world_path())
    world_parser.add_argument("--json", action="store_true", help="print a JSON report")

    demo_parser = subparsers.add_parser("demo", help="run a scripted two-player scenario")
    demo_parser.add_argument("--world", default=default_world_path())

    return parser


def load_world(path: str) -> World:
    try:
        return World.load(path)
    except WorldError as exc:
        raise SystemExit(f"world validation failed: {exc}") from exc


def check_world(path: str, as_json: bool = False) -> int:
    try:
        world = World.load(path)
    except WorldError as exc:
        if as_json:
            print(json.dumps({"valid": False, "error": str(exc)}, ensure_ascii=False))
        else:
            print(f"INVALID: {exc}", file=sys.stderr)
        return 1
    report = {"valid": True, "name": world.name, "start_room": world.start_room,
              "sleep_room": world.sleep_room, **world.stats()}
    if as_json:
        print(json.dumps(report, ensure_ascii=False, separators=(",", ":")))
    else:
        print(f"VALID: {world.name}")
        print("  " + ", ".join(f"{key}={value}" for key, value in world.stats().items()))
        print(f"  start={world.start_room}; sleep={world.sleep_room}")
    return 0


def demo(path: str) -> int:
    world = load_world(path)
    server = TapServer(world, host="127.0.0.1", port=0, logger=NullLogger())
    port = server.start()
    paul: Optional[DemoWire] = None
    chani: Optional[DemoWire] = None
    try:
        paul = DemoWire("PAUL", "127.0.0.1", port)
        chani = DemoWire("CHANI", "127.0.0.1", port)
        paul.command("CONNECT Paul")
        chani.command("CONNECT Chani")
        paul.command("MOVE east")
        paul.command("QUEST Chani")
        paul.command("TAKE melange")
        paul.command("TAKE melange")
        paul.command("QUEST Chani")
        chani.command("CHAT GLOBAL The desert remembers.")
        paul.command("GROUP CREATE fedaykin")
        paul.command("GROUP INVITE Chani")
        chani.command("GROUP JOIN fedaykin")
        paul.command("CHAT GROUP May your knife chip and shatter.")
        paul.command("LOOK")
        paul.command("QUIT")
        chani.command("QUIT")
        print("Demo complete: two real TCP clients, one shared game state.")
        return 0
    except (OSError, TimeoutError) as exc:
        print(f"demo failed: {exc}", file=sys.stderr)
        return 1
    finally:
        if paul is not None:
            paul.close()
        if chani is not None:
            chani.close()
        server.stop()


def main(argv: Optional[list] = None) -> int:
    args = build_parser().parse_args(argv)
    if args.action == "check-world":
        return check_world(args.world, args.json)
    if args.action == "demo":
        return demo(args.world)
    if args.action == "server":
        world = load_world(args.world)
        logger = JsonLogger(level=args.log_level)
        serve(world, host=args.host, port=args.port, logger=logger)
        return 0
    if args.action == "client":
        return run_client(args.host, args.port)
    if args.action == "gui":
        logger = JsonLogger(level=args.log_level)
        serve_gui(args.host, args.port, args.tap_host, args.tap_port, logger)
        return 0
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
