#pragma once
#include <algorithm>
#include <cmath>

namespace battlestar {
constexpr float board_width = 1920.f, board_height = 1080.f;
constexpr float ship_width = 92.f, ship_height = 48.f;
constexpr float max_bank_degrees = 24.f;
constexpr int bank_frames = 49;
struct Point { float x, y; };
// Positive angle points down in screen coordinates, matching the sprite rotation.
struct ShipPose {
    float x, y, forward_x, forward_y;
    Point point(float forward, float sideways = 0) const {
        return {x + forward_x * forward - forward_y * sideways,
                y + forward_y * forward + forward_x * sideways};
    }
};
inline ShipPose ship_pose(float x, float y, float degrees) {
    const float radians = degrees * 3.14159265f / 180.f;
    return {x, y, std::cos(radians), std::sin(radians)};
}
struct Input { float x = 0, y = 0; bool boost = false; };
struct Flight {
    float x = 320, y = 540, bank = 0, time = 0;
    bool paused = false;
    void reset() { *this = Flight{}; }
    void update(Input input, float dt) {
        if (paused || !std::isfinite(dt) || dt <= 0) return;
        dt = std::min(dt, .05f);
        float length = std::hypot(input.x, input.y);
        if (length > 1) { input.x /= length; input.y /= length; }
        const float speed = input.boost ? 820.f : 420.f;
        x = std::clamp(x + input.x * speed * dt, 32.f + ship_width / 2, board_width - 32.f - ship_width / 2);
        y = std::clamp(y + input.y * speed * dt, 112.f + ship_height / 2, board_height - 100.f - ship_height / 2);
        bank += (input.y * max_bank_degrees - bank) * (1.f - std::exp(-12.f * dt));
        time += dt;
    }
    float bob() const { return std::sin(time * 4.f) * 3.f; }
};
}
