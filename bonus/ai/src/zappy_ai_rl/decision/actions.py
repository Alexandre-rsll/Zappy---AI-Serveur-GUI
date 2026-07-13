from __future__ import annotations

from zappy_ai_rl.decision.behaviour_tree import Status
from zappy_ai_rl.protocol import messages
from zappy_ai_rl.world.inventory import parse_inventory
from zappy_ai_rl.world.vision import parse_look


def move_forward(bot) -> Status:
    def _on_complete(lines: list[str]) -> None:
        if lines[-1] == "ok":
            bot.mental_map.move_forward()

    bot.send(messages.FORWARD, on_complete=_on_complete)
    return Status.RUNNING


def turn_right(bot) -> Status:
    def _on_complete(lines: list[str]) -> None:
        if lines[-1] == "ok":
            bot.mental_map.turn_right()

    bot.send(messages.RIGHT, on_complete=_on_complete)
    return Status.RUNNING


def turn_left(bot) -> Status:
    def _on_complete(lines: list[str]) -> None:
        if lines[-1] == "ok":
            bot.mental_map.turn_left()

    bot.send(messages.LEFT, on_complete=_on_complete)
    return Status.RUNNING


def look(bot) -> Status:
    def _on_complete(lines: list[str]) -> None:
        tiles = parse_look(lines[-1], bot.level)
        bot.last_look = tiles
        bot.mental_map.record_vision(tiles)

    bot.send(messages.LOOK, on_complete=_on_complete)
    return Status.RUNNING


def check_inventory(bot) -> Status:
    def _on_complete(lines: list[str]) -> None:
        bot.inventory = parse_inventory(lines[-1])

    bot.send(messages.INVENTORY, on_complete=_on_complete)
    return Status.RUNNING


def check_connect_nbr(bot) -> Status:
    def _on_complete(lines: list[str]) -> None:
        bot.free_slots = int(lines[-1])

    bot.send(messages.CONNECT_NBR, on_complete=_on_complete)
    return Status.RUNNING


def fork(bot) -> Status:
    def _on_complete(lines: list[str]) -> None:
        if lines[-1] == "ok":
            bot.free_slots += 1
            bot.forks_done += 1

    bot.send(messages.FORK, on_complete=_on_complete)
    return Status.RUNNING


def take(resource: str):
    def _action(bot) -> Status:
        def _on_complete(lines: list[str]) -> None:
            if lines[-1] != "ok":
                return
            setattr(bot.inventory, resource, bot.inventory.count(resource) + 1)
            if bot.last_look is not None and resource in bot.last_look[0].objects:
                bot.last_look[0].objects.remove(resource)

        bot.send(messages.take(resource), on_complete=_on_complete)
        return Status.RUNNING

    return _action


def set_down(resource: str):
    def _action(bot) -> Status:
        def _on_complete(lines: list[str]) -> None:
            if lines[-1] != "ok":
                return
            setattr(bot.inventory, resource, bot.inventory.count(resource) - 1)
            if bot.last_look is not None:
                bot.last_look[0].objects.append(resource)

        bot.send(messages.set_down(resource), on_complete=_on_complete)
        return Status.RUNNING

    return _action


def broadcast(text: str):
    def _action(bot) -> Status:
        bot.send(messages.broadcast(text))
        return Status.RUNNING

    return _action


def incantation(bot) -> Status:
    def _on_complete(lines: list[str]) -> None:
        if lines[-1].startswith("Current level:"):
            bot.level = int(lines[-1].split(":")[1].strip())
            bot.log.debug(f"incantation succeeded, now level {bot.level}")
        else:
            bot.log.debug("incantation failed")
        bot.last_look = None
        bot.depositing = False
        bot.leader_wait_ticks = 0

    bot.send(messages.INCANTATION, on_complete=_on_complete)
    return Status.RUNNING
