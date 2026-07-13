/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** WorldRenderer
*/

#ifndef WORLDRENDERER_HPP
#define WORLDRENDERER_HPP

#include <array>
#include <cstddef>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "gui_protocol/GameState.hpp"
#include "raylib-cpp.hpp"

namespace gui {

    class WorldRenderer {
    public:
        WorldRenderer();
        ~WorldRenderer();

        WorldRenderer(const WorldRenderer &) = delete;
        WorldRenderer &operator=(const WorldRenderer &) = delete;

        [[nodiscard]] bool shouldClose() const;

        void render(const GameState &state);

    private:
        enum class PlayerAnimation {
            Idle,
            Jump,
            Turn,
        };

        enum class Selection {
            None,
            Tile,
            Player,
        };

        struct PlayerRenderState {
            float yaw = 0.0F;
            float visualX = 0.0F;
            float visualY = 0.0F;
            float startX = 0.0F;
            float startY = 0.0F;
            float targetX = 0.0F;
            float targetY = 0.0F;
            float moveElapsed = 0.0F;
            int targetTileX = 0;
            int targetTileY = 0;
            int level = 0;
            float animationFrame = 0.0F;
            float turnAnimationTime = 0.0F;
            float jumpAnimationTime = 0.0F;
            float broadcastElapsed = 0.0F;
            std::size_t broadcastVersion = 0;
            std::string broadcastMessage;
            PlayerAnimation animation = PlayerAnimation::Idle;
            bool moving = false;
        };

        static constexpr int windowWidth = 1280;
        static constexpr int windowHeight = 720;
        static constexpr float transitionSpeed = 3.0F;
        static constexpr float baseOrbitYaw = 0.25F;
        static constexpr float baseOrbitPitch = 0.52F;
        static constexpr std::size_t playerLevelCount = 8;

        raylib::Window _window;
        raylib::Camera3D _camera;
        raylib::Shader _worldShader;
        raylib::Texture2D _skyboxTexture;
        std::optional<raylib::AudioDevice> _audioDevice;
        std::optional<raylib::Music> _backgroundMusic;
        std::optional<raylib::Music> _evolutionMusic;
        std::optional<raylib::Model> _skyboxSphere;
        std::optional<raylib::Model> _eggModel;
        std::array<std::optional<raylib::Model>, playerLevelCount>
            _playerModels;
        std::vector<raylib::ModelAnimation> _playerAnimations;
        std::array<std::optional<raylib::Model>, resourceCount> _resourceModels;
        std::vector<raylib::Texture2D> _grassTextures;
        std::vector<std::size_t> _tileTextureIndices;
        std::mt19937 _randomGenerator;

        int _knownWidth = 0;
        int _knownHeight = 0;
        bool _donutMode = false;
        float _donutFactor = 0.0F;
        float _orbitYaw = baseOrbitYaw;
        float _orbitPitch = baseOrbitPitch;
        float _orbitDistance = 0.0F;
        float _eggSurfaceOffset = 0.0F;
        std::unordered_map<int, PlayerRenderState> _playerStates;
        int _playerIdleAnimationIndex = -1;
        int _playerJumpAnimationIndex = -1;
        int _playerTurnAnimationIndex = -1;
        int _backgroundMusicIndex = -1;
        int _logScrollOffset = 0;
        bool _logsVisible = false;
        bool _statisticsVisible = true;
        Selection _selection = Selection::None;
        int _selectedTileX = 0;
        int _selectedTileY = 0;
        int _selectedPlayerId = 0;

        void _loadAudio();
        void _unloadAudio();
        void _loadNextBackgroundMusic();
        void _updateAudio(const GameState &state);
        void _loadGrassTextures();
        void _loadEggModel();
        void _loadPlayerModel();
        void _loadResourceModels();
        void _loadSkybox();
        void _loadWorldShader();
        void _refreshTileTextures(int width, int height);
        void _updateCamera(int width, int height, float deltaTime);
        void _updateWorldShader();
        void _updateTransition(float deltaTime);
        void _updateSelection(const GameState &state, int width, int height);
        void _updateLogs(const GameState &state);
        void _updateStatisticsMenu();
        [[nodiscard]] bool _isMouseOverLogInterface() const;
        [[nodiscard]] bool _isMouseOverStatisticsMenu() const;

        void _drawWorld(const GameState &state, int width, int height,
                        float deltaTime);
        void _drawPlayers(const GameState &state, int width, int height,
                          float deltaTime);
        void _drawResources(const GameState &state, int width,
                            int height) const;
        void _drawEggs(const GameState &state, int width, int height) const;
        void _drawSkybox(float worldSpan) const;
        void _drawTile(int x, int y, int width, int height,
                       const raylib::Texture2D &texture) const;
        void _drawHud(const GameState &state, int width, int height) const;
        void _drawStatistics(const GameState &state) const;
        void _drawSelectionInfo(const GameState &state, int width,
                                int height) const;
        void _drawBroadcastMessages(int width, int height) const;
        void _drawLogs(const GameState &state) const;
    };

} // namespace gui

#endif /* WORLDRENDERER_HPP */
