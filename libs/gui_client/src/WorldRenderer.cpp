/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** WorldRenderer
*/

#include "WorldRendererInternals.hpp"

namespace gui {

    WorldRenderer::WorldRenderer()
        : _window(windowWidth, windowHeight, "Zappy GUI",
                  FLAG_WINDOW_RESIZABLE),
          _camera(initialCameraPosition, worldCenter, cameraUp, cameraFovy,
                  CAMERA_PERSPECTIVE),
          _randomGenerator(std::random_device{}())
    {
        _window.SetTargetFPS(targetFrameRate);
        rlDisableBackfaceCulling();
        _loadAudio();
        _loadWorldShader();
        _loadSkybox();
        _loadGrassTextures();
        _loadEggModel();
        _loadPlayerModel();
        _loadResourceModels();
    }

    WorldRenderer::~WorldRenderer()
    {
        _unloadAudio();
    }

    bool WorldRenderer::shouldClose() const
    {
        return _window.ShouldClose();
    }

    void WorldRenderer::render(const GameState &state)
    {
        const int width = state.width > 0 ? state.width : fallbackMapWidth;
        const int height = state.height > 0 ? state.height : fallbackMapHeight;
        const float deltaTime = _window.GetFrameTime();
        const float morph = smoothStep(_donutFactor);
        const float worldSpan =
            lerpFloat(getFlatWorldSpan(width, height),
                      getDonutWorldSpan(width, height), morph);

        _refreshTileTextures(width, height);
        _updateAudio(state);
        _updateTransition(deltaTime);
        _updateCamera(width, height, deltaTime);
        _updateWorldShader();
        _updateLogs(state);
        _updateStatisticsMenu();

        _window.BeginDrawing();
        _window.ClearBackground(backgroundColor);
        _camera.BeginMode();
        _drawSkybox(worldSpan);
        _worldShader.BeginMode();
        _drawWorld(state, width, height, deltaTime);
        _worldShader.EndMode();
        _updateSelection(state, width, height);
        _camera.EndMode();
        _drawBroadcastMessages(width, height);
        _drawHud(state, width, height);
        _drawStatistics(state);
        _drawSelectionInfo(state, width, height);
        _drawLogs(state);
        _window.EndDrawing();
    }

} // namespace gui
