from __future__ import annotations

from dataclasses import dataclass
from zappy_ai_rl.protocol.parser import strip_brackets

FOOD_LIFE_UNITS = 126

RESOURCES = (
    "food",
    "linemate",
    "deraumere",
    "sibur",
    "mendiane",
    "phiras",
    "thystame",
)


@dataclass
class Inventory:
    food: int = 0
    linemate: int = 0
    deraumere: int = 0
    sibur: int = 0
    mendiane: int = 0
    phiras: int = 0
    thystame: int = 0

    def survival_time_remaining(self) -> int:
        return self.food * FOOD_LIFE_UNITS

    def count(self, resource: str) -> int:
        return getattr(self, resource)


def parse_inventory(raw: str) -> Inventory:
    values: dict[str, int] = {}
    for entry in strip_brackets(raw).split(","):
        entry = entry.strip()
        if not entry:
            continue
        name, _, amount = entry.rpartition(" ")
        values[name.strip()] = int(amount)
    return Inventory(**{key: values.get(key, 0) for key in RESOURCES})
