/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** WorldRenderer
*/

#include "WorldRendererInternals.hpp"

namespace gui {

    void WorldRenderer::_drawPlayers(const GameState &state, const int width,
                                     const int height, const float deltaTime)
    {
        const float morph = smoothStep(_donutFactor);

        if (!_playerModels.front().has_value() || width <= 0 || height <= 0) {
            return;
        }
        for (auto iterator = _playerStates.begin();
             iterator != _playerStates.end();) {
            if (!state.players.contains(iterator->first)) {
                iterator = _playerStates.erase(iterator);
            } else {
                ++iterator;
            }
        }
        for (const auto &[id, player] : state.players) {
            const int x = wrapCoordinate(player.x, width);
            const int y = wrapCoordinate(player.y, height);
            const float targetYaw = playerYawForOrientation(player.orientation);
            const bool isEvolving = isPlayerIncanting(state, id);
            const std::size_t playerModelIndex = static_cast<std::size_t>(
                std::clamp(player.level, 1,
                           static_cast<int>(playerLevelCount)) -
                1);
            raylib::Model &playerModel = *_playerModels.at(playerModelIndex);
            auto [stateIterator, inserted] =
                _playerStates.emplace(id, PlayerRenderState{});
            PlayerRenderState &renderState = stateIterator->second;

            if (inserted) {
                renderState.yaw = targetYaw;
                renderState.visualX = static_cast<float>(x);
                renderState.visualY = static_cast<float>(y);
                renderState.startX = renderState.visualX;
                renderState.startY = renderState.visualY;
                renderState.targetX = renderState.visualX;
                renderState.targetY = renderState.visualY;
                renderState.targetTileX = x;
                renderState.targetTileY = y;
                renderState.level = player.level;
                renderState.broadcastVersion = player.broadcastVersion;
                renderState.broadcastMessage = player.lastBroadcast;
            } else {
                if (renderState.level != player.level) {
                    renderState.level = player.level;
                    renderState.turnAnimationTime = playerTurnAnimationDuration;
                }
                renderState.yaw =
                    smoothAngleTowards(renderState.yaw, targetYaw, deltaTime);
                if (renderState.broadcastVersion != player.broadcastVersion) {
                    renderState.broadcastVersion = player.broadcastVersion;
                    renderState.broadcastMessage = player.lastBroadcast;
                    renderState.broadcastElapsed = broadcastDisplayDuration;
                }
            }
            if (renderState.targetTileX != x || renderState.targetTileY != y) {
                const bool useWrappedMovement = morph > 0.5F;

                renderState.startX = renderState.visualX;
                renderState.startY = renderState.visualY;
                renderState.targetX = useWrappedMovement
                                          ? closestWrappedCoordinate(
                                                renderState.visualX, x, width)
                                          : static_cast<float>(x);
                renderState.targetY = useWrappedMovement
                                          ? closestWrappedCoordinate(
                                                renderState.visualY, y, height)
                                          : static_cast<float>(y);
                renderState.moveElapsed = 0.0F;
                renderState.targetTileX = x;
                renderState.targetTileY = y;
                renderState.jumpAnimationTime = getAnimationDuration(
                    _playerAnimations, _playerJumpAnimationIndex);
                renderState.moving = true;
            }
            if (renderState.moving) {
                renderState.moveElapsed = std::min(
                    renderState.moveElapsed + deltaTime, playerMoveDuration);
                const float moveAmount =
                    smoothStep(renderState.moveElapsed / playerMoveDuration);

                renderState.visualX = lerpFloat(
                    renderState.startX, renderState.targetX, moveAmount);
                renderState.visualY = lerpFloat(
                    renderState.startY, renderState.targetY, moveAmount);
                if (renderState.moveElapsed >= playerMoveDuration) {
                    renderState.visualX =
                        static_cast<float>(renderState.targetTileX);
                    renderState.visualY =
                        static_cast<float>(renderState.targetTileY);
                    renderState.moving = false;
                }
            }
            renderState.turnAnimationTime =
                std::max(0.0F, renderState.turnAnimationTime - deltaTime);
            renderState.jumpAnimationTime =
                std::max(0.0F, renderState.jumpAnimationTime - deltaTime);
            renderState.broadcastElapsed =
                std::max(0.0F, renderState.broadcastElapsed - deltaTime);
            const bool isTurning =
                isEvolving || renderState.turnAnimationTime > 0.0F ||
                std::abs(shortestAngleDelta(renderState.yaw, targetYaw)) >
                    playerTurnAnimationThreshold;
            const bool isJumping =
                renderState.moving || renderState.jumpAnimationTime > 0.0F;
            const PlayerAnimation nextAnimation =
                isJumping ? PlayerAnimation::Jump
                          : (isTurning ? PlayerAnimation::Turn
                                       : PlayerAnimation::Idle);

            if (renderState.animation != nextAnimation) {
                renderState.animation = nextAnimation;
                renderState.animationFrame = 0.0F;
            } else {
                renderState.animationFrame +=
                    deltaTime * playerAnimationFrameRate;
            }
            const TileSurface surface = getTileSurfaceAt(
                renderState.visualX, renderState.visualY, width, height, morph);
            const Vector3 position =
                offsetOnSurface(surface, 0.0F, 0.0F, playerSurfaceOffset);

            if (isEvolving) {
                drawEvolutionHalo(position);
            }
            if (renderState.broadcastElapsed > 0.0F) {
                drawBroadcastWave(surface, renderState.broadcastElapsed);
            }

            int animationIndex = _playerIdleAnimationIndex;

            switch (renderState.animation) {
            case PlayerAnimation::Jump:
                animationIndex = _playerJumpAnimationIndex >= 0
                                     ? _playerJumpAnimationIndex
                                     : _playerIdleAnimationIndex;
                break;
            case PlayerAnimation::Turn:
                animationIndex = _playerTurnAnimationIndex >= 0
                                     ? _playerTurnAnimationIndex
                                     : _playerIdleAnimationIndex;
                break;
            case PlayerAnimation::Idle:
            default:
                break;
            }
            if (!_playerAnimations.empty() && animationIndex >= 0) {
                const raylib::ModelAnimation &animation = _playerAnimations.at(
                    static_cast<std::size_t>(animationIndex));
                const int frame = static_cast<int>(renderState.animationFrame) %
                                  std::max(animation.GetFrameCount(), 1);

                playerModel.UpdateAnimation(animation, frame);
            }
            drawModelWithTransform(
                playerModel,
                buildPlayerTransform(surface, position,
                                     renderState.yaw + playerModelYawOffset +
                                         (playerTorusYawOffset * morph)));
        }
    }

    void WorldRenderer::_drawBroadcastMessages(const int width,
                                               const int height) const
    {
        const float morph = smoothStep(_donutFactor);

        for (const auto &[id, renderState] : _playerStates) {
            if (renderState.broadcastElapsed <= 0.0F ||
                renderState.broadcastMessage.empty()) {
                continue;
            }
            const TileSurface surface = getTileSurfaceAt(
                renderState.visualX, renderState.visualY, width, height, morph);
            const Vector3 labelPosition = offsetOnSurface(
                surface, 0.0F, 0.0F, playerBroadcastLabelOffset);
            const Vector2 screenPosition =
                _camera.GetWorldToScreen(labelPosition);
            const std::string message =
                "#" + std::to_string(id) + ": " +
                truncateText(renderState.broadcastMessage,
                             broadcastMessageCharacterLimit);
            const raylib::Text broadcastText(message, broadcastTextSize);
            const int textWidth = broadcastText.Measure();
            const float alpha =
                renderState.broadcastElapsed / broadcastDisplayDuration;
            const Color background = {
                .r = 9,
                .g = 14,
                .b = 18,
                .a = static_cast<unsigned char>(190.0F * alpha)};
            const Color textColor = {
                .r = 255,
                .g = 247,
                .b = 214,
                .a = static_cast<unsigned char>(255.0F * alpha)};
            const int textX =
                static_cast<int>(screenPosition.x) - (textWidth / 2);
            const int textY =
                static_cast<int>(screenPosition.y) - broadcastTextSize;

            raylib::Rectangle(textX - broadcastLabelPadding, textY - 2,
                              textWidth + (broadcastLabelPadding * 2),
                              broadcastTextSize + broadcastLabelPadding)
                .Draw(background);
            raylib::Text::Draw(message, textX, textY, broadcastTextSize,
                               textColor);
        }
    }

} // namespace gui
