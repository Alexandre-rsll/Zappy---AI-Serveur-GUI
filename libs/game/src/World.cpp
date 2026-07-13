/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** Map
*/

#include "game/World.hpp"
#include "game/GameException.hpp"

World::World(const std::size_t width, const std::size_t height,
             const std::vector<std::string> &teams, std::size_t initSlot)
    : _width(width), _height(height), _map(width * height)
{
    if (width <= 0 || height <= 0) {
        throw GameException("width and height must be positive");
    }
    for (const auto &team : teams) {
        _teams.insert({team, initSlot});
        spawnEggs(team);
    }
    _lastSpawn = std::chrono::steady_clock::now();
}

void World::generateResources()
{
    std::random_device pos;
    std::mt19937 rng(pos());
    std::uniform_int_distribution<std::mt19937::result_type> dist(
        0, (_width * _height) - 1);
    std::array<std::size_t, RESOURCE_COUNT> current{};

    for (const auto &tile : _map) {
        for (const auto &[type, count] : tile.resources) {
            current.at(static_cast<std::size_t>(type)) += count;
        }
    }
    for (std::size_t i = 0; i < RESOURCE_COUNT; ++i) {
        const auto target = static_cast<std::size_t>(
            static_cast<float>(_width) * static_cast<float>(_height) *
            DENSITIES.at(i));
        for (std::size_t j = current.at(i); j < target; ++j) {
            _map[dist(rng)].resources[static_cast<ResourceType>(i)]++;
        }
    }
}

Tile &World::getTile(const std::size_t x, const std::size_t y)
{
    if (x >= _width || y >= _height) {
        throw GameException("coordinates out of bounds");
    }
    return _map[(y * _width) + x];
}

void World::spawnEggs(const std::string &team)
{
    if (team.empty()) {
        throw GameException("team name cannot be empty");
    }
    if (!_teams.contains(team)) {
        throw GameException("team name not found");
    }
    std::random_device pos;
    std::mt19937 rng(pos());
    std::uniform_int_distribution<std::mt19937::result_type> distX(0,
                                                                   _width - 1);
    std::uniform_int_distribution<std::mt19937::result_type> distY(0,
                                                                   _height - 1);

    for (std::size_t i = 0; i < _teams.at(team); i++) {
        std::shared_ptr<Egg> egg = std::make_shared<Egg>(Egg{.id = _nextEggId++,
                                                             .team = team,
                                                             .x = distX(rng),
                                                             .y = distY(rng)});

        _map[(egg->y * _width) + egg->x].eggs.push_back(egg);
    }
}

void World::moveTrantorian(const std::shared_ptr<Trantorian> &trantorian,
                           const std::optional<DIRECTION> direction)
{
    std::size_t prevX = trantorian->getX();
    std::size_t prevY = trantorian->getY();
    trantorian->move(_width, _height, direction);
    auto &oldTile = _map[(prevY * _width) + prevX].trantorian;
    std::erase(oldTile, trantorian);
    _map[(trantorian->getY() * _width) + trantorian->getX()]
        .trantorian.push_back(trantorian);
}

std::shared_ptr<Egg> World::chooseEgg(const std::string &team) const
{
    if (team.empty()) {
        throw GameException("team name cannot be empty");
    }
    if (!_teams.contains(team)) {
        throw GameException("team name not found");
    }
    if (_teams.at(team) == 0) {
        throw GameException("team has no more available slot");
    }

    std::vector<std::shared_ptr<Egg>> teamEggs;

    for (const auto &tile : _map) {
        for (const auto &egg : tile.eggs) {
            if (egg->team == team) {
                teamEggs.push_back(egg);
            }
        }
    }
    if (teamEggs.empty()) {
        throw GameException("team has no eggs");
    }
    std::random_device pos;
    std::mt19937 rng(pos());
    std::uniform_int_distribution<std::mt19937::result_type> dist(
        0, teamEggs.size() - 1);

    int ranEgg = static_cast<int>(dist(rng));

    std::shared_ptr<Egg> eggToHatch = teamEggs[ranEgg];
    return eggToHatch;
}

std::shared_ptr<Trantorian> World::hatchEgg(const std::string &team)
{
    std::shared_ptr<Egg> eggToHatch = chooseEgg(team);
    _lastHatchedEggId = eggToHatch->id;

    std::size_t x = eggToHatch->x;
    std::size_t y = eggToHatch->y;
    Tile &tile = _map[(y * _width) + x];

    std::shared_ptr<Trantorian> trantorian =
        std::make_shared<Trantorian>(team, _width, _height, x, y);
    tile.trantorian.push_back(trantorian);

    std::erase(tile.eggs, eggToHatch);
    _teams.at(team)--;
    return trantorian;
}

std::size_t World::lastHatchedEggId() const
{
    return _lastHatchedEggId;
}

bool World::checkTime(int f)
{
    auto now = std::chrono::steady_clock::now();

    if (std::chrono::duration_cast<std::chrono::duration<double>>(now -
                                                                  _lastSpawn)
            .count() >= RESOURCE_SPAWN_INTERVAL / static_cast<double>(f)) {
        generateResources();
        _lastSpawn = std::chrono::steady_clock::now();
        return true;
    }
    return false;
}

int World::nextSpawnDelayMs(const int f) const
{
    const auto now = std::chrono::steady_clock::now();
    const auto interval =
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(
            std::chrono::duration<double>(RESOURCE_SPAWN_INTERVAL /
                                          static_cast<double>(f)));
    const auto due = _lastSpawn + interval;

    if (due <= now) {
        return 0;
    }
    return static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(due - now)
            .count());
}

std::size_t World::getWidth() const
{
    return _width;
}

std::size_t World::getHeight() const
{
    return _height;
}

const std::vector<Tile> &World::getTiles() const
{
    return _map;
}

std::size_t World::getTileSize(const std::size_t x, const std::size_t y) const
{
    return _map[(y * _width) + x].resources.size() +
           _map[(y * _width) + x].trantorian.size();
}

bool World::isEnoughTrantorian(const std::shared_ptr<Trantorian> &player) const
{
    int nbPlayers = 0;
    const Tile &tile = _map[(player->getY() * _width) + player->getX()];

    for (const auto &trantorian : tile.trantorian) {
        if (trantorian->getLvl() == player->getLvl()) {
            nbPlayers++;
        }
    }
    return nbPlayers >= ELEVATION_TABLE.at(player->getLvl() - 1).players;
}

bool World::isEnoughResources(const std::shared_ptr<Trantorian> &player) const
{
    const Tile &tile = _map[(player->getY() * _width) + player->getX()];

    for (std::size_t i = 0; i < RESOURCE_COUNT; i++) {
        const auto type = static_cast<ResourceType>(i);
        const std::size_t required =
            ELEVATION_TABLE.at(player->getLvl() - 1).resources.at(i);
        const std::size_t nbOnTile =
            tile.resources.contains(type) ? tile.resources.at(type) : 0;

        if (nbOnTile < required) {
            return false;
        }
    }
    return true;
}

bool World::canElevate(const std::shared_ptr<Trantorian> &player) const
{
    return player->getLvl() < MAX_LEVEL && isEnoughTrantorian(player) &&
           isEnoughResources(player);
}

std::vector<std::shared_ptr<Trantorian>>
World::startElevation(const std::shared_ptr<Trantorian> &player) const
{
    std::vector<std::shared_ptr<Trantorian>> participants;
    const Tile &tile = _map[(player->getY() * _width) + player->getX()];
    const std::size_t lvl = player->getLvl();

    for (const auto &trantorian : tile.trantorian) {
        if (trantorian->getLvl() == lvl) {
            trantorian->setIsFrozen(true);
            participants.push_back(trantorian);
        }
    }
    return participants;
}

ElevationResult
World::finishElevation(const std::shared_ptr<Trantorian> &player)
{
    const std::size_t lvl = player->getLvl();
    ElevationResult result{
        .success = canElevate(player), .level = lvl, .participants = {}};
    Tile &tile = _map[(player->getY() * _width) + player->getX()];

    for (const auto &trantorian : tile.trantorian) {
        if (trantorian->getLvl() == lvl) {
            result.participants.push_back(trantorian);
        }
    }
    if (result.success) {
        for (std::size_t i = 0; i < RESOURCE_COUNT; ++i) {
            tile.resources[static_cast<ResourceType>(i)] -=
                static_cast<std::size_t>(
                    ELEVATION_TABLE.at(lvl - 1).resources.at(i));
        }
        for (const auto &trantorian : result.participants) {
            trantorian->setLvl(lvl + 1);
        }
        result.level = lvl + 1;
    }
    for (const auto &trantorian : result.participants) {
        trantorian->setIsFrozen(false);
    }
    return result;
}

void World::checkLives(const int f)
{
    for (auto &tile : _map) {
        for (const auto &trantorian : tile.trantorian) {
            trantorian->checkLife(f);
        }
    }
    for (auto &tile : _map) {
        std::erase_if(tile.trantorian,
                      [](const std::shared_ptr<Trantorian> &t) {
                          return t->getLife() == 0;
                      });
    }
}

void World::fork(const std::shared_ptr<Trantorian> &trantorian)
{
    Tile &tile = _map[(trantorian->getY() * _width) + trantorian->getX()];

    _teams.at(trantorian->getTeam())++;
    const std::shared_ptr<Egg> egg =
        std::make_shared<Egg>(Egg{.id = _nextEggId++,
                                  .team = trantorian->getTeam(),
                                  .x = trantorian->getX(),
                                  .y = trantorian->getY()});
    tile.eggs.push_back(egg);
}

std::size_t World::connectNbr(const std::string &team) const
{
    if (team.empty()) {
        throw GameException("team name cannot be empty");
    }
    if (!_teams.contains(team)) {
        throw GameException("team name not found");
    }
    return _teams.at(team);
}

bool World::victoryCondition(const std::string &team) const
{
    int nbTrantorianMaxLvl = 0;

    for (const auto &tile : _map) {
        for (const auto &trantorian : tile.trantorian) {
            if (trantorian->getTeam() == team &&
                trantorian->getLvl() == MAX_LEVEL) {
                nbTrantorianMaxLvl++;
            }
        }
    }
    return nbTrantorianMaxLvl >= NB_PLAYER_MAX_LVL;
}
