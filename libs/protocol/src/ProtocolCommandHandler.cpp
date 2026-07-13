/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** ProtocolCommandHandler
*/

#include <algorithm>

#include "protocol/ProtocolCommandHandler.hpp"

namespace protocol {
    ProtocolCommandHandler::ProtocolCommandHandler()
    {
        _loadAllCommands();
    }

    void ProtocolCommandHandler::_loadAllCommands()
    {
        _commands["forward"] = {.timeCost = defaultTimeCost,
                                .handler = _forward};
        _commands["right"] = {.timeCost = defaultTimeCost, .handler = _right};
        _commands["left"] = {.timeCost = defaultTimeCost, .handler = _left};
        _commands["look"] = {.timeCost = defaultTimeCost, .handler = _look};
        _commands["inventory"] = {.timeCost = inventoryTimeCost,
                                  .handler = _inventory};
        _commands["broadcast"] = {.timeCost = defaultTimeCost,
                                  .handler = _broadcast};
        _commands["connect_nbr"] = {.timeCost = nullTimeCost,
                                    .handler = _connectNbr};
        _commands["fork"] = {.timeCost = forkTimeCost, .handler = _fork};
        _commands["eject"] = {.timeCost = defaultTimeCost, .handler = _eject};
        _commands["take"] = {.timeCost = defaultTimeCost, .handler = _take};
        _commands["set"] = {.timeCost = defaultTimeCost, .handler = _set};
    }

    std::string ProtocolCommandHandler::_getCmdName(const std::string &raw)
    {
        const auto space = raw.find(' ');
        std::string name =
            space == std::string::npos ? raw : raw.substr(0, space);
        std::ranges::transform(name, name.begin(), tolower);
        return name;
    }

    std::string ProtocolCommandHandler::_getCmdArgs(const std::string &raw)
    {
        const auto space = raw.find(' ');
        return space == std::string::npos ? "" : raw.substr(space + 1);
    }

    double ProtocolCommandHandler::getDuration(const std::string &raw,
                                               const int freq) const
    {
        const auto it = _commands.find(_getCmdName(raw));
        if (it == _commands.end()) {
            return 0.0;
        }
        return static_cast<double>(it->second.timeCost) / freq;
    }

    std::string
    ProtocolCommandHandler::execute(const std::string &raw,
                                    const std::shared_ptr<Trantorian> &t,
                                    World &world) const
    {
        const auto it = _commands.find(_getCmdName(raw));
        if (it == _commands.end()) {
            return "ko\n";
        }
        return it->second.handler(_getCmdArgs(raw), t, world);
    }
} // namespace protocol
