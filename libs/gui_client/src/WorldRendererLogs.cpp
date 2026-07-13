/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** World renderer logs
*/

#include "WorldRendererInternals.hpp"

namespace gui {
    namespace {
        constexpr int logButtonMargin = 18;
        constexpr int logButtonWidth = 74;
        constexpr int logButtonHeight = 32;
        constexpr int logPanelWidth = 350;
        constexpr int logPanelTop = 60;
        constexpr int logPanelBottomMargin = 18;
        constexpr int logPanelPadding = 14;
        constexpr int logTitleSize = 20;
        constexpr int logTextSize = 15;
        constexpr int logLineHeight = 19;
        constexpr int logScrollStep = 3;
        constexpr Color logPanelColor = {.r = 10, .g = 14, .b = 18, .a = 224};
        constexpr Color logButtonColor = {.r = 38, .g = 55, .b = 63, .a = 235};
        constexpr Color logButtonActiveColor = {
            .r = 92, .g = 119, .b = 87, .a = 245};
        constexpr Color logTitleColor = {
            .r = 255, .g = 236, .b = 158, .a = 255};

        [[nodiscard]] raylib::Rectangle
        getLogButtonBounds(const int screenWidth)
        {
            return {static_cast<float>(screenWidth - logButtonWidth -
                                       logButtonMargin),
                    static_cast<float>(logButtonMargin),
                    static_cast<float>(logButtonWidth),
                    static_cast<float>(logButtonHeight)};
        }

        [[nodiscard]] raylib::Rectangle
        getLogPanelBounds(const int screenWidth, const int screenHeight)
        {
            return {static_cast<float>(screenWidth - logPanelWidth -
                                       logButtonMargin),
                    static_cast<float>(logPanelTop),
                    static_cast<float>(logPanelWidth),
                    static_cast<float>(screenHeight - logPanelTop -
                                       logPanelBottomMargin)};
        }

        [[nodiscard]] int getVisibleLogCount(const raylib::Rectangle &panel)
        {
            const int contentHeight = static_cast<int>(panel.height) -
                                      (logPanelPadding * 2) - logTitleSize;

            return std::max(1, contentHeight / logLineHeight);
        }
    } // namespace

    void WorldRenderer::_updateLogs(const GameState &state)
    {
        const Vector2 mousePosition = raylib::Mouse::GetPosition();
        const int screenWidth = _window.GetWidth();
        const int screenHeight = _window.GetHeight();
        const raylib::Rectangle button = getLogButtonBounds(screenWidth);

        if (raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_LEFT) &&
            button.CheckCollision(mousePosition)) {
            _logsVisible = !_logsVisible;
        }
        if (!_logsVisible) {
            _logScrollOffset = 0;
            return;
        }

        const raylib::Rectangle panel =
            getLogPanelBounds(screenWidth, screenHeight);
        const int visibleLogCount = getVisibleLogCount(panel);
        const int maximumScroll =
            std::max(0, static_cast<int>(state.logs.size()) - visibleLogCount);

        if (panel.CheckCollision(mousePosition)) {
            const int scroll = static_cast<int>(raylib::Mouse::GetWheelMove());

            _logScrollOffset = std::clamp(
                _logScrollOffset + (scroll * logScrollStep), 0, maximumScroll);
        } else {
            _logScrollOffset = std::min(_logScrollOffset, maximumScroll);
        }
    }

    bool WorldRenderer::_isMouseOverLogInterface() const
    {
        const Vector2 mousePosition = raylib::Mouse::GetPosition();
        const int screenWidth = _window.GetWidth();
        const int screenHeight = _window.GetHeight();

        return getLogButtonBounds(screenWidth).CheckCollision(mousePosition) ||
               (_logsVisible && getLogPanelBounds(screenWidth, screenHeight)
                                    .CheckCollision(mousePosition));
    }

    void WorldRenderer::_drawLogs(const GameState &state) const
    {
        const int screenWidth = _window.GetWidth();
        const int screenHeight = _window.GetHeight();
        const raylib::Rectangle button = getLogButtonBounds(screenWidth);
        const Color buttonColor =
            _logsVisible ? logButtonActiveColor : logButtonColor;

        button.Draw(buttonColor);
        raylib::Text::Draw("Logs", static_cast<int>(button.x) + 14,
                           static_cast<int>(button.y) + 8, logTextSize,
                           RAYWHITE);

        if (!_logsVisible) {
            return;
        }

        const raylib::Rectangle panel =
            getLogPanelBounds(screenWidth, screenHeight);
        const int visibleLogCount = getVisibleLogCount(panel);
        const int maximumScroll =
            std::max(0, static_cast<int>(state.logs.size()) - visibleLogCount);
        const int scrollOffset = std::clamp(_logScrollOffset, 0, maximumScroll);
        const int firstLog = std::max(0, static_cast<int>(state.logs.size()) -
                                             visibleLogCount - scrollOffset);
        const int lastLog = std::min(static_cast<int>(state.logs.size()),
                                     firstLog + visibleLogCount);
        int textY = static_cast<int>(panel.y) + logPanelPadding;

        panel.Draw(logPanelColor);
        raylib::Text::Draw("Protocol logs",
                           static_cast<int>(panel.x) + logPanelPadding, textY,
                           logTitleSize, logTitleColor);
        textY += logTitleSize + logPanelPadding;
        for (int index = firstLog; index < lastLog; ++index) {
            const std::string line = truncateText(
                state.logs.at(static_cast<std::size_t>(index)), 42);

            raylib::Text::Draw(line,
                               static_cast<int>(panel.x) + logPanelPadding,
                               textY, logTextSize, hudSecondaryColor);
            textY += logLineHeight;
        }
    }
} // namespace gui
