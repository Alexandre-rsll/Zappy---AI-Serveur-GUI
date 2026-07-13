/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** ProtocolCommandHandler
*/

#ifndef ZAPPY_PROTOCOLCOMMANDHANDLER_HPP
#define ZAPPY_PROTOCOLCOMMANDHANDLER_HPP

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "game/World.hpp"

namespace protocol {
    static constexpr std::size_t defaultTimeCost = 7;
    static constexpr std::size_t inventoryTimeCost = 1;
    static constexpr std::size_t nullTimeCost = 0;
    static constexpr std::size_t forkTimeCost = 42;
    static constexpr std::size_t incantationTimeCost = 300;

    using Player = std::shared_ptr<Trantorian>;
    using CommandHandler = std::function<std::string(
        const std::string &args, const Player &t, World &world)>;

    struct Command {
        int timeCost;
        CommandHandler handler;
    };

    class ProtocolCommandHandler {
    public:
        ProtocolCommandHandler();
        ~ProtocolCommandHandler() = default;

        [[nodiscard]] double getDuration(const std::string &raw,
                                         int freq) const;
        [[nodiscard]] std::string execute(const std::string &raw,
                                          const Player &t, World &world) const;

    private:
        void _loadAllCommands();

        static std::string _getCmdName(const std::string &raw);
        static std::string _getCmdArgs(const std::string &raw);
        static std::optional<ResourceType>
        _parseResource(const std::string &name);
        static std::string _formatInventory(const Trantorian &t);

        static std::string _forward(const std::string &args, const Player &t,
                                    World &world);
        static std::string _right(const std::string &args, const Player &t,
                                  World &world);
        static std::string _left(const std::string &args, const Player &t,
                                 World &world);
        static std::string _look(const std::string &args, const Player &t,
                                 World &world);
        static std::string _inventory(const std::string &args, const Player &t,
                                      World &world);
        static std::string _broadcast(const std::string &args, const Player &t,
                                      World &world);
        static std::string _connectNbr(const std::string &args, const Player &t,
                                       World &world);
        static std::string _fork(const std::string &args, const Player &t,
                                 World &world);
        static std::string _eject(const std::string &args, const Player &t,
                                  World &world);
        static std::string _take(const std::string &args, const Player &t,
                                 World &world);
        static std::string _set(const std::string &args, const Player &t,
                                World &world);

        std::unordered_map<std::string, Command> _commands;
    };
} // namespace protocol
#endif // ZAPPY_PROTOCOLCOMMANDHANDLER_HPP
