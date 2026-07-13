/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** WorldRenderer
*/

#include "WorldRendererInternals.hpp"

namespace gui {

    void WorldRenderer::_loadAudio()
    {
        _audioDevice.emplace();

        const std::filesystem::path soundRoot = assetsPath() / "sounds";
        const std::filesystem::path evolutionPath =
            soundRoot / evolutionMusicName;

        if (std::filesystem::exists(evolutionPath)) {
            _evolutionMusic.emplace(evolutionPath.string());
            _evolutionMusic->SetLooping(true);
            _evolutionMusic->SetVolume(evolutionMusicVolume);
        }
        _loadNextBackgroundMusic();
    }

    void WorldRenderer::_unloadAudio()
    {
        _backgroundMusic.reset();
        _evolutionMusic.reset();
        _audioDevice.reset();
    }

    void WorldRenderer::_loadNextBackgroundMusic()
    {
        if (!_audioDevice.has_value()) {
            return;
        }
        _backgroundMusic.reset();

        const std::filesystem::path soundRoot = assetsPath() / "sounds";

        for (std::size_t attempt = 0; attempt < backgroundMusicNames.size();
             ++attempt) {
            _backgroundMusicIndex =
                (_backgroundMusicIndex + 1) %
                static_cast<int>(backgroundMusicNames.size());
            const std::filesystem::path musicPath =
                soundRoot / backgroundMusicNames.at(_backgroundMusicIndex);

            if (!std::filesystem::exists(musicPath)) {
                continue;
            }
            _backgroundMusic.emplace(musicPath.string());
            _backgroundMusic->SetLooping(false);
            _backgroundMusic->SetVolume(backgroundMusicVolume);
            _backgroundMusic->Play();
            return;
        }
    }

    void WorldRenderer::_updateAudio(const GameState &state)
    {
        if (!_audioDevice.has_value()) {
            return;
        }
        if (_backgroundMusic.has_value()) {
            _backgroundMusic->Update();
            if (!_backgroundMusic->IsPlaying()) {
                _loadNextBackgroundMusic();
            }
        } else {
            _loadNextBackgroundMusic();
        }
        if (!_evolutionMusic.has_value()) {
            return;
        }

        _evolutionMusic->Update();
        if (!state.incantations.empty()) {
            if (!_evolutionMusic->IsPlaying()) {
                _evolutionMusic->Play();
            }
        } else if (_evolutionMusic->IsPlaying()) {
            _evolutionMusic->Stop();
        }
    }

    void WorldRenderer::_loadWorldShader()
    {
        const std::filesystem::path shaderRoot = assetsPath() / "shaders";
        const std::filesystem::path vertexPath =
            shaderRoot / "world_lighting.vs";
        const std::filesystem::path fragmentPath =
            shaderRoot / "world_lighting.fs";

        _worldShader =
            raylib::Shader(vertexPath.string(), fragmentPath.string());
    }

    void WorldRenderer::_loadSkybox()
    {
        const std::filesystem::path skyboxTexturePath =
            assetsPath() / "textures" / "skybox.jpeg";

        _skyboxTexture = raylib::Texture2D(skyboxTexturePath.string());
        _skyboxTexture.SetFilter(TEXTURE_FILTER_BILINEAR);
        _skyboxTexture.SetWrap(TEXTURE_WRAP_REPEAT);
        _skyboxSphere.emplace(buildSkyboxMesh());
        _skyboxTexture.SetMaterial(&_skyboxSphere->materials[0],
                                   MATERIAL_MAP_DIFFUSE);
    }

    void WorldRenderer::_updateWorldShader()
    {
        const Vector3 normalizedSunDirection = Vector3Normalize(sunDirection);
        const auto sun = colorToShaderValue(sunColor);
        const auto ambient = colorToShaderValue(ambientLightColor);
        constexpr std::array diffuse = {1.0F, 1.0F, 1.0F, 1.0F};

        _worldShader.SetValue(_worldShader.GetLocation("lightDirection"),
                              &normalizedSunDirection, SHADER_UNIFORM_VEC3);
        _worldShader.SetValue(_worldShader.GetLocation("lightColor"),
                              sun.data(), SHADER_UNIFORM_VEC4);
        _worldShader.SetValue(_worldShader.GetLocation("ambientColor"),
                              ambient.data(), SHADER_UNIFORM_VEC4);
        _worldShader.SetValue(_worldShader.GetLocation("viewPosition"),
                              &_camera.position, SHADER_UNIFORM_VEC3);
        _worldShader.SetValue(_worldShader.GetLocation("rimStrength"),
                              &rimLightStrength, SHADER_UNIFORM_FLOAT);
        _worldShader.SetValue(_worldShader.GetLocation("colDiffuse"),
                              diffuse.data(), SHADER_UNIFORM_VEC4);
    }

    void WorldRenderer::_loadGrassTextures()
    {
        const std::filesystem::path textureRoot = assetsPath() / "textures";

        for (int index = firstGrassTextureIndex; index <= lastGrassTextureIndex;
             ++index) {
            const std::filesystem::path texturePath =
                textureRoot / ("grass" + std::to_string(index) + ".png");

            if (!std::filesystem::exists(texturePath)) {
                continue;
            }
            _grassTextures.emplace_back(texturePath.string());
            _grassTextures.back().SetFilter(TEXTURE_FILTER_POINT);
            _grassTextures.back().SetWrap(TEXTURE_WRAP_CLAMP);
        }
        if (_grassTextures.empty()) {
            throw std::runtime_error("No grass texture found in " +
                                     textureRoot.string());
        }
    }

    void WorldRenderer::_loadResourceModels()
    {
        const std::filesystem::path resourceRoot =
            assetsPath() / "objects" / "resources";

        for (std::size_t index = 0; index < resourceModelNames.size();
             ++index) {
            const std::filesystem::path modelPath =
                resourceRoot /
                (std::string(resourceModelNames.at(index)) + ".gltf");

            _resourceModels.at(index).emplace(modelPath.string());
            for (int materialIndex = 0;
                 materialIndex < _resourceModels.at(index)->materialCount;
                 ++materialIndex) {
                _resourceModels.at(index)->materials[materialIndex].shader =
                    _worldShader;
            }
        }
    }

    void WorldRenderer::_loadEggModel()
    {
        const std::filesystem::path eggModelPath =
            assetsPath() / "objects" / "egg.glb";

        _eggModel.emplace(eggModelPath.string());
        const BoundingBox bounds = _eggModel->GetBoundingBox();

        _eggSurfaceOffset =
            (-bounds.min.y * eggModelScale) + eggGroundClearance;
        for (int materialIndex = 0; materialIndex < _eggModel->materialCount;
             ++materialIndex) {
            _eggModel->materials[materialIndex].shader = _worldShader;
        }
    }

    void WorldRenderer::_loadPlayerModel()
    {
        const std::filesystem::path playerModelRoot = assetsPath() / "objects";

        for (std::size_t index = 0; index < _playerModels.size(); ++index) {
            const std::filesystem::path playerModelPath =
                playerModelRoot /
                ("slime_level" + std::to_string(index + 1) + ".gltf");

            _playerModels.at(index).emplace(playerModelPath.string());
            for (int materialIndex = 0;
                 materialIndex < _playerModels.at(index)->materialCount;
                 ++materialIndex) {
                _playerModels.at(index)->materials[materialIndex].shader =
                    _worldShader;
            }
        }
        const std::filesystem::path animationPath =
            playerModelRoot / "slime_level1.gltf";
        _playerAnimations =
            raylib::ModelAnimation::Load(animationPath.string());
        _playerIdleAnimationIndex =
            findAnimationIndex(_playerAnimations, "idle");
        _playerJumpAnimationIndex =
            findAnimationIndex(_playerAnimations, "jump");
        _playerTurnAnimationIndex =
            findAnimationIndex(_playerAnimations, "turn");
    }

    void WorldRenderer::_refreshTileTextures(const int width, const int height)
    {
        if (_knownWidth == width && _knownHeight == height) {
            return;
        }

        _knownWidth = width;
        _knownHeight = height;
        _tileTextureIndices.clear();
        _tileTextureIndices.resize(static_cast<std::size_t>(width) * height);

        std::uniform_int_distribution<std::size_t> distribution(
            0, _grassTextures.size() - 1);

        for (std::size_t &textureIndex : _tileTextureIndices) {
            textureIndex = distribution(_randomGenerator);
        }
    }

} // namespace gui
