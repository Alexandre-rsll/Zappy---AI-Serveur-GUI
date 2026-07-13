/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** Trantorian
*/

#ifndef ZAPPY_TRANTORIAN_HPP
#define ZAPPY_TRANTORIAN_HPP

#define LIFE_TICK 126.0

#define INIT_LIFES_STACK 10
#define SAME_TILE 0
#define FRONT_TILE 1
#define FRONT_RIGHT_TILE 2
#define RIGHT_TILE 3
#define BACK_RIGHT_TILE 4
#define BACK_TILE 5
#define BACK_LEFT_TILE 6
#define LEFT_TILE 7
#define FRONT_LEFT_TILE 8

#include <chrono>
#include <cstddef>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <sstream>

#include "Resources.hpp"

enum DIRECTION {
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

struct Vec2d {
    int x;
    int y;
};

class World;

class Trantorian {
public:
    Trantorian(std::string team, std::size_t width, std::size_t height,
               std::size_t x, std::size_t y);
    ~Trantorian() = default;

    [[nodiscard]] std::size_t getLife() const;
    std::string &getTeam();
    [[nodiscard]] const std::map<ResourceType, std::size_t> &
    getInventory() const;
    [[nodiscard]] DIRECTION getDirection() const;
    [[nodiscard]] bool isFrozen() const;

    void checkLife(int f);
    bool eatFood();

    void move(std::size_t width, std::size_t height,
              std::optional<DIRECTION> direction);
    void turnLeft();
    void turnRight();

    [[nodiscard]] std::string look(const World &world) const;

    [[nodiscard]] std::size_t getX() const;
    [[nodiscard]] std::size_t getY() const;
    [[nodiscard]] std::size_t getLvl() const;

    static std::string contentToString(const World &world, std::size_t x,
                                       std::size_t y);

    [[nodiscard]] Vec2d getForward() const;
    [[nodiscard]] Vec2d getSide() const;

    [[nodiscard]] int calculAfectedTile(std::size_t emitterX,
                                        std::size_t emitterY,
                                        std::size_t mapWidth,
                                        std::size_t mapHeight) const;
    static int getAffectedTile(int projForward, int projSide);

    void setIsFrozen(bool isFrozen);
    void setLvl(std::size_t lvl);

    bool take(World &world, ResourceType object);
    bool set(World &world, ResourceType object);

    bool eject(World &world);

private:
    std::string _team;
    std::size_t _x;
    std::size_t _y;
    std::size_t _lvl;
    std::size_t _life;
    std::chrono::steady_clock::time_point _lastLifeUpdate;
    DIRECTION _direction;
    bool _isFrozen;
    std::map<ResourceType, std::size_t> _resources;
};

#endif // ZAPPY_TRANTORIAN_HPP
