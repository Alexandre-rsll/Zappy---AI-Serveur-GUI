/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CLIManager file
*/

#include "cli/CLIManager.hpp"

namespace cli {
    const CLIOptionConfig &CLIManager::getConfig(const std::string &key) const
    {
        return _results.at(key);
    }

    bool CLIManager::hasConfig(const std::string &key) const
    {
        return _results.contains(key);
    }

    void CLIManager::parseMultipleValues(
        const std::vector<std::string> &params, std::size_t &index,
        const CLIOption &option,
        const std::unordered_map<std::string, const CLIOption *> &optionByKey)
    {
        std::vector<std::string> values;
        std::size_t nextIndex = index + 1;

        while (nextIndex < params.size() &&
               !optionByKey.contains(params[nextIndex])) {
            values.push_back(params[nextIndex]);
            ++nextIndex;
        }

        if (values.empty()) {
            throw CLIManagerException("Missing value for option: -" +
                                      option.key);
        }

        _results[option.key] = CLIOptionConfig(option.key, true, values);
        index = nextIndex - 1;
    }

    void CLIManager::parseSingleValue(
        const std::vector<std::string> &params, std::size_t &index,
        const CLIOption &option,
        const std::unordered_map<std::string, const CLIOption *> &optionByKey)
    {
        std::optional<std::string> value;

        if (option.hasValue) {
            const std::size_t valueIndex = index + 1;

            if (valueIndex >= params.size()) {
                throw CLIManagerException("Missing value for option: -" +
                                          option.key);
            }

            const std::string &next = params[valueIndex];

            if (optionByKey.contains(next)) {
                throw CLIManagerException("Missing value for option: -" +
                                          option.key);
            }

            value = next;
            index = valueIndex;
        }

        _results[option.key] = CLIOptionConfig(option.key, true, value);
    }

    bool CLIManager::parse(const std::vector<std::string> &params)
    {
        std::unordered_map<std::string, const CLIOption *> optionByKey;

        _results.clear();

        for (const auto &option : _options) {
            optionByKey["-" + option.key] = &option;
        }

        for (std::size_t i = 0; i < params.size(); ++i) {
            const std::string &param = params.at(i);

            if (param == "--help") {
                printHelp(std::cout);
                return false;
            }
            if (!optionByKey.contains(param)) {
                throw CLIManagerException("Unknown CLI option: " + param);
            }

            const CLIOption *option = optionByKey.at(param);
            std::optional<std::string> value = std::nullopt;
            if (option->hasMultipleValues) {
                parseMultipleValues(params, i, *option, optionByKey);
                continue;
            }
            parseSingleValue(params, i, *option, optionByKey);
        }

        for (const auto &option : _options) {
            if (_results.contains(option.key)) {
                continue;
            }

            if (!option.isOptional) {
                throw CLIManagerException("Missing required option: -" +
                                          option.key);
            }

            _results[option.key] =
                CLIOptionConfig(option.key, false, option.defaultValue);
        }
        return true;
    }

    void CLIManager::printHelp(std::ostream &stream) const noexcept
    {
        stream << "USAGE: ./zappy_server" << std::endl;
        for (const auto &option : _options) {
            stream << " -" << option.key << " " << option.description;
        }
        stream << std::endl;
    }

    void CLIManager::registerConfigOption(const CLIOption &configOption)
    {
        _options.push_back(configOption);
        _logger.debug() << "config option '" << configOption.key
                        << "' registered." << std::endl;
    }
} // namespace cli
