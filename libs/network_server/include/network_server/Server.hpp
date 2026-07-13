/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Server
*/

#ifndef ZAPPY_SERVER_HPP
#define ZAPPY_SERVER_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <poll.h>

#include "Client.hpp"
#include "logger/Logger.hpp"
#include "protocol/Protocol.hpp"
#include "protocol/ProtocolCommandHandler.hpp"
#include "game/World.hpp"

#define UNUSED __attribute__((unused))

namespace server {
    static constexpr std::size_t endpointFailure = 84;

    static constexpr std::size_t maxClients = 100;
    static constexpr std::size_t defaultPort = 8080;
    static constexpr std::size_t defaultWidth = 30;
    static constexpr std::size_t defaultHeight = 10;
    static constexpr std::size_t defaultMaxTeamClient = 3;
    static const std::vector<std::string> defaultTeamNames = {"PULSE", "POC"};
    static constexpr std::size_t defaultFreq = 100;

    struct ServerConfig {
        std::size_t port = defaultPort;
        std::size_t width = defaultWidth;
        std::size_t height = defaultHeight;
        std::vector<std::string> teamNames = defaultTeamNames;
        std::size_t clientsNb = 0;
        std::size_t freq = defaultFreq;
    };

    class Server {
    public:
        explicit Server(ServerConfig config);
        ~Server();

        void run();

    private:
        void _setupServer();
        void _setupPoll();
        [[nodiscard]] int _computePollTimeout() const;

        void _processClients();
        void _processClient(const pollfd &pfd, Client &client);

        void _acceptNewClient();
        void _disconnectClient(int fd);

        void _readClient(Client &client);
        void _writeToClient(Client &client);

        void _assignClientToTeam(Client &client, const std::string &teamName);
        void _assignClientToTeamGUI(Client &client);

        void _handleGUICommand(Client &client, const std::string &line);
        void _guiSetTimeUnit(Client &client, const std::string &arg);
        void _guiSendTile(Client &client, const std::string &arg);

        void _pushAICommand(Client &client, const std::string &line);

        void _executeReadyCommands();
        void _beginCommand(Client &client, const std::string &raw);
        void _completeCommand(Client &client, const std::string &raw);
        void _tickWorld();
        void _handleDeaths();
        void _checkVictory();
        void _broadcastSound(const std::shared_ptr<Trantorian> &emitter,
                             int emitterId, const std::string &text);

        void _broadcastGUI(const std::string &msg);
        void _sendInitialState(Client &client);
        void _sendTeams(Client &client) const;
        void _sendPlayers(Client &client);
        void _sendEggs(Client &client);
        void _handleGUIPlayerQuery(Client &client, const std::string &verb,
                                   const std::string &arg);
        [[nodiscard]] std::string _mapTiles();
        [[nodiscard]] static protocol::Protocol::NewPlayer
        _playerInfo(Client &client);

        void _emitPlayerConnected(Client &client);
        void _emitAfterCommand(const std::string &name, const std::string &arg,
                               Client &client);
        std::vector<int> _tileMates(const std::shared_ptr<Trantorian> &emitter);
        std::vector<std::size_t>
        _tileEggIds(const std::shared_ptr<Trantorian> &emitter);
        void _ejectFanout(const std::shared_ptr<Trantorian> &emitter,
                          const std::vector<int> &victims,
                          const std::vector<std::size_t> &eggIds);
        void _startIncantation(Client &client,
                               const std::shared_ptr<Trantorian> &player);
        void _finishIncantation(Client &client,
                                const std::shared_ptr<Trantorian> &player);

        [[nodiscard]] protocol::Protocol::Quantities _tileQuantities(int x,
                                                                     int y);
        [[nodiscard]] Client *_clientByPlayerId(int n);
        [[nodiscard]] Client *
        _clientByPlayer(const std::shared_ptr<Trantorian> &player);
        [[nodiscard]] static int _orientation(const Trantorian &t);
        [[nodiscard]] static protocol::Protocol::Quantities
        _playerQuantities(const Trantorian &t);
        [[nodiscard]] static int _resourceIndex(const std::string &name);
        [[nodiscard]] static std::string _commandName(const std::string &raw);

        ServerConfig _cfg;
        int _serverFd = -1;

        std::vector<pollfd> _pollFds;
        std::unordered_map<int, std::unique_ptr<Client>> _clients;
        bool _pollChanges = false;

        protocol::ProtocolCommandHandler _commandHandler;
        World _world;

        logger::Logger _logger;
    };
} // namespace server

#endif // ZAPPY_SERVER_HPP
