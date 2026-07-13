/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** GUIProtocolHandler
*/

#ifndef GUIPROTOCOLHANDLER_HPP
#define GUIPROTOCOLHANDLER_HPP

#include <functional>
#include <sstream>
#include <string>
#include <unordered_map>

#include "gui_protocol/GameState.hpp"

#define UNUSED __attribute__((unused))
namespace gui {

    constexpr char idPrefix = '#';

    class GUIProtocolHandler {
    public:
        explicit GUIProtocolHandler(GameState &state);
        ~GUIProtocolHandler() = default;

        void handleLine(const std::string &line);

    private:
        GameState &_state;

        std::unordered_map<std::string,
                           std::function<void(std::istringstream &)>>
            _handlers;

        void _handleMsz(std::istringstream &stream);
        void _handleBct(std::istringstream &stream);
        void _handleTna(std::istringstream &stream);
        void _handlePnw(std::istringstream &stream);
        void _handlePpo(std::istringstream &stream);
        void _handlePlv(std::istringstream &stream);
        void _handlePin(std::istringstream &stream);
        void _handlePex(std::istringstream &stream);
        void _handlePbc(std::istringstream &stream);
        void _handlePic(std::istringstream &stream);
        void _handlePie(std::istringstream &stream);
        void _handlePfk(std::istringstream &stream);
        void _handlePdr(std::istringstream &stream);
        void _handlePgt(std::istringstream &stream);
        void _handlePdi(std::istringstream &stream);
        void _handleEnw(std::istringstream &stream);
        void _handleEbo(std::istringstream &stream);
        void _handleEdi(std::istringstream &stream);
        void _handleSgt(std::istringstream &stream);
        void _handleSst(std::istringstream &stream);
        void _handleSeg(std::istringstream &stream);
        void _handleSmg(std::istringstream &stream);
        void _handleSuc(std::istringstream &stream);
        void _handleSbp(std::istringstream &stream);

        static int _parseId(const std::string &raw);
        static std::string _readRemainingMessage(std::istringstream &stream);
        void _appendLog(const std::string &command, const std::string &line);
    };
} // namespace gui

#endif // GUIPROTOCOLHANDLER_HPP
