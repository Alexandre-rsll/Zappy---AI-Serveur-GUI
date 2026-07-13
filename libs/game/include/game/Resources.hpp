/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** resources
*/

#ifndef ZAPPY_RESOURCES_HPP
#define ZAPPY_RESOURCES_HPP

#define MAX_LEVEL 8
#define NB_PLAYER_MAX_LVL 6

#include <iostream>
#include <array>

enum ResourceType {
    FOOD = 0,
    LINEMATE,
    DERAUMERE,
    SIBUR,
    MENDIANE,
    PHIRAS,
    THYSTAME,
    RESOURCE_COUNT,
};

inline std::array<float, RESOURCE_COUNT> DENSITIES = {0.5, 0.3,  0.15, 0.1,
                                                      0.1, 0.08, 0.05};

const std::array<std::string, RESOURCE_COUNT> RESOURCE_NAME = {
    "food", "linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame",
};

struct ElevationRequirement {
    int players;
    std::array<int, RESOURCE_COUNT> resources;
};

constexpr std::array<ElevationRequirement, 7> ELEVATION_TABLE = {
    {{1, {0, 1, 0, 0, 0, 0, 0}},
     {2, {0, 1, 1, 1, 0, 0, 0}},
     {2, {0, 2, 0, 1, 0, 2, 0}},
     {4, {0, 1, 1, 2, 0, 1, 0}},
     {4, {0, 1, 2, 1, 3, 0, 0}},
     {6, {0, 1, 2, 3, 0, 1, 0}},
     {6, {0, 2, 2, 2, 2, 2, 1}}}};

#endif // ZAPPY_RESOURCES_HPP
