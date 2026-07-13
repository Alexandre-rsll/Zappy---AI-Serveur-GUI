/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** ProtocolCommands
*/

#include "protocol/ProtocolCommandHandler.hpp"

namespace protocol {
    std::string ProtocolCommandHandler::_forward(const std::string &args,
                                                 const Player &t, World &world)
    {
        world.moveTrantorian(t, std::nullopt);
        return "ok\n";
    }

    std::string ProtocolCommandHandler::_right(const std::string &args,
                                               const Player &t, World &world)
    {
        t->turnRight();
        return "ok\n";
    }

    std::string ProtocolCommandHandler::_left(const std::string &args,
                                              const Player &t, World &world)
    {
        t->turnLeft();
        return "ok\n";
    }

    std::string ProtocolCommandHandler::_look(const std::string &args,
                                              const Player &t, World &world)
    {
        return t->look(world) + "\n";
    }

    std::string ProtocolCommandHandler::_inventory(const std::string &args,
                                                   const Player &t,
                                                   World &world)
    {
        return _formatInventory(*t);
    }

    std::string ProtocolCommandHandler::_broadcast(const std::string &args,
                                                   const Player &t,
                                                   World &world)
    {
        return "ok\n";
    }

    std::string ProtocolCommandHandler::_connectNbr(const std::string &args,
                                                    const Player &t,
                                                    World &world)
    {
        return std::to_string(world.connectNbr(t->getTeam())) + "\n";
    }

    std::string ProtocolCommandHandler::_fork(const std::string &args,
                                              const Player &t, World &world)
    {
        world.fork(t);
        return "ok\n";
    }

    std::string ProtocolCommandHandler::_eject(const std::string &args,
                                               const Player &t, World &world)
    {
        return t->eject(world) ? "ok\n" : "ko\n";
    }

    std::string ProtocolCommandHandler::_take(const std::string &args,
                                              const Player &t, World &world)
    {
        const auto object = _parseResource(args);

        if (!object || !t->take(world, *object)) {
            return "ko\n";
        }
        return "ok\n";
    }

    std::string ProtocolCommandHandler::_set(const std::string &args,
                                             const Player &t, World &world)
    {
        const auto object = _parseResource(args);

        if (!object || !t->set(world, *object)) {
            return "ko\n";
        }
        return "ok\n";
    }

    std::optional<ResourceType>
    ProtocolCommandHandler::_parseResource(const std::string &name)
    {
        for (std::size_t i = 0; i < RESOURCE_COUNT; ++i) {
            if (RESOURCE_NAME.at(i) == name) {
                return static_cast<ResourceType>(i);
            }
        }
        return std::nullopt;
    }

    std::string ProtocolCommandHandler::_formatInventory(const Trantorian &t)
    {
        const auto &inventory = t.getInventory();
        std::string out = "[";

        for (std::size_t i = 0; i < RESOURCE_COUNT; ++i) {
            const auto type = static_cast<ResourceType>(i);
            const std::size_t count =
                inventory.contains(type) ? inventory.at(type) : 0;
            out += RESOURCE_NAME.at(i) + " " + std::to_string(count);
            if (i + 1 < RESOURCE_COUNT) {
                out += ", ";
            }
        }
        out += "]\n";
        return out;
    }
} // namespace protocol
