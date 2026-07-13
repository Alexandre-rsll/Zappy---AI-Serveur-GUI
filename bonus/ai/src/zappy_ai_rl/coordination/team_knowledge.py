from __future__ import annotations

from dataclasses import dataclass

_DIRECTIONS = {
    1: (1, 0),
    2: (1, -1),
    3: (0, -1),
    4: (-1, -1),
    5: (-1, 0),
    6: (-1, 1),
    7: (0, 1),
    8: (1, 1),
}

_SWITCH_MARGIN = 1


@dataclass
class _Heard:
    level: int
    count: int
    direction: int
    target: tuple[int, int]
    heard_at_tick: int
    rally: bool = False


class TeamKnowledge:
    def __init__(self) -> None:
        self._calls: dict[tuple[int, int], _Heard] = {}
        self._committed_key: tuple[int, int] | None = None

    def note_recruit_call(
        self, bot, level: int, count: int, rally: bool, direction: int
    ) -> None:
        assumed_distance = min(bot.mental_map.width, bot.mental_map.height) // 2
        if direction == 0:
            target = bot.mental_map.position()
        else:
            forward, lateral = _DIRECTIONS[direction]
            dx, dy = bot.mental_map.absolute_offset(
                forward * assumed_distance, lateral * assumed_distance
            )
            target = (
                (bot.mental_map.x + dx) % bot.mental_map.width,
                (bot.mental_map.y + dy) % bot.mental_map.height,
            )
        bucket_size = max(1, assumed_distance // 2)
        key = (target[0] // bucket_size, target[1] // bucket_size)
        self._calls[key] = _Heard(
            level=level,
            count=count,
            direction=direction,
            target=target,
            heard_at_tick=bot.tick_count,
            rally=rally,
        )

    def recruit_heading(
        self, level: int, current_tick: int, max_age_ticks: int
    ) -> int | None:
        call = self._committed_call(level, current_tick, max_age_ticks)
        return None if call is None else call.direction

    def committed_is_rally(
        self, level: int, current_tick: int, max_age_ticks: int
    ) -> bool:
        call = self._committed_call(level, current_tick, max_age_ticks)
        return call is not None and call.rally

    def best_known_count(
        self, level: int, current_tick: int, max_age_ticks: int
    ) -> int:
        live = self._live_calls(level, current_tick, max_age_ticks)
        if not live:
            return 0
        return max(call.count for call in live.values())

    def consume_recruit_call(self) -> None:
        if self._committed_key is not None:
            self._calls.pop(self._committed_key, None)
            self._committed_key = None

    def _committed_call(
        self, level: int, current_tick: int, max_age_ticks: int
    ) -> _Heard | None:
        live = self._live_calls(level, current_tick, max_age_ticks)
        if not live:
            self._committed_key = None
            return None
        best_key = max(
            live, key=lambda key: (live[key].count, live[key].heard_at_tick)
        )
        if self._committed_key in live:
            current_count = live[self._committed_key].count
            if live[best_key].count >= current_count + _SWITCH_MARGIN:
                self._committed_key = best_key
        else:
            self._committed_key = best_key
        return live[self._committed_key]

    def _live_calls(
        self, level: int, current_tick: int, max_age_ticks: int
    ) -> dict[tuple[int, int], _Heard]:
        stale = [
            key
            for key, call in self._calls.items()
            if current_tick - call.heard_at_tick > max_age_ticks
        ]
        for key in stale:
            del self._calls[key]
        return {key: call for key, call in self._calls.items() if call.level == level}
