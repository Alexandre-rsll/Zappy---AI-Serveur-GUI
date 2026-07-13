from __future__ import annotations

from collections import deque
from dataclasses import dataclass, field
from typing import Callable

MAX_IN_FLIGHT = 10

IsTerminal = Callable[[str, list[str]], bool]
OnComplete = Callable[[list[str]], None]


@dataclass
class _Pending:
    command: str
    lines: list[str] = field(default_factory=list)
    on_complete: OnComplete | None = None


class CommandQueue:
    def __init__(self, is_terminal: IsTerminal) -> None:
        self._pending: deque[_Pending] = deque()
        self._is_terminal = is_terminal

    def has_room(self) -> bool:
        return len(self._pending) < MAX_IN_FLIGHT

    def in_flight_count(self) -> int:
        return len(self._pending)

    def head_command(self) -> str | None:
        return self._pending[0].command if self._pending else None

    def note_sent(self, command: str, on_complete: OnComplete | None = None) -> None:
        if not self.has_room():
            raise RuntimeError(
                f"command queue full ({MAX_IN_FLIGHT}), cannot send {command!r}"
            )
        self._pending.append(_Pending(command=command, on_complete=on_complete))

    def feed_line(self, line: str) -> tuple[str, list[str]] | None:
        if not self._pending:
            return None
        head = self._pending[0]
        head.lines.append(line)
        if self._is_terminal(head.command, head.lines):
            self._pending.popleft()
            if head.on_complete is not None:
                head.on_complete(head.lines)
            return head.command, head.lines
        return None
