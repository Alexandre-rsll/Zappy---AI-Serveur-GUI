/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** GUIProtocolHandler
*/

#include <algorithm>
#include <stdexcept>

#include "gui_protocol/GUIProtocolHandler.hpp"

namespace gui {
    GUIProtocolHandler::GUIProtocolHandler(GameState &state)
        : _state(state),
          _handlers{
              {"msz",
               [this](std::istringstream &stream) { _handleMsz(stream); }},
              {"bct",
               [this](std::istringstream &stream) { _handleBct(stream); }},
              {"tna",
               [this](std::istringstream &stream) { _handleTna(stream); }},
              {"pnw",
               [this](std::istringstream &stream) { _handlePnw(stream); }},
              {"ppo",
               [this](std::istringstream &stream) { _handlePpo(stream); }},
              {"plv",
               [this](std::istringstream &stream) { _handlePlv(stream); }},
              {"pin",
               [this](std::istringstream &stream) { _handlePin(stream); }},
              {"pex",
               [this](std::istringstream &stream) { _handlePex(stream); }},
              {"pbc",
               [this](std::istringstream &stream) { _handlePbc(stream); }},
              {"pic",
               [this](std::istringstream &stream) { _handlePic(stream); }},
              {"pie",
               [this](std::istringstream &stream) { _handlePie(stream); }},
              {"pfk",
               [this](std::istringstream &stream) { _handlePfk(stream); }},
              {"pdr",
               [this](std::istringstream &stream) { _handlePdr(stream); }},
              {"pgt",
               [this](std::istringstream &stream) { _handlePgt(stream); }},
              {"pdi",
               [this](std::istringstream &stream) { _handlePdi(stream); }},
              {"enw",
               [this](std::istringstream &stream) { _handleEnw(stream); }},
              {"ebo",
               [this](std::istringstream &stream) { _handleEbo(stream); }},
              {"edi",
               [this](std::istringstream &stream) { _handleEdi(stream); }},
              {"sgt",
               [this](std::istringstream &stream) { _handleSgt(stream); }},
              {"sst",
               [this](std::istringstream &stream) { _handleSst(stream); }},
              {"seg",
               [this](std::istringstream &stream) { _handleSeg(stream); }},
              {"smg",
               [this](std::istringstream &stream) { _handleSmg(stream); }},
              {"suc",
               [this](std::istringstream &stream) { _handleSuc(stream); }},
              {"sbp",
               [this](std::istringstream &stream) { _handleSbp(stream); }},
          }
    {}

    int GUIProtocolHandler::_parseId(const std::string &raw)
    {
        if (raw.empty()) {
            throw std::runtime_error("Invalid protocol id");
        }

        if (raw.front() == gui::idPrefix) {
            return std::stoi(raw.substr(1));
        }
        return std::stoi(raw);
    }

    std::string
    GUIProtocolHandler::_readRemainingMessage(std::istringstream &stream)
    {
        std::string message;

        std::getline(stream >> std::ws, message);
        return message;
    }

    void GUIProtocolHandler::handleLine(const std::string &line)
    {
        std::istringstream stream(line);
        std::string command;

        stream >> command;

        _appendLog(command, line);

        const auto handler = _handlers.find(command);

        if (handler == _handlers.end()) {
            _state.lastServerMessage =
                "Unknown GUI protocol command: " + command;
            return;
        }

        handler->second(stream);
    }

    void GUIProtocolHandler::_appendLog(const std::string &command,
                                        const std::string &line)
    {
        if (command == "bct" || command == "ppo" || command == "plv" ||
            command == "pin") {
            return;
        }

        _state.logs.push_back(line);
        if (_state.logs.size() > maxLogEntries) {
            _state.logs.erase(_state.logs.begin());
        }
    }

    void GUIProtocolHandler::_handleMsz(std::istringstream &stream)
    {
        stream >> _state.width >> _state.height;

        if (_state.width <= 0 || _state.height <= 0) {
            throw std::runtime_error("Invalid map size");
        }

        const std::size_t tileCount = static_cast<std::size_t>(_state.width) *
                                      static_cast<std::size_t>(_state.height);

        _state.tiles.clear();
        _state.tiles.resize(tileCount);
    }

    void GUIProtocolHandler::_handleBct(std::istringstream &stream)
    {
        int x = 0;
        int y = 0;

        stream >> x >> y;

        if (x < 0 || y < 0 || x >= _state.width || y >= _state.height) {
            return;
        }

        Tile tile{};

        for (std::size_t i = 0; i < resourceCount; ++i) {
            stream >> tile.resources.at(i);
        }

        const std::size_t index = static_cast<std::size_t>(y) *
                                      static_cast<std::size_t>(_state.width) +
                                  static_cast<std::size_t>(x);

        if (index < _state.tiles.size()) {
            _state.tiles.at(index) = tile;
        }
    }

    void GUIProtocolHandler::_handleTna(std::istringstream &stream)
    {
        std::string teamName;

        stream >> teamName;

        if (teamName.empty()) {
            return;
        }

        if (std::find(_state.teams.begin(), _state.teams.end(), teamName) ==
            _state.teams.end()) {
            _state.teams.push_back(teamName);
        }
    }

    void GUIProtocolHandler::_handlePnw(std::istringstream &stream)
    {
        std::string rawId;
        Player player{};

        stream >> rawId >> player.x >> player.y >> player.orientation >>
            player.level >> player.teamName;

        player.id = _parseId(rawId);
        _state.players[player.id] = player;
    }

    void GUIProtocolHandler::_handlePpo(std::istringstream &stream)
    {
        std::string rawId;
        int x = 0;
        int y = 0;
        int orientation = 0;

        stream >> rawId >> x >> y >> orientation;

        const int id = _parseId(rawId);

        if (!_state.players.contains(id)) {
            return;
        }

        _state.players.at(id).x = x;
        _state.players.at(id).y = y;
        _state.players.at(id).orientation = orientation;
    }

    void GUIProtocolHandler::_handlePlv(std::istringstream &stream)
    {
        std::string rawId;
        int level = 0;

        stream >> rawId >> level;

        const int id = _parseId(rawId);

        if (_state.players.contains(id)) {
            _state.players.at(id).level = level;
        }
    }

    void GUIProtocolHandler::_handlePin(std::istringstream &stream)
    {
        std::string rawId;
        int x = 0;
        int y = 0;

        stream >> rawId >> x >> y;

        const int id = _parseId(rawId);

        if (!_state.players.contains(id)) {
            return;
        }

        Player &player = _state.players.at(id);

        player.x = x;
        player.y = y;

        for (std::size_t i = 0; i < resourceCount; ++i) {
            stream >> player.inventory.at(i);
        }
    }

    void GUIProtocolHandler::_handlePex(std::istringstream &stream)
    {
        std::string rawId;

        stream >> rawId;
        _state.lastAction = "Player " + std::to_string(_parseId(rawId)) +
                            " expelled other players";
    }

    void GUIProtocolHandler::_handlePbc(std::istringstream &stream)
    {
        std::string rawId;

        stream >> rawId;

        const int id = _parseId(rawId);
        const std::string message = _readRemainingMessage(stream);

        if (_state.players.contains(id)) {
            Player &player = _state.players.at(id);

            player.lastBroadcast = message;
            ++player.broadcastVersion;
        }

        _state.lastServerMessage = message;
    }

    void GUIProtocolHandler::_handlePic(std::istringstream &stream)
    {
        Incantation incantation{};
        std::string rawId;

        stream >> incantation.x >> incantation.y >> incantation.level;

        while (stream >> rawId) {
            incantation.playerIds.push_back(_parseId(rawId));
        }

        _state.incantations.push_back(incantation);
    }

    void GUIProtocolHandler::_handlePie(std::istringstream &stream)
    {
        int x = 0;
        int y = 0;
        int result = 0;

        stream >> x >> y >> result;

        _state.incantations.erase(
            std::remove_if(_state.incantations.begin(),
                           _state.incantations.end(),
                           [x, y](const Incantation &incantation) {
                               return incantation.x == x && incantation.y == y;
                           }),
            _state.incantations.end());

        _state.lastAction = "Incantation result: " + std::to_string(result);
    }

    void GUIProtocolHandler::_handlePfk(std::istringstream &stream)
    {
        std::string rawId;

        stream >> rawId;
        _state.lastAction = "Player " + std::to_string(_parseId(rawId)) +
                            " started laying an egg";
    }

    void GUIProtocolHandler::_handlePdr(std::istringstream &stream)
    {
        std::string rawId;
        int resource = 0;

        stream >> rawId >> resource;
        _state.lastAction = "Player " + std::to_string(_parseId(rawId)) +
                            " dropped resource " + std::to_string(resource);
    }

    void GUIProtocolHandler::_handlePgt(std::istringstream &stream)
    {
        std::string rawId;
        int resource = 0;

        stream >> rawId >> resource;
        _state.lastAction = "Player " + std::to_string(_parseId(rawId)) +
                            " collected resource " + std::to_string(resource);
    }

    void GUIProtocolHandler::_handlePdi(std::istringstream &stream)
    {
        std::string rawId;

        stream >> rawId;
        _state.players.erase(_parseId(rawId));
        ++_state.totalDeaths;
    }

    void GUIProtocolHandler::_handleEnw(std::istringstream &stream)
    {
        std::string rawEggId;
        std::string rawPlayerId;
        Egg egg{};

        stream >> rawEggId >> rawPlayerId >> egg.x >> egg.y;

        egg.id = _parseId(rawEggId);
        egg.playerId = _parseId(rawPlayerId);

        _state.eggs[egg.id] = egg;
    }

    void GUIProtocolHandler::_handleEbo(std::istringstream &stream)
    {
        std::string rawEggId;

        stream >> rawEggId;
        _state.eggs.erase(_parseId(rawEggId));
    }

    void GUIProtocolHandler::_handleEdi(std::istringstream &stream)
    {
        std::string rawEggId;

        stream >> rawEggId;
        _state.eggs.erase(_parseId(rawEggId));
    }

    void GUIProtocolHandler::_handleSgt(std::istringstream &stream)
    {
        stream >> _state.timeUnit;
    }

    void GUIProtocolHandler::_handleSst(std::istringstream &stream)
    {
        stream >> _state.timeUnit;
    }

    void GUIProtocolHandler::_handleSeg(std::istringstream &stream)
    {
        stream >> _state.winningTeam;
    }

    void GUIProtocolHandler::_handleSmg(std::istringstream &stream)
    {
        _state.lastServerMessage = _readRemainingMessage(stream);
    }

    void GUIProtocolHandler::_handleSuc(UNUSED std::istringstream &stream)
    {
        _state.lastServerMessage = "Unknown command";
    }

    void GUIProtocolHandler::_handleSbp(UNUSED std::istringstream &stream)
    {
        _state.lastServerMessage = "Bad command parameter";
    }
} // namespace gui
