/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** Server
*/

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <iostream>
#include <stdexcept>
#include <csignal>
#include <ranges>
#include <algorithm>
#include <sstream>
#include <cctype>
#include <utility>

#include "network_server/Server.hpp"
#include "protocol/Protocol.hpp"
#include "protocol/ProtocolCommandHandler.hpp"
#include "game/World.hpp"

namespace server {
    static volatile bool running = false;

    void stop(UNUSED int sig)
    {
        running = false;
    }

    Server::Server(ServerConfig config)
        : _cfg(std::move(config)),
          _world(_cfg.width, _cfg.height, _cfg.teamNames, _cfg.clientsNb)
    {
        _logger.info() << "Creating server" << std::endl;
        _world.generateResources();
        _setupServer();
        _setupPoll();
        running = true;
    }

    Server::~Server()
    {
        if (_serverFd >= 0) {
            close(_serverFd);
            std::cout << std::endl;
            _logger.info() << "Closing server" << std::endl;
        }
    }

    void Server::_setupServer()
    {
        _serverFd = socket(AF_INET, SOCK_STREAM, 0);
        if (_serverFd <= 0) {
            throw std::runtime_error("Unable to create socket");
        }

        int opt = 1;
        setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(int));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(_cfg.port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(_serverFd, reinterpret_cast<sockaddr *>(&addr),
                 sizeof(addr)) == -1) {
            std::cerr << _cfg.port << std::endl;
            throw std::runtime_error("Unable to bind fd");
        }
        if (listen(_serverFd, maxClients) < 0) {
            throw std::runtime_error("Unable to listen on fd");
        }

        _logger.info() << "Listening on port " << _cfg.port << std::endl;
    }

    void Server::_setupPoll()
    {
        _pollFds.clear();
        _pollFds.push_back({.fd = _serverFd, .events = POLLIN, .revents = 0});

        for (auto &[fd, client] : _clients) {
            short events = POLLIN;
            if (client->hasDataToWrite()) {
                events |= POLLOUT;
            }
            _pollFds.push_back({.fd = fd, .events = events, .revents = 0});
        }
        _pollChanges = false;
    }

    int Server::_computePollTimeout() const
    {
        auto soonest = std::chrono::steady_clock::time_point::max();
        bool found = false;

        for (const auto &client : _clients | std::views::values) {
            if (client->state() != ClientState::AI || !client->hasCommand()) {
                continue;
            }
            const auto &cmd = client->frontCommand();
            if (cmd.executing && cmd.readyAt < soonest) {
                soonest = cmd.readyAt;
                found = true;
            }
        }
        if (!found) {
            return -1;
        }
        const auto now = std::chrono::steady_clock::now();
        if (soonest <= now) {
            return 0;
        }
        return static_cast<int>(
            std::chrono::duration_cast<std::chrono::milliseconds>(soonest - now)
                .count());
    }

    void Server::run()
    {
        signal(SIGINT, stop);
        while (running) {
            if (_pollChanges) {
                _setupPoll();
            }
            const int worldMs =
                _world.nextSpawnDelayMs(static_cast<int>(_cfg.freq));
            int timeout = _computePollTimeout();
            if (timeout < 0 || worldMs < timeout) {
                timeout = worldMs;
            }
            if (poll(_pollFds.data(), _pollFds.size(), timeout) < 0) {
                continue;
            }
            _processClients();
            _executeReadyCommands();
            _tickWorld();
            _handleDeaths();
            _checkVictory();
        }
    }

    void Server::_processClients()
    {
        const std::size_t nbPfds = _pollFds.size();
        for (std::size_t i = 0; i < nbPfds; ++i) {
            auto &pfd = _pollFds.at(i);
            if (pfd.revents == 0) {
                continue;
            }
            if (pfd.fd == _serverFd) {
                _acceptNewClient();
                continue;
            }

            if ((pfd.revents & (POLLERR | POLLHUP)) != 0) {
                _disconnectClient(pfd.fd);
                continue;
            }

            auto it = _clients.find(pfd.fd);
            if (it == _clients.end()) {
                continue;
            }
            Client &client = *it->second;
            _processClient(pfd, client);
        }
    }

    void Server::_processClient(const pollfd &pfd, Client &client)
    {
        if ((pfd.revents & POLLIN) != 0) {
            _readClient(client);
            if (!_clients.contains(pfd.fd)) {
                return;
            }
        }

        if ((pfd.revents & POLLOUT) != 0) {
            _writeToClient(client);
        }
    }

    void Server::_acceptNewClient()
    {
        sockaddr_in addr{};
        socklen_t len = sizeof(addr);
        int newFd =
            accept(_serverFd, reinterpret_cast<sockaddr *>(&addr), &len);
        if (newFd < 0) {
            return;
        }

        auto client = std::make_unique<Client>(newFd);
        client->pushResponse("WELCOME\n");
        _logger.info() << "New connection fd=" << newFd << std::endl;
        _clients.emplace(newFd, std::move(client));
        _pollChanges = true;
    }

    void Server::_disconnectClient(const int fd)
    {
        const auto it = _clients.find(fd);
        if (it == _clients.end()) {
            return;
        }

        if (const auto &player = it->second->player()) {
            Tile &tile = _world.getTile(player->getX(), player->getY());
            std::erase(tile.trantorian, player);
        }
        _logger.info() << "Disconnected fd=" << fd << std::endl;
        _clients.erase(it);
        _pollChanges = true;
    }

    void Server::_readClient(Client &client)
    {
        if (!client.readSocket()) {
            _disconnectClient(client.fd());
            return;
        }

        const int fd = client.fd();
        std::string line;
        while (client.readLine(line)) {
            switch (client.state()) {
            case ClientState::PENDING:
                _assignClientToTeam(client, line);
                break;
            case ClientState::GUI:
                _handleGUICommand(client, line);
                break;
            case ClientState::AI:
                _pushAICommand(client, line);
                break;
            }
            if (!_clients.contains(fd)) {
                return;
            }
        }
    }

    void Server::_writeToClient(Client &client)
    {
        if (!client.sendWriteBuffer()) {
            _disconnectClient(client.fd());
            return;
        }
        if (!client.hasDataToWrite()) {
            _pollChanges = true;
        }
    }

    void Server::_assignClientToTeam(Client &client,
                                     const std::string &teamName)
    {
        if (teamName == "GRAPHIC") {
            _assignClientToTeamGUI(client);
            return;
        }
        if (std::ranges::find(_cfg.teamNames, teamName) ==
                _cfg.teamNames.end() ||
            _world.connectNbr(teamName) == 0) {
            _disconnectClient(client.fd());
            return;
        }
        client.setState(ClientState::AI);
        client.setTeamName(teamName);
        client.setPlayer(_world.hatchEgg(teamName));
        client.pushResponse(std::to_string(_world.connectNbr(teamName)) + "\n");
        client.pushResponse(std::to_string(_cfg.width) + " " +
                            std::to_string(_cfg.height) + "\n");
        _broadcastGUI(protocol::Protocol::ebo(
            static_cast<int>(_world.lastHatchedEggId())));
        _emitPlayerConnected(client);
        _pollChanges = true;
    }

    void Server::_assignClientToTeamGUI(Client &client)
    {
        client.setState(ClientState::GUI);
        _logger.info() << "fd=" << client.fd() << " is a GUI client"
                       << std::endl;

        _sendInitialState(client);
        _pollChanges = true;
    }

    void Server::_handleGUICommand(Client &client, const std::string &line)
    {
        const auto [verb, arg] = protocol::Protocol::splitVerbArg(line);

        if (verb == "msz") {
            client.pushResponse(protocol::Protocol::msz(
                static_cast<int>(_cfg.width), static_cast<int>(_cfg.height)));
        } else if (verb == "sgt") {
            client.pushResponse(
                protocol::Protocol::sgt(static_cast<int>(_cfg.freq)));
        } else if (verb == "tna") {
            _sendTeams(client);
        } else if (verb == "sst") {
            _guiSetTimeUnit(client, arg);
        } else if (verb == "bct") {
            _guiSendTile(client, arg);
        } else if (verb == "mct") {
            client.pushResponse(_mapTiles());
        } else if (verb == "ppo" || verb == "plv" || verb == "pin") {
            _handleGUIPlayerQuery(client, verb, arg);
        } else {
            client.pushResponse(protocol::Protocol::suc());
        }
        _pollChanges = true;
    }

    void Server::_guiSetTimeUnit(Client &client, const std::string &arg)
    {
        std::size_t freq = 0;

        if (std::istringstream(arg) >> freq) {
            _cfg.freq = freq;
        }
        client.pushResponse(
            protocol::Protocol::sst(static_cast<int>(_cfg.freq)));
    }

    void Server::_guiSendTile(Client &client, const std::string &arg)
    {
        int x = 0;
        int y = 0;
        std::istringstream(arg) >> x >> y;
        if (x < 0 || y < 0 || std::cmp_greater_equal(x, _cfg.width) ||
            std::cmp_greater_equal(y, _cfg.height)) {
            client.pushResponse(protocol::Protocol::sbp());
            return;
        }
        client.pushResponse(
            protocol::Protocol::bct(x, y, _tileQuantities(x, y)));
    }

    void Server::_pushAICommand(Client &client, const std::string &line)
    {
        if (line.empty()) {
            return;
        }
        if (!client.pushCommand(line)) {
            _logger.info() << "queue full for fd=" << client.fd()
                           << ", dropping: " << line << std::endl;
        }
    }

    std::string Server::_commandName(const std::string &raw)
    {
        std::string name = protocol::Protocol::splitVerbArg(raw).first;

        std::ranges::transform(name, name.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return name;
    }

    void Server::_executeReadyCommands()
    {
        const auto now = std::chrono::steady_clock::now();

        for (const auto &client : _clients | std::views::values) {
            if (client->state() != ClientState::AI || !client->hasCommand()) {
                continue;
            }
            const auto &cmd = client->frontCommand();
            if (!cmd.executing) {
                _beginCommand(*client, cmd.raw);
            } else if (now >= cmd.readyAt) {
                _completeCommand(*client, cmd.raw);
            }
        }
    }

    void Server::_beginCommand(Client &client, const std::string &raw)
    {
        const auto player = client.player();

        if (player && player->isFrozen()) {
            return;
        }
        if (_commandName(raw) == "incantation") {
            _startIncantation(client, player);
            return;
        }
        client.startCommand(
            _commandHandler.getDuration(raw, static_cast<int>(_cfg.freq)));
    }

    void Server::_completeCommand(Client &client, const std::string &raw)
    {
        const auto player = client.player();
        const std::string name = _commandName(raw);
        const std::string arg = protocol::Protocol::splitVerbArg(raw).second;

        if (name == "incantation") {
            _finishIncantation(client, player);
            client.popCommand();
            _pollChanges = true;
            return;
        }
        std::vector<int> victims;
        std::vector<std::size_t> eggIds;
        if (name == "eject" && player) {
            victims = _tileMates(player);
            eggIds = _tileEggIds(player);
        }
        const std::string response =
            _commandHandler.execute(raw, player, _world);
        client.pushResponse(response);
        if (name == "broadcast" && player) {
            _broadcastSound(player, client.id(), arg);
        } else if (name == "eject" && player && response == "ok\n") {
            _ejectFanout(player, victims, eggIds);
        } else if (response == "ok\n") {
            _emitAfterCommand(name, arg, client);
        }
        client.popCommand();
        _pollChanges = true;
    }

    void Server::_tickWorld()
    {
        const int freq = static_cast<int>(_cfg.freq);

        if (_world.checkTime(freq)) {
            _broadcastGUI(_mapTiles());
        }
        _world.checkLives(freq);
    }

    void Server::_handleDeaths()
    {
        std::vector<int> deadFds;

        for (const auto &client : _clients | std::views::values) {
            if (client->state() == ClientState::AI && client->player() &&
                client->player()->getLife() == 0) {
                deadFds.push_back(client->fd());
            }
        }
        for (const int fd : deadFds) {
            const auto it = _clients.find(fd);
            if (it == _clients.end()) {
                continue;
            }
            it->second->pushResponse("dead\n");
            it->second->sendWriteBuffer();
            _broadcastGUI(protocol::Protocol::pdi(it->second->id()));
            _disconnectClient(fd);
        }
    }

    void Server::_broadcastSound(const std::shared_ptr<Trantorian> &emitter,
                                 const int emitterId, const std::string &text)
    {
        const std::size_t width = _world.getWidth();
        const std::size_t height = _world.getHeight();

        for (const auto &client : _clients | std::views::values) {
            if (client->state() != ClientState::AI || !client->player()) {
                continue;
            }
            const int direction = client->player()->calculAfectedTile(
                emitter->getX(), emitter->getY(), width, height);
            client->pushResponse("message " + std::to_string(direction) + ", " +
                                 text + "\n");
        }
        _broadcastGUI(protocol::Protocol::pbc(emitterId, text));
        _pollChanges = true;
    }

    void Server::_broadcastGUI(const std::string &msg)
    {
        for (const auto &client : _clients | std::views::values) {
            if (client->state() == ClientState::GUI) {
                client->pushResponse(msg);
                _pollChanges = true;
            }
        }
    }

    void Server::_checkVictory()
    {
        for (const auto &team : _cfg.teamNames) {
            if (_world.victoryCondition(team)) {
                _broadcastGUI(protocol::Protocol::seg(team));
                running = false;
                return;
            }
        }
    }

    int Server::_orientation(const Trantorian &t)
    {
        const DIRECTION dir = t.getDirection();

        if (dir == RIGHT) {
            return 2;
        }
        if (dir == DOWN) {
            return 3;
        }
        if (dir == LEFT) {
            return 4;
        }
        return 1;
    }

    protocol::Protocol::Quantities Server::_tileQuantities(const int x,
                                                           const int y)
    {
        protocol::Protocol::Quantities quantities{};
        const Tile &tile = _world.getTile(static_cast<std::size_t>(x),
                                          static_cast<std::size_t>(y));

        for (const auto &[type, count] : tile.resources) {
            quantities.at(static_cast<std::size_t>(type)) = count;
        }
        return quantities;
    }

    protocol::Protocol::Quantities
    Server::_playerQuantities(const Trantorian &t)
    {
        protocol::Protocol::Quantities quantities{};

        for (const auto &[type, count] : t.getInventory()) {
            quantities.at(static_cast<std::size_t>(type)) = count;
        }
        return quantities;
    }

    int Server::_resourceIndex(const std::string &name)
    {
        for (std::size_t i = 0; i < RESOURCE_COUNT; ++i) {
            if (RESOURCE_NAME.at(i) == name) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    Client *Server::_clientByPlayerId(const int n)
    {
        for (const auto &client : _clients | std::views::values) {
            if (client->state() == ClientState::AI && client->id() == n) {
                return client.get();
            }
        }
        return nullptr;
    }

    Client *Server::_clientByPlayer(const std::shared_ptr<Trantorian> &player)
    {
        for (const auto &client : _clients | std::views::values) {
            if (client->player() == player) {
                return client.get();
            }
        }
        return nullptr;
    }

    void Server::_emitPlayerConnected(Client &client)
    {
        if (client.player()) {
            _broadcastGUI(protocol::Protocol::pnw(_playerInfo(client)));
        }
    }

    protocol::Protocol::NewPlayer Server::_playerInfo(Client &client)
    {
        const auto player = client.player();

        return {.n = client.id(),
                .x = static_cast<int>(player->getX()),
                .y = static_cast<int>(player->getY()),
                .o = _orientation(*player),
                .l = static_cast<int>(player->getLvl()),
                .team = player->getTeam()};
    }

    std::string Server::_mapTiles()
    {
        std::string out;

        for (std::size_t y = 0; y < _cfg.height; ++y) {
            for (std::size_t x = 0; x < _cfg.width; ++x) {
                const int ix = static_cast<int>(x);
                const int iy = static_cast<int>(y);
                out += protocol::Protocol::bct(ix, iy, _tileQuantities(ix, iy));
            }
        }
        return out;
    }

    void Server::_sendTeams(Client &client) const
    {
        for (const auto &team : _cfg.teamNames) {
            client.pushResponse(protocol::Protocol::tna(team));
        }
    }

    void Server::_sendPlayers(Client &client)
    {
        for (const auto &other : _clients | std::views::values) {
            if (other->state() == ClientState::AI && other->player()) {
                client.pushResponse(
                    protocol::Protocol::pnw(_playerInfo(*other)));
            }
        }
    }

    void Server::_sendEggs(Client &client)
    {
        for (std::size_t y = 0; y < _cfg.height; ++y) {
            for (std::size_t x = 0; x < _cfg.width; ++x) {
                for (const auto &egg : _world.getTile(x, y).eggs) {
                    client.pushResponse(protocol::Protocol::enw(
                        static_cast<int>(egg->id), -1, static_cast<int>(x),
                        static_cast<int>(y)));
                }
            }
        }
    }

    void Server::_sendInitialState(Client &client)
    {
        client.pushResponse(protocol::Protocol::msz(
            static_cast<int>(_cfg.width), static_cast<int>(_cfg.height)));
        client.pushResponse(
            protocol::Protocol::sgt(static_cast<int>(_cfg.freq)));
        _sendTeams(client);
        client.pushResponse(_mapTiles());
        _sendPlayers(client);
        _sendEggs(client);
    }

    void Server::_handleGUIPlayerQuery(Client &client, const std::string &verb,
                                       const std::string &arg)
    {
        std::string idStr = arg;
        if (!idStr.empty() && idStr.front() == '#') {
            idStr.erase(0, 1);
        }
        int n = 0;
        if (!(std::istringstream(idStr) >> n)) {
            return;
        }
        const Client *target = _clientByPlayerId(n);
        if (target == nullptr || !target->player()) {
            return;
        }
        const auto player = target->player();
        const int x = static_cast<int>(player->getX());
        const int y = static_cast<int>(player->getY());
        if (verb == "ppo") {
            client.pushResponse(
                protocol::Protocol::ppo(n, x, y, _orientation(*player)));
        } else if (verb == "plv") {
            client.pushResponse(
                protocol::Protocol::plv(n, static_cast<int>(player->getLvl())));
        } else {
            client.pushResponse(
                protocol::Protocol::pin(n, x, y, _playerQuantities(*player)));
        }
    }

    void Server::_emitAfterCommand(const std::string &name,
                                   const std::string &arg, Client &client)
    {
        const auto player = client.player();
        if (!player) {
            return;
        }
        const int n = client.id();
        const int x = static_cast<int>(player->getX());
        const int y = static_cast<int>(player->getY());

        if (name == "forward" || name == "right" || name == "left") {
            _broadcastGUI(
                protocol::Protocol::ppo(n, x, y, _orientation(*player)));
            return;
        }
        if (name == "fork") {
            _broadcastGUI(protocol::Protocol::pfk(n));
            const Tile &tile = _world.getTile(static_cast<std::size_t>(x),
                                              static_cast<std::size_t>(y));
            if (!tile.eggs.empty()) {
                _broadcastGUI(protocol::Protocol::enw(
                    static_cast<int>(tile.eggs.back()->id), n, x, y));
            }
            return;
        }
        if (name != "take" && name != "set") {
            return;
        }
        const int idx = _resourceIndex(arg);
        if (idx >= 0) {
            _broadcastGUI(name == "take" ? protocol::Protocol::pgt(n, idx)
                                         : protocol::Protocol::pdr(n, idx));
        }
        _broadcastGUI(protocol::Protocol::bct(x, y, _tileQuantities(x, y)));
        _broadcastGUI(
            protocol::Protocol::pin(n, x, y, _playerQuantities(*player)));
    }

    std::vector<int>
    Server::_tileMates(const std::shared_ptr<Trantorian> &emitter)
    {
        std::vector<int> mates;

        for (const auto &client : _clients | std::views::values) {
            const auto player = client->player();
            if (client->state() == ClientState::AI && player &&
                player != emitter && player->getX() == emitter->getX() &&
                player->getY() == emitter->getY()) {
                mates.push_back(client->id());
            }
        }
        return mates;
    }

    std::vector<std::size_t>
    Server::_tileEggIds(const std::shared_ptr<Trantorian> &emitter)
    {
        std::vector<std::size_t> ids;
        const Tile &tile = _world.getTile(emitter->getX(), emitter->getY());

        ids.reserve(tile.eggs.size());
        for (const auto &egg : tile.eggs) {
            ids.push_back(egg->id);
        }
        return ids;
    }

    void Server::_ejectFanout(const std::shared_ptr<Trantorian> &emitter,
                              const std::vector<int> &victims,
                              const std::vector<std::size_t> &eggIds)
    {
        const std::size_t width = _world.getWidth();
        const std::size_t height = _world.getHeight();

        for (const int id : victims) {
            Client *victim = _clientByPlayerId(id);
            if (victim == nullptr || !victim->player()) {
                continue;
            }
            const int direction = victim->player()->calculAfectedTile(
                emitter->getX(), emitter->getY(), width, height);
            victim->pushResponse("eject: " + std::to_string(direction) + "\n");
            _broadcastGUI(protocol::Protocol::pex(id));
        }
        for (const std::size_t eggId : eggIds) {
            _broadcastGUI(protocol::Protocol::edi(static_cast<int>(eggId)));
        }
        _pollChanges = true;
    }

    void Server::_startIncantation(Client &client,
                                   const std::shared_ptr<Trantorian> &player)
    {
        if (!player) {
            client.popCommand();
            return;
        }
        if (!_world.canElevate(player)) {
            client.pushResponse("ko\n");
            client.popCommand();
            _pollChanges = true;
            return;
        }
        const int x = static_cast<int>(player->getX());
        const int y = static_cast<int>(player->getY());
        const std::size_t lvl = player->getLvl();
        std::vector<int> participants;

        for (const auto &mate : _world.startElevation(player)) {
            const Client *c = _clientByPlayer(mate);
            if (c != nullptr) {
                participants.push_back(c->id());
            }
        }
        client.pushResponse("Elevation underway\n");
        _broadcastGUI(
            protocol::Protocol::pic(x, y, static_cast<int>(lvl), participants));
        client.startCommand(static_cast<double>(protocol::incantationTimeCost) /
                            static_cast<double>(_cfg.freq));
        _pollChanges = true;
    }

    void Server::_finishIncantation(Client &client,
                                    const std::shared_ptr<Trantorian> &player)
    {
        if (!player) {
            return;
        }
        const int x = static_cast<int>(player->getX());
        const int y = static_cast<int>(player->getY());
        const ElevationResult result = _world.finishElevation(player);

        for (const auto &mate : result.participants) {
            Client *c = _clientByPlayer(mate);
            if (c == nullptr) {
                continue;
            }
            c->pushResponse(
                result.success
                    ? "Current level: " + std::to_string(result.level) + "\n"
                    : std::string("ko\n"));
            if (result.success) {
                _broadcastGUI(protocol::Protocol::plv(
                    c->id(), static_cast<int>(result.level)));
            }
        }
        _broadcastGUI(protocol::Protocol::pie(x, y, result.success ? 1 : 0));
        if (result.success) {
            _broadcastGUI(protocol::Protocol::bct(x, y, _tileQuantities(x, y)));
        }
        _pollChanges = true;
    }
} // namespace server
