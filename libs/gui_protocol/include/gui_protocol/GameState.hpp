/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** GameState
*/

#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

#include <array>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace gui {
    static constexpr std::size_t resourceCount = 7;
    static constexpr std::size_t maxLogEntries = 200;

    struct Tile {
        std::array<int, resourceCount> resources{};
    };

    struct Player {
        int id = 0;
        int x = 0;
        int y = 0;
        int orientation = 0;
        int level = 0;
        std::string teamName;
        std::array<int, resourceCount> inventory{};
        std::string lastBroadcast;
        std::size_t broadcastVersion = 0;
    };

    struct Egg {
        int id = 0;
        int playerId = 0;
        int x = 0;
        int y = 0;
    };

    struct Incantation {
        int x = 0;
        int y = 0;
        int level = 0;
        std::vector<int> playerIds;
    };

    struct GameState {
        int width = 0;
        int height = 0;
        int timeUnit = 0;
        std::size_t totalDeaths = 0;

        std::string winningTeam;
        std::string lastServerMessage;
        std::string lastAction;
        std::vector<std::string> logs;

        std::vector<std::string> teams;
        std::vector<Tile> tiles;
        std::vector<Incantation> incantations;

        std::unordered_map<int, Player> players;
        std::unordered_map<int, Egg> eggs;
    };
} // namespace gui

#endif // GAMESTATE_HPP
