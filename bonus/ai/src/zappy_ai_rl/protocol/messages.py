from __future__ import annotations

FORWARD = "Forward"
RIGHT = "Right"
LEFT = "Left"
LOOK = "Look"
INVENTORY = "Inventory"
CONNECT_NBR = "Connect_nbr"
FORK = "Fork"
EJECT = "Eject"
INCANTATION = "Incantation"


def take(object_name: str) -> str:
    return f"Take {object_name}"


def set_down(object_name: str) -> str:
    return f"Set {object_name}"


def broadcast(text: str) -> str:
    return f"Broadcast {text}"


def is_response_terminal(command: str, lines: list[str]) -> bool:
    if command == INCANTATION and len(lines) == 1:
        return lines[0] != "Elevation underway"
    return True
