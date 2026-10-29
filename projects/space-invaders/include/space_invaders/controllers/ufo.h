#pragma once

#include <cheryl/core/display/window-interface.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <optional>
#include <random>

#include <space_invaders/controllers/player.h>
#include <space_invaders/game-speed.h>

class UFOController final : public ActorController {
    std::default_random_engine random{
        static_cast<std::default_random_engine::result_type>(std::chrono::steady_clock::now().time_since_epoch().count())
    };
    float screen_width = 1.0f;
    std::optional<float> target_x;

public:
    UFOController() {
        horizontal = 160.0;
    }
    void init_platform(CE::Runtime::Context& engine) override {
        screen_width = static_cast<float>(engine.window().framebuffer_size().width);
    }
    void init_simulation() override { target_x.reset(); }
    void deinit_simulation() override {
        detach();
        target_x.reset();
    }
    void update(const CE::Game::TickContext& tick) override {
        if (!actor || !actor->is_alive() || tick.delta_seconds <= 0.0) return;
        const auto shape = actor->bounds();
        const float minimum_x = actor->position.x - shape.minimum.x + 16.0f;
        const float maximum_x = screen_width - (shape.maximum.x - actor->position.x) - 16.0f;
        if (minimum_x >= maximum_x) {
            actor->position.x = screen_width * 0.5f;
            target_x.reset();
            return;
        }
        if (!target_x || std::abs(*target_x - actor->position.x) <= 1.0f) {
            target_x = std::uniform_real_distribution<float>{minimum_x, maximum_x}(random);
        }
        const double distance = *target_x - actor->position.x;
        const double speed = std::min(horizontal * game_speed, std::abs(distance) / tick.delta_seconds);
        actor->move(tick.delta_seconds, std::copysign(speed, distance));
    }
};
