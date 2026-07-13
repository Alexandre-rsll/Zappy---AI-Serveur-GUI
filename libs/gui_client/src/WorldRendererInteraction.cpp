/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** World renderer interaction
*/

#include "WorldRendererInternals.hpp"

namespace gui {
    namespace {
        constexpr float playerSelectionRadius = 26.0F;
        constexpr float tileSelectionHalfSize = tileWorldSize * 0.5F;
        constexpr int selectionPanelMargin = 18;
        constexpr int selectionPanelWidth = 292;
        constexpr int selectionPanelPadding = 14;
        constexpr int selectionPanelLineHeight = 18;
        constexpr int selectionPanelTitleSize = 20;
        constexpr int selectionPanelTextSize = 16;
        constexpr Color selectionPanelColor = {
            .r = 10, .g = 14, .b = 18, .a = 210};
        constexpr Color selectionTitleColor = {
            .r = 255, .g = 236, .b = 158, .a = 255};

        [[nodiscard]] const char *orientationName(const int orientation)
        {
            switch (orientation) {
            case 1:
                return "North";
            case 2:
                return "East";
            case 3:
                return "South";
            case 4:
                return "West";
            default:
                return "Unknown";
            }
        }

        [[nodiscard]] bool isInFrontOfCamera(const Camera3D &camera,
                                             const Vector3 position)
        {
            const Vector3 cameraForward = Vector3Normalize(
                Vector3Subtract(camera.target, camera.position));
            const Vector3 toPosition =
                Vector3Subtract(position, camera.position);

            return Vector3DotProduct(cameraForward, toPosition) > 0.0F;
        }

        [[nodiscard]] raylib::RayCollision
        getTileCollision(const raylib::Ray &ray, const TileSurface &surface)
        {
            const Vector3 bottomLeft = offsetOnSurface(
                surface, -tileSelectionHalfSize, -tileSelectionHalfSize, 0.0F);
            const Vector3 topLeft = offsetOnSurface(
                surface, -tileSelectionHalfSize, tileSelectionHalfSize, 0.0F);
            const Vector3 topRight = offsetOnSurface(
                surface, tileSelectionHalfSize, tileSelectionHalfSize, 0.0F);
            const Vector3 bottomRight = offsetOnSurface(
                surface, tileSelectionHalfSize, -tileSelectionHalfSize, 0.0F);
            const raylib::RayCollision first =
                ray.GetCollision(bottomLeft, topLeft, topRight);
            const raylib::RayCollision second =
                ray.GetCollision(bottomLeft, topRight, bottomRight);

            if (!first.hit) {
                return second;
            }
            if (!second.hit || first.distance < second.distance) {
                return first;
            }
            return second;
        }
    } // namespace

    void WorldRenderer::_updateSelection(const GameState &state,
                                         const int width, const int height)
    {
        if (_selection == Selection::Player &&
            !state.players.contains(_selectedPlayerId)) {
            _selection = Selection::None;
        }
        if (_selection == Selection::Tile &&
            (_selectedTileX >= width || _selectedTileY >= height)) {
            _selection = Selection::None;
        }
        if (!raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_LEFT) ||
            _isMouseOverLogInterface() || _isMouseOverStatisticsMenu()) {
            return;
        }

        const Vector2 mousePosition = raylib::Mouse::GetPosition();
        float closestPlayerDistance =
            playerSelectionRadius * playerSelectionRadius;
        std::optional<int> closestPlayerId;

        for (const auto &entry : state.players) {
            const int playerId = entry.first;
            const auto renderState = _playerStates.find(playerId);

            if (renderState == _playerStates.end()) {
                continue;
            }
            const TileSurface surface = getTileSurfaceAt(
                renderState->second.visualX, renderState->second.visualY, width,
                height, smoothStep(_donutFactor));
            const Vector3 position =
                offsetOnSurface(surface, 0.0F, 0.0F, playerSurfaceOffset);

            if (!isInFrontOfCamera(_camera, position)) {
                continue;
            }
            const Vector2 screenPosition = _camera.GetWorldToScreen(position);
            const float distanceX = mousePosition.x - screenPosition.x;
            const float distanceY = mousePosition.y - screenPosition.y;
            const float squaredDistance =
                (distanceX * distanceX) + (distanceY * distanceY);

            if (squaredDistance < closestPlayerDistance) {
                closestPlayerDistance = squaredDistance;
                closestPlayerId = playerId;
            }
        }
        if (closestPlayerId.has_value()) {
            _selection = Selection::Player;
            _selectedPlayerId = *closestPlayerId;
            return;
        }

        const raylib::Ray ray = _camera.GetMouseRay(mousePosition);
        float closestTileDistance = 0.0F;
        int tileX = 0;
        int tileY = 0;
        bool foundTile = false;
        const float morph = smoothStep(_donutFactor);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const raylib::RayCollision collision = getTileCollision(
                    ray, getTileSurface(x, y, width, height, morph));

                if (!collision.hit ||
                    (foundTile && collision.distance >= closestTileDistance)) {
                    continue;
                }
                foundTile = true;
                closestTileDistance = collision.distance;
                tileX = x;
                tileY = y;
            }
        }
        if (foundTile) {
            _selection = Selection::Tile;
            _selectedTileX = tileX;
            _selectedTileY = tileY;
        } else {
            _selection = Selection::None;
        }
    }

    void WorldRenderer::_drawSelectionInfo(const GameState &state,
                                           const int width,
                                           const int height) const
    {
        if (_selection == Selection::None) {
            return;
        }

        std::vector<std::string> lines;
        std::string title;

        if (_selection == Selection::Player) {
            const auto player = state.players.find(_selectedPlayerId);

            if (player == state.players.end()) {
                return;
            }
            title = "Player #" + std::to_string(player->second.id);
            lines.emplace_back("Team: " + player->second.teamName);
            lines.emplace_back("Level: " +
                               std::to_string(player->second.level));
            lines.emplace_back("Position: " + std::to_string(player->second.x) +
                               ", " + std::to_string(player->second.y));
            lines.emplace_back(
                "Direction: " +
                std::string(orientationName(player->second.orientation)));
            for (std::size_t index = 0; index < resourceModelNames.size();
                 ++index) {
                lines.emplace_back(
                    std::string(resourceModelNames.at(index)) + ": " +
                    std::to_string(player->second.inventory.at(index)));
            }
        } else {
            const std::size_t tileIndex = static_cast<std::size_t>(
                _selectedTileY * width + _selectedTileX);

            if (tileIndex >= state.tiles.size()) {
                return;
            }
            title = "Tile " + std::to_string(_selectedTileX) + ", " +
                    std::to_string(_selectedTileY);
            for (std::size_t index = 0; index < resourceModelNames.size();
                 ++index) {
                lines.emplace_back(
                    std::string(resourceModelNames.at(index)) + ": " +
                    std::to_string(
                        state.tiles.at(tileIndex).resources.at(index)));
            }
        }

        const int panelHeight =
            selectionPanelPadding * 2 + selectionPanelTitleSize +
            static_cast<int>(lines.size()) * selectionPanelLineHeight;
        const raylib::Rectangle panel(
            static_cast<float>(_logsVisible
                                   ? selectionPanelMargin
                                   : _window.GetWidth() - selectionPanelWidth -
                                         selectionPanelMargin),
            static_cast<float>(_logsVisible ? 138 : selectionPanelMargin),
            static_cast<float>(selectionPanelWidth),
            static_cast<float>(panelHeight));
        int textY = static_cast<int>(panel.y) + selectionPanelPadding;

        panel.Draw(selectionPanelColor);
        raylib::Text::Draw(title,
                           static_cast<int>(panel.x) + selectionPanelPadding,
                           textY, selectionPanelTitleSize, selectionTitleColor);
        textY += selectionPanelTitleSize + selectionPanelPadding / 2;
        for (const std::string &line : lines) {
            raylib::Text::Draw(
                line, static_cast<int>(panel.x) + selectionPanelPadding, textY,
                selectionPanelTextSize, hudSecondaryColor);
            textY += selectionPanelLineHeight;
        }
    }
} // namespace gui
