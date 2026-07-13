/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** Trantorian
*/

#include <utility>

#include "game/Trantorian.hpp"
#include "game/World.hpp"
#include "game/GameException.hpp"

Trantorian::Trantorian(std::string team, std::size_t width, std::size_t height,
                       std::size_t x, std::size_t y)
    : _team(std::move(team)), _lvl(1), _life(INIT_LIFES_STACK), _isFrozen(false)
{
    if (x >= width || y >= height || x < 0 || y < 0) {
        throw GameException("invalid parameter x and y");
    }
    _x = x;
    _y = y;
    _lastLifeUpdate = std::chrono::steady_clock::now();
    std::random_device pos;
    std::mt19937 rng(pos());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, 3);
    _direction = static_cast<DIRECTION>(dist(rng));
}

std::size_t Trantorian::getLife() const
{
    return _life;
}

std::string &Trantorian::getTeam()
{
    return _team;
}

const std::map<ResourceType, std::size_t> &Trantorian::getInventory() const
{
    return _resources;
}

DIRECTION Trantorian::getDirection() const
{
    return _direction;
}

bool Trantorian::isFrozen() const
{
    return _isFrozen;
}

void Trantorian::checkLife(int f)
{
    auto now = std::chrono::steady_clock::now();

    if (std::chrono::duration_cast<std::chrono::duration<double>>(
            now - _lastLifeUpdate)
            .count() >= LIFE_TICK / f) {
        if (_life > 0) {
            --_life;
            _lastLifeUpdate = std::chrono::steady_clock::now();
        }
    }
}

bool Trantorian::eatFood()
{
    if (_resources[ResourceType::FOOD] > 0) {
        _resources[ResourceType::FOOD]--;
        ++_life;
        return true;
    }
    return false;
}

void Trantorian::move(std::size_t width, std::size_t height,
                      std::optional<DIRECTION> direction)
{
    if (width <= 0 || height <= 0) {
        throw GameException("width and height must be positive");
    }
    if (direction.has_value()) {
        _direction = direction.value();
    }
    if (_direction == UP) {
        _y = (_y + height - 1) % height;
        return;
    }
    if (_direction == DOWN) {
        _y = (_y + 1) % height;
        return;
    }
    if (_direction == LEFT) {
        _x = (_x + width - 1) % width;
        return;
    }
    if (_direction == RIGHT) {
        _x = (_x + 1) % width;
    }
}

void Trantorian::turnLeft()
{
    if (_direction == UP) {
        _direction = LEFT;
        return;
    }
    if (_direction == DOWN) {
        _direction = RIGHT;
        return;
    }
    if (_direction == LEFT) {
        _direction = DOWN;
        return;
    }
    if (_direction == RIGHT) {
        _direction = UP;
    }
}

void Trantorian::turnRight()
{
    if (_direction == UP) {
        _direction = RIGHT;
        return;
    }
    if (_direction == DOWN) {
        _direction = LEFT;
        return;
    }
    if (_direction == LEFT) {
        _direction = UP;
        return;
    }
    if (_direction == RIGHT) {
        _direction = DOWN;
    }
}

std::size_t Trantorian::getX() const
{
    return _x;
}

std::size_t Trantorian::getY() const
{
    return _y;
}

std::size_t Trantorian::getLvl() const
{
    return _lvl;
}

std::string Trantorian::look(const World &world) const
{
    Vec2d forward = getForward();
    Vec2d side = getSide();
    std::string result = "[";
    std::vector<std::string> tiles;

    for (std::size_t i = 0; i <= _lvl; ++i) {
        for (int j = static_cast<int>(-i); std::cmp_less_equal(j, i); ++j) {
            std::size_t tileX =
                (static_cast<int>(_x) + forward.x * static_cast<int>(i) +
                 side.x * j + static_cast<int>(world.getWidth())) %
                static_cast<int>(world.getWidth());
            std::size_t tileY =
                (static_cast<int>(_y) + forward.y * static_cast<int>(i) +
                 side.y * j + static_cast<int>(world.getHeight())) %
                static_cast<int>(world.getHeight());
            tiles.push_back(contentToString(world, tileX, tileY));
        }
    }
    for (std::size_t i = 0; i < tiles.size(); ++i) {
        result += tiles[i];
        if (i < tiles.size() - 1) {
            result += ", ";
        }
    }
    result += "]";
    return result;
}

std::string Trantorian::contentToString(const World &world, std::size_t tileX,
                                        std::size_t tileY)
{
    const Tile &tile = world.getTiles()[(tileY * world.getWidth()) + tileX];
    std::vector<std::string> tokens;

    tokens.reserve(tile.trantorian.size() + tile.resources.size());
    for (std::size_t i = 0; i < tile.trantorian.size(); ++i) {
        tokens.emplace_back("player");
    }
    for (const auto &[type, count] : tile.resources) {
        for (std::size_t i = 0; i < count; ++i) {
            tokens.push_back(RESOURCE_NAME.at(type));
        }
    }
    std::string content;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        content += tokens[i];
        if (i + 1 < tokens.size()) {
            content += " ";
        }
    }
    return content;
}

Vec2d Trantorian::getForward() const
{
    if (_direction == UP) {
        return {.x = 0, .y = -1};
    }
    if (_direction == DOWN) {
        return {.x = 0, .y = 1};
    }
    if (_direction == LEFT) {
        return {.x = -1, .y = 0};
    }
    if (_direction == RIGHT) {
        return {.x = 1, .y = 0};
    }
    return {};
}

Vec2d Trantorian::getSide() const
{
    if (_direction == UP) {
        return {.x = 1, .y = 0};
    }
    if (_direction == DOWN) {
        return {.x = -1, .y = 0};
    }
    if (_direction == LEFT) {
        return {.x = 0, .y = -1};
    }
    if (_direction == RIGHT) {
        return {.x = 0, .y = 1};
    }
    return {};
}

int Trantorian::calculAfectedTile(std::size_t emitterX, std::size_t emitterY,
                                  std::size_t mapWidth,
                                  std::size_t mapHeight) const
{
    int dx = static_cast<int>(emitterX) - static_cast<int>(_x);
    int dy = static_cast<int>(emitterY) - static_cast<int>(_y);
    const int width = static_cast<int>(mapWidth);
    const int height = static_cast<int>(mapHeight);
    Vec2d forward = getForward();
    Vec2d side = getSide();

    if (dx > width / 2) {
        dx -= width;
    }
    if (dx < -width / 2) {
        dx += width;
    }
    if (dy > height / 2) {
        dy -= height;
    }
    if (dy < -height / 2) {
        dy += height;
    }

    const int projForward = (dx * forward.x) + (dy * forward.y);
    const int projSide = (dx * side.x) + (dy * side.y);

    return getAffectedTile(projForward, projSide);
}

int Trantorian::getAffectedTile(int projForward, int projSide)
{
    int affectedTile = 0;

    if (projForward == 0 && projSide == 0) {
        affectedTile = SAME_TILE;
    }
    if (projForward > 0 && projSide == 0) {
        affectedTile = FRONT_TILE;
    }
    if (projForward > 0 && projSide > 0) {
        affectedTile = FRONT_RIGHT_TILE;
    }
    if (projForward == 0 && projSide > 0) {
        affectedTile = RIGHT_TILE;
    }
    if (projForward < 0 && projSide > 0) {
        affectedTile = BACK_RIGHT_TILE;
    }
    if (projForward < 0 && projSide == 0) {
        affectedTile = BACK_TILE;
    }
    if (projForward < 0 && projSide < 0) {
        affectedTile = BACK_LEFT_TILE;
    }
    if (projForward == 0 && projSide < 0) {
        affectedTile = LEFT_TILE;
    }
    if (projForward > 0 && projSide < 0) {
        affectedTile = FRONT_LEFT_TILE;
    }
    return affectedTile;
}

void Trantorian::setIsFrozen(bool isFrozen)
{
    _isFrozen = isFrozen;
}

void Trantorian::setLvl(std::size_t lvl)
{
    if (lvl < _lvl || _lvl > MAX_LEVEL) {
        throw GameException(
            "lvl must be between 1 and 8 and superior to the actual lvl");
    }
    _lvl = lvl;
}

bool Trantorian::take(World &world, ResourceType object)
{
    Tile &tile = world.getTile(_x, _y);

    if (tile.resources[object] > 0) {
        _resources[object]++;
        tile.resources[object]--;
        return true;
    }
    return false;
}

bool Trantorian::set(World &world, ResourceType object)
{
    Tile &tile = world.getTile(_x, _y);

    if (_resources[object] > 0) {
        _resources[object]--;
        tile.resources[object]++;
        return true;
    }
    return false;
}

bool Trantorian::eject(World &world)
{
    Tile &tile = world.getTile(_x, _y);
    std::vector<std::shared_ptr<Trantorian>> trantoriansToEject;

    if (tile.trantorian.empty() || tile.trantorian.size() < 2) {
        return false;
    }
    for (const auto &trantorian : tile.trantorian) {
        if (trantorian.get() != this) {
            trantoriansToEject.push_back(trantorian);
        }
    }
    tile.eggs.clear();
    for (const auto &trantorian : trantoriansToEject) {
        world.moveTrantorian(trantorian, _direction);
    }
    return true;
}
