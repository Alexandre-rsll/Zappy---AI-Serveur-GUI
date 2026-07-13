/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** WorldRenderer
*/

#include "WorldRendererInternals.hpp"

namespace gui {

    void WorldRenderer::_drawWorld(const GameState &state, const int width,
                                   const int height, const float deltaTime)
    {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const std::size_t tileIndex =
                    static_cast<std::size_t>(y * width + x);
                const std::size_t textureIndex =
                    _tileTextureIndices.at(tileIndex);

                _drawTile(x, y, width, height, _grassTextures.at(textureIndex));
            }
        }
        _drawResources(state, width, height);
        _drawEggs(state, width, height);
        _drawPlayers(state, width, height, deltaTime);
    }

    void WorldRenderer::_drawEggs(const GameState &state, const int width,
                                  const int height) const
    {
        const float morph = smoothStep(_donutFactor);

        if (!_eggModel.has_value()) {
            return;
        }
        for (const auto &[id, egg] : state.eggs) {
            static_cast<void>(id);
            const int x = wrapCoordinate(egg.x, width);
            const int y = wrapCoordinate(egg.y, height);
            const TileSurface surface =
                getTileSurface(x, y, width, height, morph);
            const Vector3 position =
                offsetOnSurface(surface, 0.0F, 0.0F, _eggSurfaceOffset);

            _eggModel->Draw(position, rotationAxisForNormal(surface.normal),
                            rotationAngleForNormal(surface.normal),
                            Vector3{.x = eggModelScale,
                                    .y = eggModelScale,
                                    .z = eggModelScale},
                            WHITE);
        }
    }

    void WorldRenderer::_drawResources(const GameState &state, const int width,
                                       const int height) const
    {
        const float morph = smoothStep(_donutFactor);

        if (state.tiles.empty()) {
            return;
        }
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const std::size_t tileIndex =
                    static_cast<std::size_t>(y * width + x);

                if (tileIndex >= state.tiles.size()) {
                    continue;
                }
                const TileSurface surface =
                    getTileSurface(x, y, width, height, morph);
                const Tile &tile = state.tiles.at(tileIndex);
                int visibleResourceCount = 0;

                for (int quantity : tile.resources) {
                    if (quantity > 0) {
                        ++visibleResourceCount;
                    }
                }
                if (visibleResourceCount == 0) {
                    continue;
                }
                int visibleIndex = 0;
                for (std::size_t resourceIndex = 0;
                     resourceIndex < tile.resources.size(); ++resourceIndex) {
                    const int quantity = tile.resources.at(resourceIndex);

                    if (quantity <= 0 ||
                        !_resourceModels.at(resourceIndex).has_value()) {
                        continue;
                    }
                    const float angle =
                        (visibleResourceCount == 1)
                            ? 0.0F
                            : (static_cast<float>(visibleIndex) /
                               static_cast<float>(visibleResourceCount)) *
                                  fullTurn;
                    const float radialDistance =
                        (visibleResourceCount == 1) ? 0.0F : resourceRingRadius;
                    const int visibleStack =
                        std::min(quantity, maxVisibleStackPerResource);

                    for (int stackIndex = 0; stackIndex < visibleStack;
                         ++stackIndex) {
                        const Vector3 position = offsetOnSurface(
                            surface, std::cos(angle) * radialDistance,
                            std::sin(angle) * radialDistance,
                            resourceStackHeight *
                                static_cast<float>(stackIndex));

                        _resourceModels.at(resourceIndex)
                            ->Draw(position,
                                   rotationAxisForNormal(surface.normal),
                                   rotationAngleForNormal(surface.normal),
                                   Vector3{.x = resourceBaseScale,
                                           .y = resourceBaseScale,
                                           .z = resourceBaseScale},
                                   WHITE);
                    }
                    ++visibleIndex;
                }
            }
        }
    }

    void WorldRenderer::_drawSkybox(const float worldSpan) const
    {
        const float scale =
            std::max(minSkyboxScale, worldSpan * skyboxScaleFactor);
        const Vector3 center = _camera.position;

        if (!_skyboxSphere.has_value()) {
            return;
        }
        rlDisableBackfaceCulling();
        rlDisableDepthTest();
        rlDisableDepthMask();
        _skyboxSphere->Draw(center, cameraUp, skyboxRotationDegrees,
                            Vector3{.x = scale, .y = scale, .z = scale}, WHITE);
        rlEnableDepthMask();
        rlEnableDepthTest();
        rlDisableBackfaceCulling();
    }

    void WorldRenderer::_drawTile(const int x, const int y, int width,
                                  int height,
                                  const raylib::Texture2D &texture) const
    {
        const float x0 =
            ((static_cast<float>(x) - (static_cast<float>(width) / 2.0F)) *
             tileWorldSize) +
            tileGap;
        const float x1 = x0 + tileWorldSize - (tileGap * 2.0F);
        const float z0 =
            ((static_cast<float>(y) - (static_cast<float>(height) / 2.0F)) *
             tileWorldSize) +
            tileGap;
        const float z1 = z0 + tileWorldSize - (tileGap * 2.0F);
        const std::array<Vector3, 4> flatTop = {
            Vector3{.x = x0, .y = 0.0F, .z = z1},
            Vector3{.x = x1, .y = 0.0F, .z = z1},
            Vector3{.x = x1, .y = 0.0F, .z = z0},
            Vector3{.x = x0, .y = 0.0F, .z = z0},
        };
        const TorusLayout layout = buildTorusLayout(width, height);
        const auto angleFor = [width, height, &layout](const float tileX,
                                                       const float tileY) {
            if (layout.yIsMajorAxis) {
                return std::array{tileY / static_cast<float>(height) * fullTurn,
                                  tileX / static_cast<float>(width) * fullTurn};
            }
            return std::array{tileX / static_cast<float>(width) * fullTurn,
                              tileY / static_cast<float>(height) * fullTurn};
        };
        const std::array angles = {
            angleFor(static_cast<float>(x) + tileGap,
                     static_cast<float>(y + 1) - tileGap),
            angleFor(static_cast<float>(x + 1) - tileGap,
                     static_cast<float>(y + 1) - tileGap),
            angleFor(static_cast<float>(x + 1) - tileGap,
                     static_cast<float>(y) + tileGap),
            angleFor(static_cast<float>(x) + tileGap,
                     static_cast<float>(y) + tileGap),
        };
        const std::array<Vector3, 4> torusTop = {
            torusPoint(angles[0][0], angles[0][1], layout.majorRadius,
                       layout.minorRadius),
            torusPoint(angles[1][0], angles[1][1], layout.majorRadius,
                       layout.minorRadius),
            torusPoint(angles[2][0], angles[2][1], layout.majorRadius,
                       layout.minorRadius),
            torusPoint(angles[3][0], angles[3][1], layout.majorRadius,
                       layout.minorRadius),
        };
        const std::array<Vector3, 4> torusNormals = {
            torusNormal(angles[0][0], angles[0][1]),
            torusNormal(angles[1][0], angles[1][1]),
            torusNormal(angles[2][0], angles[2][1]),
            torusNormal(angles[3][0], angles[3][1]),
        };
        std::array<Vector3, 4> top{};
        std::array<Vector3, 4> normals{};

        for (std::size_t index = 0; index < top.size(); ++index) {
            top[index] =
                morphedVertex(flatTop[index], torusTop[index], _donutFactor);
            normals[index] = morphedNormal(flatWorldNormal, torusNormals[index],
                                           _donutFactor);
        }

        drawTexturedFace(texture, top, normals, WHITE, false);
    }

    void WorldRenderer::_drawHud(const GameState &state, const int width,
                                 const int height) const
    {
        const char *mode = _donutMode ? "Donut" : "Flat";
        static constexpr std::string title = "Zappy GUI";
        const std::string mapText =
            "Map " + std::to_string(width) + "x" + std::to_string(height);
        const std::string modeText = std::string("Mode ") + mode;
        const std::string teamsText =
            "Teams " + std::to_string(state.teams.size()) + "  Players " +
            std::to_string(state.players.size());

        raylib::Rectangle(hudBounds).Draw(hudBackgroundColor);
        raylib::Text::Draw(title, hudLeft, hudTitleY, hudTitleSize, RAYWHITE);
        raylib::Text::Draw(mapText, hudLeft, hudMapY, hudTextSize,
                           hudSecondaryColor);
        raylib::Text::Draw(modeText, hudLeft, hudModeY, hudTextSize,
                           hudModeColor);
        raylib::Text::Draw(teamsText, hudLeft, hudTeamsY, hudTextSize,
                           hudSecondaryColor);

        if (!state.lastServerMessage.empty()) {
            raylib::Text::Draw(state.lastServerMessage, hudMessageX, hudTitleY,
                               hudTextSize, hudMessageColor);
        }
    }

    void WorldRenderer::_drawStatistics(const GameState &state) const
    {
        const int panelY =
            _window.GetHeight() - statisticsPanelHeight - statisticsPanelMargin;
        const int toggleY = _statisticsVisible
                                ? panelY + statisticsPanelPadding
                                : _window.GetHeight() - statisticsToggleSize -
                                      statisticsPanelMargin;
        const raylib::Rectangle toggle(
            static_cast<float>(statisticsPanelMargin + statisticsPanelWidth -
                               statisticsToggleSize - statisticsPanelPadding),
            static_cast<float>(toggleY),
            static_cast<float>(statisticsToggleSize),
            static_cast<float>(statisticsToggleSize));

        if (!_statisticsVisible) {
            toggle.Draw(hudBackgroundColor);
            raylib::Text::Draw("+", static_cast<int>(toggle.x) + 8,
                               static_cast<int>(toggle.y) + 3, hudTitleSize,
                               hudSecondaryColor);
            return;
        }

        std::array<int, playerLevelCount> playersByLevel{};
        std::array<long long, resourceCount> mapResources{};
        const raylib::Rectangle panel(
            static_cast<float>(statisticsPanelMargin),
            static_cast<float>(panelY),
            static_cast<float>(statisticsPanelWidth),
            static_cast<float>(statisticsPanelHeight));
        int textY = panelY + statisticsPanelPadding;

        for (const auto &[id, player] : state.players) {
            static_cast<void>(id);
            const int level =
                std::clamp(player.level, 1, static_cast<int>(playerLevelCount));

            ++playersByLevel.at(static_cast<std::size_t>(level - 1));
        }
        for (const Tile &tile : state.tiles) {
            for (std::size_t index = 0; index < tile.resources.size();
                 ++index) {
                mapResources.at(index) += tile.resources.at(index);
            }
        }

        panel.Draw(hudBackgroundColor);
        raylib::Text::Draw("World statistics",
                           statisticsPanelMargin + statisticsPanelPadding,
                           textY, statisticsTitleSize, hudModeColor);
        toggle.Draw(hudBackgroundColor);
        raylib::Text::Draw("x", static_cast<int>(toggle.x) + 8,
                           static_cast<int>(toggle.y) + 3, hudTitleSize,
                           hudSecondaryColor);
        textY += statisticsTitleSize + 8;
        raylib::Text::Draw("Alive: " + std::to_string(state.players.size()) +
                               "  Dead: " + std::to_string(state.totalDeaths),
                           statisticsPanelMargin + statisticsPanelPadding,
                           textY, hudTextSize, hudSecondaryColor);
        textY += statisticsLineHeight + 8;
        raylib::Text::Draw("Players by level",
                           statisticsPanelMargin + statisticsPanelPadding,
                           textY, hudTextSize, hudMessageColor);
        textY += statisticsLineHeight;
        for (std::size_t index = 0; index < playersByLevel.size(); ++index) {
            const int column = static_cast<int>(index % 2);
            const int row = static_cast<int>(index / 2);
            const int textX =
                statisticsPanelMargin + statisticsPanelPadding + (column * 145);
            const int levelY = textY + (row * statisticsLineHeight);

            raylib::Text::Draw("L" + std::to_string(index + 1) + ": " +
                                   std::to_string(playersByLevel.at(index)),
                               textX, levelY, hudTextSize, hudSecondaryColor);
        }
        textY += statisticsLineHeight * 4 + 8;
        raylib::Text::Draw("Map resources",
                           statisticsPanelMargin + statisticsPanelPadding,
                           textY, hudTextSize, hudMessageColor);
        textY += statisticsLineHeight;
        for (std::size_t index = 0; index < mapResources.size(); ++index) {
            const int column = static_cast<int>(index % 2);
            const int row = static_cast<int>(index / 2);
            const int textX =
                statisticsPanelMargin + statisticsPanelPadding + (column * 165);
            const int resourceY = textY + (row * statisticsLineHeight);

            raylib::Text::Draw(
                std::string(resourceModelNames.at(index)) + ": " +
                    std::to_string(mapResources.at(index)),
                textX, resourceY, hudTextSize, hudSecondaryColor);
        }
    }

    void WorldRenderer::_updateStatisticsMenu()
    {
        if (!raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_LEFT)) {
            return;
        }

        const int panelY =
            _window.GetHeight() - statisticsPanelHeight - statisticsPanelMargin;
        const int toggleY = _statisticsVisible
                                ? panelY + statisticsPanelPadding
                                : _window.GetHeight() - statisticsToggleSize -
                                      statisticsPanelMargin;
        const raylib::Rectangle toggle(
            static_cast<float>(statisticsPanelMargin + statisticsPanelWidth -
                               statisticsToggleSize - statisticsPanelPadding),
            static_cast<float>(toggleY),
            static_cast<float>(statisticsToggleSize),
            static_cast<float>(statisticsToggleSize));

        if (toggle.CheckCollision(raylib::Mouse::GetPosition())) {
            _statisticsVisible = !_statisticsVisible;
        }
    }

    bool WorldRenderer::_isMouseOverStatisticsMenu() const
    {
        const int panelY =
            _window.GetHeight() - statisticsPanelHeight - statisticsPanelMargin;
        const int toggleY = _statisticsVisible
                                ? panelY + statisticsPanelPadding
                                : _window.GetHeight() - statisticsToggleSize -
                                      statisticsPanelMargin;
        const raylib::Rectangle toggle(
            static_cast<float>(statisticsPanelMargin + statisticsPanelWidth -
                               statisticsToggleSize - statisticsPanelPadding),
            static_cast<float>(toggleY),
            static_cast<float>(statisticsToggleSize),
            static_cast<float>(statisticsToggleSize));
        const raylib::Rectangle panel(
            static_cast<float>(statisticsPanelMargin),
            static_cast<float>(panelY),
            static_cast<float>(statisticsPanelWidth),
            static_cast<float>(statisticsPanelHeight));
        const Vector2 mousePosition = raylib::Mouse::GetPosition();

        return toggle.CheckCollision(mousePosition) ||
               (_statisticsVisible && panel.CheckCollision(mousePosition));
    }

} // namespace gui
