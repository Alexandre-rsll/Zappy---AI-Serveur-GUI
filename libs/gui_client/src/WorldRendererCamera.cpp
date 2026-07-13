/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** WorldRenderer
*/

#include "WorldRendererInternals.hpp"

namespace gui {

    void WorldRenderer::_updateCamera(const int width, const int height,
                                      const float deltaTime)
    {
        const float morph = smoothStep(_donutFactor);
        const float flatSpan = getFlatWorldSpan(width, height);
        const float donutSpan = getDonutWorldSpan(width, height);
        const float worldSpan = lerpFloat(flatSpan, donutSpan, morph);
        const float minDistance =
            std::max(minCameraDistance,
                     lerpFloat(worldSpan * flatMinDistanceFactor,
                               worldSpan * donutMinDistanceFactor, morph));
        const float maxDistance =
            std::max(minMaxCameraDistance, worldSpan * maxDistanceFactor);
        const float keyboardSpeed = keyboardOrbitSpeed * deltaTime;
        const float zoomStep = worldSpan * zoomStepFactor;

        if (_orbitDistance <= 0.0F) {
            _orbitDistance = worldSpan * initialDistanceFactor;
        }
        if (raylib::Mouse::IsButtonDown(MOUSE_BUTTON_RIGHT)) {
            const Vector2 delta = raylib::Mouse::GetDelta();

            _orbitYaw -= delta.x * mouseOrbitSensitivity;
            _orbitPitch += delta.y * mouseOrbitSensitivity;
        }
        if (raylib::Keyboard::IsKeyDown(KEY_LEFT)) {
            _orbitYaw += keyboardSpeed;
        }
        if (raylib::Keyboard::IsKeyDown(KEY_RIGHT)) {
            _orbitYaw -= keyboardSpeed;
        }
        if (raylib::Keyboard::IsKeyDown(KEY_UP)) {
            _orbitPitch += keyboardSpeed;
        }
        if (raylib::Keyboard::IsKeyDown(KEY_DOWN)) {
            _orbitPitch -= keyboardSpeed;
        }
        if (!_isMouseOverLogInterface()) {
            _orbitDistance -= raylib::Mouse::GetWheelMove() * zoomStep;
        }
        _orbitPitch = std::clamp(_orbitPitch, minOrbitPitch, maxOrbitPitch);
        _orbitDistance = std::clamp(_orbitDistance, minDistance, maxDistance);

        _camera.target = {0.0F, morph * cameraTargetYOffset, 0.0F};
        _camera.position = {
            _camera.target.x +
                _orbitDistance * std::cos(_orbitPitch) * std::sin(_orbitYaw),
            _camera.target.y + _orbitDistance * std::sin(_orbitPitch),
            _camera.target.z +
                _orbitDistance * std::cos(_orbitPitch) * std::cos(_orbitYaw),
        };
        _camera.fovy = cameraFovy;
    }

    void WorldRenderer::_updateTransition(const float deltaTime)
    {
        if (raylib::Keyboard::IsKeyPressed(KEY_T)) {
            _donutMode = !_donutMode;
        }

        const float target = _donutMode ? 1.0F : 0.0F;
        const float step = transitionSpeed * deltaTime;

        if (_donutFactor < target) {
            _donutFactor = std::min(_donutFactor + step, target);
        } else if (_donutFactor > target) {
            _donutFactor = std::max(_donutFactor - step, target);
        }
    }

} // namespace gui
