from __future__ import annotations

from zappy_ai_rl.config.roles_config import RolesConfig
from zappy_ai_rl.decision import rl_policy
from zappy_ai_rl.decision.behaviour_tree import Node
from zappy_ai_rl.decision.roles import collector, forker, leader, survivor

_TREES: dict[str, Node] = {}
_POLICY: rl_policy.RLPolicy | None = None


def _base_cascade(bot, config: RolesConfig) -> str:
    if survivor.is_triggered(bot, config.survivor):
        return "SURVIVOR"
    if forker.is_triggered(bot, config.forker):
        return "FORKER"
    if leader.is_triggered(bot, config.leader):
        return "LEADER"
    return "COLLECTOR"


def _policy() -> rl_policy.RLPolicy:
    global _POLICY
    if _POLICY is None:
        _POLICY = rl_policy.RLPolicy.load()
    return _POLICY


def compute_role_name(bot, config: RolesConfig) -> str:
    policy = _policy()
    state = rl_policy.featurize(bot, config)
    if not policy.is_known(state):
        return _base_cascade(bot, config)
    return policy.select_action(state, explore=False)


def get_tree(role_name: str, config: RolesConfig) -> Node:
    if role_name not in _TREES:
        if role_name == "SURVIVOR":
            _TREES[role_name] = survivor.build_tree(config.survivor)
        elif role_name == "FORKER":
            _TREES[role_name] = forker.build_tree(config.forker)
        elif role_name == "LEADER":
            _TREES[role_name] = leader.build_tree(config.leader)
        else:
            _TREES[role_name] = collector.build_tree(config.collector)
    return _TREES[role_name]
