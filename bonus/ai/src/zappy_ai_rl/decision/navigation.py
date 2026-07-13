from __future__ import annotations

from typing import Callable

from zappy_ai_rl.decision import actions
from zappy_ai_rl.decision.behaviour_tree import Status
from zappy_ai_rl.world.vision import Tile

Predicate = Callable[[Tile], bool]


def nearest_tile(tiles: list[Tile], predicate: Predicate) -> Tile | None:
    matches = [tile for tile in tiles if predicate(tile)]
    if not matches:
        return None
    return min(matches, key=lambda tile: (tile.forward, abs(tile.lateral)))


def _step_towards(bot, forward: int, lateral: int) -> Status:
    if forward == 0 and lateral == 0:
        return Status.FAILURE
    if abs(lateral) < forward:
        return actions.move_forward(bot)
    bot.nav_force_forward = True
    if lateral < 0:
        return actions.turn_left(bot)
    return actions.turn_right(bot)


def navigate_towards(bot, predicate: Predicate) -> Status:
    if bot.nav_force_forward:
        bot.nav_force_forward = False
        return actions.move_forward(bot)

    if bot.last_look is None:
        return actions.look(bot)

    target = nearest_tile(bot.last_look, predicate)
    bot.last_look = None
    if target is None:
        return actions.move_forward(bot)
    return _step_towards(bot, target.forward, target.lateral)


def navigate_towards_position(bot, target: tuple[int, int]) -> Status:
    if bot.nav_force_forward:
        bot.nav_force_forward = False
        return actions.move_forward(bot)

    forward, lateral = bot.mental_map.heading_to(target)
    return _step_towards(bot, forward, lateral)


def seek_resource(bot, resource: str) -> Status:
    pos = bot.mental_map.position()

    if bot.seek_position == pos:
        bot.seek_position = None
        bot.last_look = None
        return actions.look(bot)

    if bot.last_look is not None and any(
        resource in tile.objects for tile in bot.last_look
    ):
        bot.seek_position = None
        return navigate_towards(bot, lambda tile: resource in tile.objects)

    if bot.seek_position is None:
        bot.seek_position = bot.mental_map.nearest_known(resource)
    if bot.seek_position is not None:
        bot.last_look = None
        return navigate_towards_position(bot, bot.seek_position)

    return navigate_towards(bot, lambda tile: resource in tile.objects)
