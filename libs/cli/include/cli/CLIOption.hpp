/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CLIOption file
*/

#ifndef CLIOPTION_HPP
#define CLIOPTION_HPP

#include <string>
#include <optional>

namespace cli {

    struct CLIOption {
        explicit CLIOption() = default;
        ~CLIOption() = default;

        std::string key;
        std::string description = "No description";
        bool hasValue = false;
        bool hasMultipleValues = false;
        std::string defaultValue;
        bool isOptional = false;
    };

    class CLIOptionBuilder {
    public:
        explicit CLIOptionBuilder() = default;
        ~CLIOptionBuilder() = default;

        CLIOptionBuilder &setKey(const std::string &key)
        {
            _config.key = key;
            return *this;
        }

        CLIOptionBuilder &setHasValue(const bool hasValue)
        {
            _config.hasValue = hasValue;
            return *this;
        }

        CLIOptionBuilder &setHasMultipleValues(const bool hasMultipleValues)
        {
            _config.hasMultipleValues = hasMultipleValues;
            return *this;
        }

        CLIOptionBuilder &setIsOptional(const bool isOptional)
        {
            _config.isOptional = isOptional;
            return *this;
        }

        CLIOptionBuilder &setDefaultValue(const std::string &defaultValue)
        {
            _config.defaultValue = defaultValue;
            return *this;
        }

        CLIOptionBuilder &setDescription(const std::string &description)
        {
            _config.description = description;
            return *this;
        }

        [[nodiscard]]
        CLIOption build()
        {
            return _config;
        }

    private:
        CLIOption _config;
    };
} // namespace cli

#endif /* !CLIOPTION_HPP */
