from __future__ import annotations

from zappy_ai_rl.config.roles_config import CollectorConfig
from zappy_ai_rl.decision import actions
from zappy_ai_rl.decision.behaviour_tree import Node, condition, selector, sequence
from zappy_ai_rl.decision.navigation import seek_resource
from zappy_ai_rl.protocol.constants import ELEVATION_REQUIREMENTS

_TRACKED_RESOURCES = (
    "linemate",
    "deraumere",
    "sibur",
    "mendiane",
    "phiras",
    "thystame",
)


def tile_count(bot, token: str) -> int:
    if bot.last_look is None:
        return 0
    return bot.last_look[0].objects.count(token)


def stones_ready(bot) -> bool:
    requirement = ELEVATION_REQUIREMENTS.get(bot.level)
    if requirement is None or bot.last_look is None:
        return False
    return all(
        tile_count(bot, resource) >= getattr(requirement, resource)
        for resource in _TRACKED_RESOURCES
    )


def missing_resource(bot) -> str | None:
    requirement = ELEVATION_REQUIREMENTS.get(bot.level)
    if requirement is None:
        return None
    for resource in _TRACKED_RESOURCES:
        if bot.inventory.count(resource) < getattr(requirement, resource):
            return resource
    return None


def _has_target_here(bot) -> bool:
    target = missing_resource(bot)
    return (
        target is not None
        and bot.last_look is not None
        and target in bot.last_look[0].objects
    )


def _take_target(bot):
    return actions.take(missing_resource(bot))(bot)


def _seek_target(bot):
    return seek_resource(bot, missing_resource(bot))


def _resource_to_deposit(bot) -> str | None:
    requirement = ELEVATION_REQUIREMENTS.get(bot.level)
    if requirement is None or bot.last_look is None:
        return None
    for resource in _TRACKED_RESOURCES:
        needed = getattr(requirement, resource)
        if (
            needed > 0
            and tile_count(bot, resource) < needed
            and bot.inventory.count(resource) > 0
        ):
            return resource
    return None


def _recruit_heading(bot, config: CollectorConfig) -> int | None:
    return bot.team_knowledge.recruit_heading(
        bot.level, bot.tick_count, config.recruit_call_max_age_ticks
    )


def _step_towards_sound(bot, direction: int):
    if direction in (1, 2, 8):
        return actions.move_forward(bot)
    if direction in (3, 4, 5):
        return actions.turn_right(bot)
    return actions.turn_left(bot)


def _deposit_here_or_look(bot):
    if bot.last_look is None:
        return actions.look(bot)
    resource = _resource_to_deposit(bot)
    if resource is not None:
        return actions.set_down(resource)(bot)
    return actions.look(bot)


def _waiting_at_rendezvous(bot, config: CollectorConfig) -> bool:
    return _recruit_heading(bot, config) == 0


def _answering_rally(bot, config: CollectorConfig) -> bool:
    return bot.team_knowledge.committed_is_rally(
        bot.level, bot.tick_count, config.recruit_call_max_age_ticks
    )


def _rally_step(bot, config: CollectorConfig):
    heading = _recruit_heading(bot, config)
    if heading is None or heading == 0:
        return _deposit_here_or_look(bot)
    bot.last_look = None
    return _step_towards_sound(bot, heading)


def _beaconing(bot, config: CollectorConfig) -> bool:
    if _recruit_heading(bot, config) is not None:
        bot.depositing = False
        return False
    pos = bot.mental_map.position()
    if bot.depositing:
        if bot.deposit_position == pos:
            return True
        bot.depositing = False
    if missing_resource(bot) is None:
        bot.depositing = True
        bot.deposit_position = pos
        return True
    return False


def _regroup_step(bot, config: CollectorConfig):
    heading = _recruit_heading(bot, config)
    if heading is not None and heading != 0:
        bot.last_look = None
        return _step_towards_sound(bot, heading)
    return _deposit_here_or_look(bot)


def build_tree(config: CollectorConfig) -> Node:
    return selector(
        sequence(
            condition(lambda bot: _waiting_at_rendezvous(bot, config)),
            _deposit_here_or_look,
        ),
        sequence(
            condition(lambda bot: _answering_rally(bot, config)),
            lambda bot: _rally_step(bot, config),
        ),
        sequence(
            condition(lambda bot: _beaconing(bot, config)),
            _deposit_here_or_look,
        ),
        sequence(
            condition(lambda bot: missing_resource(bot) is None),
            lambda bot: _regroup_step(bot, config),
        ),
        sequence(condition(_has_target_here), _take_target),
        _seek_target,
    )
