/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** WorldRenderer internals
*/

#ifndef WORLDRENDERER_INTERNALS_HPP
#define WORLDRENDERER_INTERNALS_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <numbers>
#include <stdexcept>
#include <string_view>

#include "gui_client/rendering/WorldRenderer.hpp"
#include "raymath.h"
#include "rlgl.h"

#ifndef ZAPPY_ASSETS_DIR
#    define ZAPPY_ASSETS_DIR "gui/assets"
#endif

namespace gui {
    namespace {
        constexpr float pi = std::numbers::pi_v<float>;
        constexpr float fullTurn = 2.0F * pi;
        constexpr int fallbackMapWidth = 20;
        constexpr int fallbackMapHeight = 15;
        constexpr int firstGrassTextureIndex = 1;
        constexpr int lastGrassTextureIndex = 7;
        constexpr int targetFrameRate = 60;
        constexpr float cameraFovy = 45.0F;
        constexpr float cameraTargetYOffset = 0.35F;
        constexpr float mouseOrbitSensitivity = 0.006F;
        constexpr float keyboardOrbitSpeed = 1.7F;
        constexpr float zoomStepFactor = 0.08F;
        constexpr float initialDistanceFactor = 1.45F;
        constexpr float flatMinDistanceFactor = 0.35F;
        constexpr float donutMinDistanceFactor = 0.78F;
        constexpr float maxDistanceFactor = 2.4F;
        constexpr float minCameraDistance = 3.0F;
        constexpr float minMaxCameraDistance = 18.0F;
        constexpr float minOrbitPitch = -1.45F;
        constexpr float maxOrbitPitch = 1.45F;
        constexpr float torusMinimumInnerRadius = 1.2F;
        constexpr float torusMinimumMajorToMinorRatio = 2.25F;
        constexpr float tileWorldSize = 1.0F;
        constexpr float tileGap = 0.0F;
        constexpr float resourceBaseScale = 1.0F;
        constexpr float resourceRingRadius = 0.28F;
        constexpr float resourceStackHeight = 0.10F;
        constexpr int maxVisibleStackPerResource = 3;
        constexpr float playerModelScale = 1.68F;
        constexpr float playerSurfaceOffset = 0.28F;
        constexpr float eggModelScale = 0.50F;
        constexpr float eggGroundClearance = 0.02F;
        constexpr float playerModelYawOffset = -pi * 0.5F;
        constexpr float playerTorusYawOffset = -pi * 0.5F;
        constexpr float playerTurnSpeed = 8.0F;
        constexpr float playerMoveDuration = 0.22F;
        constexpr float playerAnimationFrameRate = 1000.0F / 17.0F;
        constexpr float playerTurnAnimationDuration = 0.45F;
        constexpr float playerTurnAnimationThreshold = 0.05F;
        constexpr float broadcastDisplayDuration = 2.0F;
        constexpr float broadcastWaveMinRadius = 0.6F;
        constexpr float broadcastWaveMaxRadius = 4.8F;
        constexpr float broadcastWaveDistortion = 0.075F;
        constexpr float broadcastWaveHeight = 0.12F;
        constexpr float broadcastWaveLineWidth = 1.6F;
        constexpr int broadcastWaveCount = 3;
        constexpr int broadcastWaveSegmentCount = 48;
        constexpr float playerBroadcastLabelOffset = 1.35F;
        constexpr int broadcastTextSize = 16;
        constexpr int broadcastLabelPadding = 6;
        constexpr std::size_t broadcastMessageCharacterLimit = 54;
        constexpr float backgroundMusicVolume = 0.35F;
        constexpr float evolutionMusicVolume = 0.70F;
        constexpr float playerEvolutionHaloOuterRadius = 0.95F;
        constexpr float playerEvolutionHaloInnerRadius = 0.62F;
        constexpr int playerEvolutionHaloRings = 12;
        constexpr int playerEvolutionHaloSlices = 16;
        constexpr Color playerEvolutionHaloOuterColor = {
            .r = 255, .g = 205, .b = 72, .a = 70};
        constexpr Color playerEvolutionHaloInnerColor = {
            .r = 255, .g = 248, .b = 196, .a = 100};
        constexpr float skyboxScaleFactor = 5.0F;
        constexpr float minSkyboxScale = 80.0F;
        constexpr float skyboxRotationDegrees = 90.0F;
        constexpr int skyboxRingCount = 32;
        constexpr int skyboxSliceCount = 64;
        constexpr float rimLightStrength = 0.22F;
        constexpr Vector3 initialCameraPosition = {
            .x = 10.0F, .y = 13.0F, .z = 18.0F};
        constexpr Vector3 worldCenter = {.x = 0.0F, .y = 0.0F, .z = 0.0F};
        constexpr Vector3 cameraUp = {.x = 0.0F, .y = 1.0F, .z = 0.0F};
        constexpr Vector3 flatWorldNormal = {.x = 0.0F, .y = 1.0F, .z = 0.0F};
        constexpr Vector3 sunDirection = {
            .x = -0.35F, .y = -0.85F, .z = -0.38F};
        constexpr Color sunColor = {.r = 255, .g = 244, .b = 211, .a = 255};
        constexpr Color ambientLightColor = {
            .r = 96, .g = 116, .b = 110, .a = 255};
        constexpr Color backgroundColor = {.r = 21, .g = 27, .b = 31, .a = 255};
        constexpr Color hudBackgroundColor = {
            .r = 10, .g = 14, .b = 18, .a = 190};
        constexpr Color hudSecondaryColor = {
            .r = 202, .g = 214, .b = 205, .a = 255};
        constexpr Color hudModeColor = {.r = 166, .g = 210, .b = 139, .a = 255};
        constexpr Color hudMessageColor = {
            .r = 230, .g = 220, .b = 172, .a = 255};
        constexpr Rectangle hudBounds = {
            .x = 18.0F, .y = 16.0F, .width = 290.0F, .height = 104.0F};
        constexpr int hudLeft = 32;
        constexpr int hudTitleY = 28;
        constexpr int hudMapY = 56;
        constexpr int hudModeY = 78;
        constexpr int hudTeamsY = 100;
        constexpr int hudMessageX = 330;
        constexpr int hudTitleSize = 22;
        constexpr int hudTextSize = 16;
        constexpr int statisticsPanelWidth = 336;
        constexpr int statisticsPanelHeight = 270;
        constexpr int statisticsPanelMargin = 18;
        constexpr int statisticsPanelPadding = 12;
        constexpr int statisticsLineHeight = 16;
        constexpr int statisticsTitleSize = 20;
        constexpr int statisticsToggleSize = 26;
        constexpr std::array<std::string_view, resourceCount>
            resourceModelNames = {"food",     "linemate", "deraumere", "sibur",
                                  "mendiane", "phiras",   "thystame"};
        constexpr std::array<std::string_view, 3> backgroundMusicNames = {
            "music1.mp3", "music2.mp3", "music3.mp3"};
        constexpr std::string_view evolutionMusicName = "evolution.mp3";

        [[nodiscard]] float smoothStep(float value)
        {
            const float clamped = std::clamp(value, 0.0F, 1.0F);

            return clamped * clamped * (3.0F - (2.0F * clamped));
        }

        [[nodiscard]] Vector3 lerp(const Vector3 from, const Vector3 to,
                                   const float amount)
        {
            return {.x = from.x + ((to.x - from.x) * amount),
                    .y = from.y + ((to.y - from.y) * amount),
                    .z = from.z + ((to.z - from.z) * amount)};
        }

        [[nodiscard]] float lerpFloat(const float from, const float to,
                                      const float amount)
        {
            return from + ((to - from) * amount);
        }

        [[nodiscard]] std::array<float, 4> colorToShaderValue(const Color color)
        {
            constexpr float colorChannelScale = 255.0F;

            return {static_cast<float>(color.r) / colorChannelScale,
                    static_cast<float>(color.g) / colorChannelScale,
                    static_cast<float>(color.b) / colorChannelScale,
                    static_cast<float>(color.a) / colorChannelScale};
        }

        [[nodiscard]] Vector3 torusPoint(const float u, const float v,
                                         const float majorRadius,
                                         const float minorRadius)
        {
            const float ring = majorRadius + (minorRadius * std::cos(v));

            return {.x = ring * std::cos(u),
                    .y = minorRadius * std::sin(v),
                    .z = ring * std::sin(u)};
        }

        [[nodiscard]] Vector3 torusNormal(const float u, const float v)
        {
            return Vector3Normalize({.x = std::cos(v) * std::cos(u),
                                     .y = std::sin(v),
                                     .z = std::cos(v) * std::sin(u)});
        }

        [[nodiscard]] Vector3 morphedVertex(const Vector3 flat,
                                            const Vector3 torus,
                                            const float amount)
        {
            return lerp(flat, torus, smoothStep(amount));
        }

        [[nodiscard]] Vector3 morphedNormal(const Vector3 flat,
                                            const Vector3 torus,
                                            const float amount)
        {
            return Vector3Normalize(lerp(flat, torus, smoothStep(amount)));
        }

        struct TorusLayout {
            bool yIsMajorAxis = false;
            float majorRadius = 1.0F;
            float minorRadius = 1.0F;
        };

        struct TileSurface {
            Vector3 center{};
            Vector3 normal{};
            Vector3 right{};
            Vector3 forward{};
        };

        [[nodiscard]] TorusLayout buildTorusLayout(const int width,
                                                   const int height)
        {
            const int majorTiles = std::max(width, height);
            const int minorTiles = std::min(width, height);
            TorusLayout layout{};

            layout.yIsMajorAxis = height > width;
            layout.minorRadius =
                static_cast<float>(minorTiles) * tileWorldSize / fullTurn;
            const float majorRadiusFromTiles =
                static_cast<float>(majorTiles) * tileWorldSize / fullTurn;
            const float minimumMajorRadius =
                std::max(layout.minorRadius + torusMinimumInnerRadius,
                         layout.minorRadius * torusMinimumMajorToMinorRatio);

            layout.majorRadius =
                std::max(majorRadiusFromTiles, minimumMajorRadius);
            return layout;
        }

        [[nodiscard]] float getFlatWorldSpan(const int width, const int height)
        {
            return std::sqrt(
                static_cast<float>((width * width) + (height * height)));
        }

        [[nodiscard]] float getDonutWorldSpan(const int width, const int height)
        {
            const TorusLayout layout = buildTorusLayout(width, height);

            return (layout.majorRadius + layout.minorRadius) * 2.0F;
        }

        [[nodiscard]] std::array<float, 2>
        getTorusAngles(const TorusLayout &layout, const int width,
                       const int height, const float tileX, const float tileY)
        {
            if (layout.yIsMajorAxis) {
                return {tileY / static_cast<float>(height) * fullTurn,
                        tileX / static_cast<float>(width) * fullTurn};
            }
            return {tileX / static_cast<float>(width) * fullTurn,
                    tileY / static_cast<float>(height) * fullTurn};
        }

        [[nodiscard]] TileSurface getTileSurfaceAt(const float x, const float y,
                                                   const int width,
                                                   const int height,
                                                   const float morph)
        {
            const TorusLayout layout = buildTorusLayout(width, height);
            const auto angles =
                getTorusAngles(layout, width, height, x + 0.5F, y + 0.5F);
            const Vector3 flatCenter = {
                .x = (x + 0.5F - (static_cast<float>(width) / 2.0F)) *
                     tileWorldSize,
                .y = 0.02F,
                .z = (y + 0.5F - (static_cast<float>(height) / 2.0F)) *
                     tileWorldSize};
            const Vector3 torusCenter = torusPoint(
                angles[0], angles[1], layout.majorRadius, layout.minorRadius);
            const Vector3 torusNormalValue = torusNormal(angles[0], angles[1]);
            const Vector3 torusMajorTangent =
                Vector3Normalize({.x = -std::sin(angles[0]),
                                  .y = 0.0F,
                                  .z = std::cos(angles[0])});
            const Vector3 torusMinorTangent = Vector3Normalize(
                Vector3CrossProduct(torusMajorTangent, torusNormalValue));
            const Vector3 torusRight =
                layout.yIsMajorAxis ? torusMinorTangent : torusMajorTangent;
            const Vector3 torusForward =
                layout.yIsMajorAxis ? torusMajorTangent : torusMinorTangent;
            TileSurface surface{};

            surface.center = morphedVertex(flatCenter, torusCenter, morph);
            surface.normal =
                morphedNormal(flatWorldNormal, torusNormalValue, morph);
            surface.right =
                Vector3Normalize(lerp(Vector3{.x = 1.0F, .y = 0.0F, .z = 0.0F},
                                      torusRight, smoothStep(morph)));
            surface.forward =
                Vector3Normalize(lerp(Vector3{.x = 0.0F, .y = 0.0F, .z = 1.0F},
                                      torusForward, smoothStep(morph)));
            return surface;
        }

        [[nodiscard]] TileSurface getTileSurface(const int x, const int y,
                                                 const int width,
                                                 const int height,
                                                 const float morph)
        {
            return getTileSurfaceAt(static_cast<float>(x),
                                    static_cast<float>(y), width, height,
                                    morph);
        }

        [[nodiscard]] Vector3 offsetOnSurface(const TileSurface &surface,
                                              const float right,
                                              const float forward,
                                              const float height)
        {
            return Vector3Add(
                Vector3Add(surface.center, Vector3Scale(surface.right, right)),
                Vector3Add(Vector3Scale(surface.forward, forward),
                           Vector3Scale(surface.normal, height)));
        }

        [[nodiscard]] int wrapCoordinate(const int value, const int limit)
        {
            const int wrapped = value % limit;

            if (wrapped < 0) {
                return wrapped + limit;
            }
            return wrapped;
        }

        [[nodiscard]] float closestWrappedCoordinate(const float current,
                                                     const int target,
                                                     const int limit)
        {
            const float targetValue = static_cast<float>(target);
            const float wrappedAbove = targetValue + static_cast<float>(limit);
            const float wrappedBelow = targetValue - static_cast<float>(limit);
            float closest = targetValue;

            if (std::abs(wrappedAbove - current) <
                std::abs(closest - current)) {
                closest = wrappedAbove;
            }
            if (std::abs(wrappedBelow - current) <
                std::abs(closest - current)) {
                closest = wrappedBelow;
            }
            return closest;
        }

        [[nodiscard]] float playerYawForOrientation(const int orientation)
        {
            switch (orientation) {
            case 1:
                return pi;
            case 2:
                return pi * 0.5F;
            case 4:
                return -pi * 0.5F;
            case 3:
            default:
                return 0.0F;
            }
        }

        [[nodiscard]] float shortestAngleDelta(const float from, const float to)
        {
            return std::atan2(std::sin(to - from), std::cos(to - from));
        }

        [[nodiscard]] float smoothAngleTowards(const float current,
                                               const float target,
                                               const float deltaTime)
        {
            const float amount = 1.0F - std::exp(-playerTurnSpeed * deltaTime);

            return current + (shortestAngleDelta(current, target) * amount);
        }

        [[nodiscard]] Vector3 playerForwardOnSurface(const TileSurface &surface,
                                                     const float yaw)
        {
            return Vector3Normalize(
                Vector3Add(Vector3Scale(surface.forward, std::cos(yaw)),
                           Vector3Scale(surface.right, std::sin(yaw))));
        }

        [[nodiscard]] Matrix buildPlayerTransform(const TileSurface &surface,
                                                  const Vector3 position,
                                                  const float yaw)
        {
            const Vector3 up = surface.normal;
            const Vector3 forward = playerForwardOnSurface(surface, yaw);
            const Vector3 right =
                Vector3Normalize(Vector3CrossProduct(up, forward));

            return {
                .m0 = right.x * playerModelScale,
                .m4 = up.x * playerModelScale,
                .m8 = forward.x * playerModelScale,
                .m12 = position.x,
                .m1 = right.y * playerModelScale,
                .m5 = up.y * playerModelScale,
                .m9 = forward.y * playerModelScale,
                .m13 = position.y,
                .m2 = right.z * playerModelScale,
                .m6 = up.z * playerModelScale,
                .m10 = forward.z * playerModelScale,
                .m14 = position.z,
                .m3 = 0.0F,
                .m7 = 0.0F,
                .m11 = 0.0F,
                .m15 = 1.0F,
            };
        }

        void drawModelWithTransform(const raylib::Model &model,
                                    const Matrix transform)
        {
            const Matrix modelTransform =
                MatrixMultiply(model.transform, transform);

            for (int meshIndex = 0; meshIndex < model.meshCount; ++meshIndex) {
                raylib::MeshUnmanaged(model.meshes[meshIndex])
                    .Draw(model.materials[model.meshMaterial[meshIndex]],
                          modelTransform);
            }
        }

        [[nodiscard]] int findAnimationIndex(
            const std::vector<raylib::ModelAnimation> &animations,
            const std::string_view name)
        {
            for (std::size_t index = 0; index < animations.size(); ++index) {
                if (std::string_view(animations.at(index).name) == name) {
                    return static_cast<int>(index);
                }
            }
            return -1;
        }

        [[nodiscard]] float getAnimationDuration(
            const std::vector<raylib::ModelAnimation> &animations,
            const int animationIndex)
        {
            if (animationIndex < 0 ||
                static_cast<std::size_t>(animationIndex) >= animations.size()) {
                return playerMoveDuration;
            }
            return static_cast<float>(std::max(
                       animations.at(static_cast<std::size_t>(animationIndex))
                           .GetFrameCount(),
                       1)) /
                   playerAnimationFrameRate;
        }

        [[nodiscard]] std::string truncateText(const std::string &text,
                                               const std::size_t maximumLength)
        {
            if (text.size() <= maximumLength) {
                return text;
            }
            return text.substr(0, maximumLength - 3) + "...";
        }

        [[nodiscard]] bool isPlayerIncanting(const GameState &state,
                                             const int playerId)
        {
            return std::any_of(
                state.incantations.begin(), state.incantations.end(),
                [playerId](const Incantation &incantation) {
                    return std::find(incantation.playerIds.begin(),
                                     incantation.playerIds.end(),
                                     playerId) != incantation.playerIds.end();
                });
        }

        void drawEvolutionHalo(const Vector3 position)
        {
            rlSetBlendMode(RL_BLEND_ADDITIVE);
            raylib::Vector3(position).DrawSphere(
                playerEvolutionHaloOuterRadius, playerEvolutionHaloRings,
                playerEvolutionHaloSlices, playerEvolutionHaloOuterColor);
            raylib::Vector3(position).DrawSphere(
                playerEvolutionHaloInnerRadius, playerEvolutionHaloRings,
                playerEvolutionHaloSlices, playerEvolutionHaloInnerColor);
            rlSetBlendMode(RL_BLEND_ALPHA);
        }

        [[nodiscard]] Vector3 rotationAxisForNormal(const Vector3 normal)
        {
            const Vector3 axis = Vector3CrossProduct(cameraUp, normal);

            if (Vector3Length(axis) < 0.001F) {
                if (Vector3DotProduct(cameraUp, normal) < 0.0F) {
                    return {.x = 1.0F, .y = 0.0F, .z = 0.0F};
                }
                return cameraUp;
            }
            return Vector3Normalize(axis);
        }

        [[nodiscard]] float rotationAngleForNormal(const Vector3 normal)
        {
            return Vector3Angle(cameraUp, normal) * RAD2DEG;
        }

        void drawBroadcastWave(const TileSurface &surface,
                               const float remainingTime)
        {
            const float progress =
                1.0F - (remainingTime / broadcastDisplayDuration);
            rlSetBlendMode(RL_BLEND_ADDITIVE);
            rlSetLineWidth(broadcastWaveLineWidth);
            for (int waveIndex = 0; waveIndex < broadcastWaveCount;
                 ++waveIndex) {
                const float waveProgress = std::clamp(
                    progress - (static_cast<float>(waveIndex) * 0.16F), 0.0F,
                    1.0F);
                const float radius =
                    lerpFloat(broadcastWaveMinRadius, broadcastWaveMaxRadius,
                              waveProgress);
                const float phase = (progress * fullTurn * 2.0F) +
                                    static_cast<float>(waveIndex);
                const float alpha =
                    (1.0F - waveProgress) * (1.0F - (progress * 0.35F));
                const Color waveColor = {
                    .r = 221,
                    .g = 236,
                    .b = 247,
                    .a = static_cast<unsigned char>(130.0F * alpha)};

                rlBegin(RL_LINES);
                rlColor4ub(waveColor.r, waveColor.g, waveColor.b, waveColor.a);
                for (int segment = 0; segment < broadcastWaveSegmentCount;
                     ++segment) {
                    const float firstAngle =
                        static_cast<float>(segment) /
                        static_cast<float>(broadcastWaveSegmentCount) *
                        fullTurn;
                    const float secondAngle =
                        static_cast<float>(segment + 1) /
                        static_cast<float>(broadcastWaveSegmentCount) *
                        fullTurn;
                    const auto getWavePoint = [&](const float angle) {
                        const float distortion =
                            std::sin((angle * 5.0F) + phase) *
                            (broadcastWaveDistortion * (1.0F + radius));
                        const float distortedRadius = radius + distortion;

                        return offsetOnSurface(
                            surface, std::cos(angle) * distortedRadius,
                            std::sin(angle) * distortedRadius,
                            broadcastWaveHeight);
                    };
                    const Vector3 first = getWavePoint(firstAngle);
                    const Vector3 second = getWavePoint(secondAngle);

                    rlNormal3f(surface.normal.x, surface.normal.y,
                               surface.normal.z);
                    rlVertex3f(first.x, first.y, first.z);
                    rlNormal3f(surface.normal.x, surface.normal.y,
                               surface.normal.z);
                    rlVertex3f(second.x, second.y, second.z);
                }
                rlEnd();
            }
            rlSetLineWidth(1.0F);
            rlSetBlendMode(RL_BLEND_ALPHA);
        }

        [[nodiscard]] Mesh buildSkyboxMesh()
        {
            const int vertexCount =
                (skyboxRingCount + 1) * (skyboxSliceCount + 1);
            constexpr int indexCount = skyboxRingCount * skyboxSliceCount * 6;
            Mesh mesh{};

            mesh.vertexCount = vertexCount;
            mesh.triangleCount = indexCount / 3;
            mesh.vertices = static_cast<float *>(MemAlloc(
                static_cast<unsigned int>(vertexCount * 3 * sizeof(float))));
            mesh.texcoords = static_cast<float *>(MemAlloc(
                static_cast<unsigned int>(vertexCount * 2 * sizeof(float))));
            mesh.normals = static_cast<float *>(MemAlloc(
                static_cast<unsigned int>(vertexCount * 3 * sizeof(float))));
            mesh.indices = static_cast<unsigned short *>(
                MemAlloc(static_cast<unsigned int>(indexCount *
                                                   sizeof(unsigned short))));

            for (int ring = 0; ring <= skyboxRingCount; ++ring) {
                const float v = static_cast<float>(ring) /
                                static_cast<float>(skyboxRingCount);
                const float phi = v * pi;
                const float sinPhi = std::sin(phi);

                for (int slice = 0; slice <= skyboxSliceCount; ++slice) {
                    const float u = static_cast<float>(slice) /
                                    static_cast<float>(skyboxSliceCount);
                    const float theta = u * fullTurn;
                    const int vertexIndex =
                        (ring * (skyboxSliceCount + 1)) + slice;
                    const int vertexOffset = vertexIndex * 3;
                    const int textureOffset = vertexIndex * 2;
                    const Vector3 position = {.x = sinPhi * std::cos(theta),
                                              .y = std::cos(phi),
                                              .z = sinPhi * std::sin(theta)};

                    mesh.vertices[vertexOffset] = position.x;
                    mesh.vertices[vertexOffset + 1] = position.y;
                    mesh.vertices[vertexOffset + 2] = position.z;
                    mesh.normals[vertexOffset] = -position.x;
                    mesh.normals[vertexOffset + 1] = -position.y;
                    mesh.normals[vertexOffset + 2] = -position.z;
                    mesh.texcoords[textureOffset] = 1.0F - u;
                    mesh.texcoords[textureOffset + 1] = v;
                }
            }

            int indexOffset = 0;
            for (int ring = 0; ring < skyboxRingCount; ++ring) {
                for (int slice = 0; slice < skyboxSliceCount; ++slice) {
                    const auto topLeft = static_cast<unsigned short>(
                        (ring * (skyboxSliceCount + 1)) + slice);
                    const auto topRight =
                        static_cast<unsigned short>(topLeft + 1);
                    const auto bottomLeft = static_cast<unsigned short>(
                        ((ring + 1) * (skyboxSliceCount + 1)) + slice);
                    const auto bottomRight =
                        static_cast<unsigned short>(bottomLeft + 1);

                    mesh.indices[indexOffset++] = topLeft;
                    mesh.indices[indexOffset++] = bottomRight;
                    mesh.indices[indexOffset++] = bottomLeft;
                    mesh.indices[indexOffset++] = topLeft;
                    mesh.indices[indexOffset++] = topRight;
                    mesh.indices[indexOffset++] = bottomRight;
                }
            }
            UploadMesh(&mesh, false);
            return mesh;
        }

        void beginQuad(const Color tint)
        {
            rlBegin(RL_QUADS);
            rlColor4ub(tint.r, tint.g, tint.b, tint.a);
        }

        void emitTexturedVertex(const Vector3 &vertex, const Vector3 &normal,
                                const Vector2 textureCoord)
        {
            rlNormal3f(normal.x, normal.y, normal.z);
            rlTexCoord2f(textureCoord.x, textureCoord.y);
            rlVertex3f(vertex.x, vertex.y, vertex.z);
        }

        void drawTexturedFace(const Texture2D &texture,
                              const std::array<Vector3, 4> &vertices,
                              const std::array<Vector3, 4> &normals,
                              const Color tint, const bool reversed)
        {
            constexpr std::array textureCoords = {
                Vector2{.x = 0.0F, .y = 1.0F},
                Vector2{.x = 1.0F, .y = 1.0F},
                Vector2{.x = 1.0F, .y = 0.0F},
                Vector2{.x = 0.0F, .y = 0.0F},
            };
            constexpr std::array<std::size_t, 4> normalOrder = {0, 1, 2, 3};
            constexpr std::array<std::size_t, 4> reversedOrder = {0, 3, 2, 1};
            const auto &drawOrder = reversed ? reversedOrder : normalOrder;

            rlSetTexture(texture.id);
            beginQuad(tint);
            for (std::size_t index : drawOrder) {
                emitTexturedVertex(vertices[index], normals[index],
                                   textureCoords[index]);
            }
            rlEnd();
            rlSetTexture(0);
        }

        [[nodiscard]] std::filesystem::path assetsPath()
        {
            return std::filesystem::path(ZAPPY_ASSETS_DIR);
        }
    } // namespace
} // namespace gui

#endif /* WORLDRENDERER_INTERNALS_HPP */
