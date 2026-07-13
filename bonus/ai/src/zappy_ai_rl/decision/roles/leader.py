from __future__ import annotations

from zappy_ai_rl.config.roles_config import LeaderConfig
from zappy_ai_rl.coordination import broadcast_codec
from zappy_ai_rl.decision import actions
from zappy_ai_rl.decision.behaviour_tree import Node, condition, selector, sequence
from zappy_ai_rl.decision.roles import collector
from zappy_ai_rl.protocol.constants import ELEVATION_REQUIREMENTS


def is_triggered(bot, config: LeaderConfig) -> bool:
    if bot.tick_count < bot.leader_cooldown_until:
        return False
    if not collector.stones_ready(bot):
        return False
    return not _outclassed(bot, config)


def _outclassed(bot, config: LeaderConfig) -> bool:
    own_count = collector.tile_count(bot, "player")
    rival_count = bot.team_knowledge.best_known_count(
        bot.level, bot.tick_count, config.rival_call_max_age_ticks
    )
    return rival_count >= own_count + config.yield_margin


def _ready_to_incant(bot) -> bool:
    requirement = ELEVATION_REQUIREMENTS.get(bot.level)
    if requirement is None or bot.last_look is None:
        return False
    return collector.tile_count(bot, "player") >= requirement.players


def _patience_exhausted(bot, config: LeaderConfig) -> bool:
    if bot.leader_wait_ticks < config.patience_ticks:
        return False
    bot.leader_cooldown_until = bot.tick_count + config.cooldown_ticks
    bot.leader_wait_ticks = 0
    bot.log.debug(f"LEADER giving up at level {bot.level}, cooling down")
    return True


def _recruit_step(bot, config: LeaderConfig):
    bot.leader_wait_ticks += 1
    if bot.leader_wait_ticks % config.broadcast_interval_ticks == 0:
        count = collector.tile_count(bot, "player")
        requirement = ELEVATION_REQUIREMENTS.get(bot.level)
        rally = (
            requirement is not None
            and requirement.players >= config.rally_min_players
        )
        return actions.broadcast(
            broadcast_codec.encode_recruit(bot.level, count, rally)
        )(bot)
    return actions.look(bot)


def build_tree(config: LeaderConfig) -> Node:
    return selector(
        sequence(condition(_ready_to_incant), actions.incantation),
        sequence(
            condition(lambda bot: _patience_exhausted(bot, config)), actions.look
        ),
        lambda bot: _recruit_step(bot, config),
    )
