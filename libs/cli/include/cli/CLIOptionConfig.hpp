/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CLIOptionConfig
*/

#ifndef CLIOPTIONCONFIG_HPP
#define CLIOPTIONCONFIG_HPP

#include <vector>
#include <optional>
#include <string>

namespace cli {
    struct CLIOptionConfig {
        explicit CLIOptionConfig() = default;
        explicit CLIOptionConfig(const std::string &key, const bool isDefined,
                                 const std::optional<std::string> &value)
            : key(key), isDefined(isDefined), value(value)
        {
            if (value.has_value()) {
                values.push_back(value.value());
            }
        };

        explicit CLIOptionConfig(const std::string &key, const bool isDefined,
                                 const std::vector<std::string> &values)
            : key(key), isDefined(isDefined), values(values)
        {
            if (!values.empty()) {
                value = values.front();
            }
        };

        ~CLIOptionConfig() = default;

        std::string key;
        bool isDefined = false;
        std::optional<std::string> value;
        std::vector<std::string> values;
    };

} // namespace cli

#endif // CLIOPTIONCONFIG_HPP
