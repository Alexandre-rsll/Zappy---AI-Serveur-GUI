/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** Client
*/

#ifndef ZAPPY_CLIENT_HPP
#define ZAPPY_CLIENT_HPP

#include <chrono>
#include <queue>
#include <string>

#include "game/Trantorian.hpp"

namespace server {
    static constexpr std::size_t maxClientInput = 4096;
    static constexpr std::size_t maxPendingCommands = 10;

    enum class ClientState { PENDING, AI, GUI };

    struct PendingCommand {
        std::string raw;
        std::chrono::steady_clock::time_point readyAt;
        bool executing;
    };

    class Client {
    public:
        explicit Client(int fd);
        ~Client();

        [[nodiscard]] int fd() const;
        [[nodiscard]] int id() const;
        [[nodiscard]] ClientState state() const;
        [[nodiscard]] const std::string &teamName() const;
        [[nodiscard]] std::shared_ptr<Trantorian> player();
        [[nodiscard]] std::shared_ptr<const Trantorian> player() const;
        [[nodiscard]] std::size_t commandCount() const;

        void setState(ClientState state);
        void setTeamName(const std::string &teamName);
        void setPlayer(std::shared_ptr<Trantorian> player);

        bool readSocket();
        bool readLine(std::string &out);
        void pushResponse(const std::string &msg);
        bool sendWriteBuffer();
        [[nodiscard]] bool hasDataToWrite() const;

        [[nodiscard]] bool pushCommand(const std::string &raw);
        [[nodiscard]] bool hasCommand() const;
        [[nodiscard]] PendingCommand &frontCommand();
        void popCommand();
        void startCommand(double duration);

    private:
        int _fd;
        int _id;
        static int _nextId;
        ClientState _state = ClientState::PENDING;
        std::string _teamName;
        std::shared_ptr<Trantorian> _player;

        std::string _readBuffer;
        std::string _writeBuffer;
        std::queue<PendingCommand> _commandQueue;
    };
} // namespace server

#endif // ZAPPY_CLIENT_HPP
