/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** Map
*/

#ifndef ZAPPY_MAP_HPP
#define ZAPPY_MAP_HPP

#define RESOURCE_SPAWN_INTERVAL 20.0

#include <chrono>
#include <map>
#include <memory>
#include <vector>
#include <random>
#include <algorithm>
#include <sstream>

#include "Resources.hpp"
#include "Trantorian.hpp"

struct Egg {
    std::size_t id;
    std::string team;
    std::size_t x;
    std::size_t y;
};

struct Tile {
    std::map<ResourceType, std::size_t> resources;
    std::vector<std::shared_ptr<Trantorian>> trantorian;
    std::vector<std::shared_ptr<Egg>> eggs;
};

struct ElevationResult {
    bool success;
    std::size_t level;
    std::vector<std::shared_ptr<Trantorian>> participants;
};

class World {
public:
    World(std::size_t width, std::size_t height,
          const std::vector<std::string> &teams, std::size_t initSlot);
    ~World() = default;

    void generateResources();
    Tile &getTile(std::size_t x, std::size_t y);

    void spawnEggs(const std::string &team);
    void moveTrantorian(const std::shared_ptr<Trantorian> &trantorian,
                        std::optional<DIRECTION> direction);
    [[nodiscard]] std::shared_ptr<Egg> chooseEgg(const std::string &team) const;
    std::shared_ptr<Trantorian> hatchEgg(const std::string &team);

    bool checkTime(int f);
    [[nodiscard]] int nextSpawnDelayMs(int f) const;

    [[nodiscard]] std::size_t getWidth() const;
    [[nodiscard]] std::size_t getHeight() const;

    [[nodiscard]] const std::vector<Tile> &getTiles() const;
    [[nodiscard]] std::size_t getTileSize(std::size_t x, std::size_t y) const;

    [[nodiscard]] bool
    canElevate(const std::shared_ptr<Trantorian> &player) const;
    [[nodiscard]] std::vector<std::shared_ptr<Trantorian>>
    startElevation(const std::shared_ptr<Trantorian> &player) const;
    ElevationResult finishElevation(const std::shared_ptr<Trantorian> &player);

    void checkLives(int f);

    void fork(const std::shared_ptr<Trantorian> &trantorian);
    [[nodiscard]] std::size_t connectNbr(const std::string &team) const;
    [[nodiscard]] std::size_t lastHatchedEggId() const;

    [[nodiscard]] bool victoryCondition(const std::string &team) const;

private:
    [[nodiscard]] bool
    isEnoughTrantorian(const std::shared_ptr<Trantorian> &player) const;
    [[nodiscard]] bool
    isEnoughResources(const std::shared_ptr<Trantorian> &player) const;

    std::size_t _width;
    std::size_t _height;
    std::vector<Tile> _map;
    std::chrono::steady_clock::time_point _lastSpawn;
    std::map<std::string, std::size_t> _teams;
    std::size_t _nextEggId = 0;
    std::size_t _lastHatchedEggId = 0;
};

#endif // ZAPPY_MAP_HPP
