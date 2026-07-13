from __future__ import annotations

from dataclasses import dataclass, field
from zappy_ai_rl.world.vision import Tile

_FORWARD_VECTORS = ((0, -1), (1, 0), (0, 1), (-1, 0))
_RIGHT_VECTORS = ((1, 0), (0, 1), (-1, 0), (0, -1))


@dataclass
class MentalMap:
    width: int
    height: int
    x: int = 0
    y: int = 0
    facing: int = 0
    known_tiles: dict[tuple[int, int], list[str]] = field(default_factory=dict)

    def turn_right(self) -> None:
        self.facing = (self.facing + 1) % 4

    def turn_left(self) -> None:
        self.facing = (self.facing - 1) % 4

    def move_forward(self) -> None:
        dx, dy = _FORWARD_VECTORS[self.facing]
        self.x = (self.x + dx) % self.width
        self.y = (self.y + dy) % self.height

    def position(self) -> tuple[int, int]:
        return self.x, self.y

    def absolute_offset(self, forward: int, lateral: int) -> tuple[int, int]:
        fx, fy = _FORWARD_VECTORS[self.facing]
        rx, ry = _RIGHT_VECTORS[self.facing]
        return forward * fx + lateral * rx, forward * fy + lateral * ry

    def relative_offset(self, dx: int, dy: int) -> tuple[int, int]:
        fx, fy = _FORWARD_VECTORS[self.facing]
        rx, ry = _RIGHT_VECTORS[self.facing]
        return dx * fx + dy * fy, dx * rx + dy * ry

    def heading_to(self, target: tuple[int, int]) -> tuple[int, int]:
        dx = _axis_delta(self.x, target[0], self.width)
        dy = _axis_delta(self.y, target[1], self.height)
        return self.relative_offset(dx, dy)

    def record_vision(self, tiles: list[Tile]) -> None:
        for tile in tiles:
            dx, dy = self.absolute_offset(tile.forward, tile.lateral)
            pos = ((self.x + dx) % self.width, (self.y + dy) % self.height)
            self.known_tiles[pos] = tile.objects

    def nearest_known(self, resource: str) -> tuple[int, int] | None:
        best: tuple[int, int] | None = None
        best_dist = 0
        for pos, objects in self.known_tiles.items():
            if pos == (self.x, self.y) or resource not in objects:
                continue
            dist = abs(_axis_delta(self.x, pos[0], self.width)) + abs(
                _axis_delta(self.y, pos[1], self.height)
            )
            if best is None or dist < best_dist:
                best, best_dist = pos, dist
        return best


def _axis_delta(current: int, target: int, size: int) -> int:
    delta = (target - current) % size
    if delta > size // 2:
        delta -= size
    return delta
