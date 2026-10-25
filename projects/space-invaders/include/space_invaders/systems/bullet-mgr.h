#pragma once

#include <cheryl/core/display/window-interface.h>
#include <cheryl/core/engine/engine-context.h>
#include <cheryl/core/game-framework/tick-context.h>
#include <cheryl/core/subsystems/event-bus.h>

#include <glm/vec2.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include <space_invaders/events.h>
#include <space_invaders/game-objects/abstract-game-object.h>
#include <space_invaders/game-objects/bullets.h>

class BulletManager final : public AbstractGameObject {
    std::vector<std::unique_ptr<Bullet>> bullets;
    CE::Events::EventBus::Registration shot_listener;
    CE::Events::EventBus* events = nullptr;
    std::function<void(int)> count_changed;
    glm::vec2 world_size{1, 1};

    void report_count() const {
        if (count_changed) count_changed(static_cast<int>(bullets.size()));
    }

public:
    explicit BulletManager(std::function<void(int)> observer = {}) : count_changed(std::move(observer)) {}
    ~BulletManager() override { shot_listener.disconnect(); }

    [[nodiscard]] std::size_t count() const noexcept { return bullets.size(); }
    void init_platform(CE::Runtime::Context& engine) override {
        events = &engine.events();
        const auto size = engine.window().framebuffer_size();
        world_size = {size.width, size.height};
    }
    void init_simulation() override {
        if (!events) {
            throw std::logic_error("Initialize BulletManager platform before simulation");
        }
        shot_listener.disconnect();
        Bullet::preallocate();
        shot_listener = events->register_listener(
            SpaceInvadersEvents::shot_requested_event,
            [this](const SpaceInvadersEvents::ShotRequested& shot) {
                bullets.push_back(std::make_unique<Bullet>(shot.position, shot.speed));
                report_count();
            }
        );
    }
    void deinit_simulation() override {
        shot_listener.disconnect();
        shot_listener = {};
        bullets.clear();
        report_count();
    }
    void deinit_platform() override {
        shot_listener.disconnect();
        events = nullptr;
    }
    void update(const CE::Game::TickContext& tick) override {
        for (const auto& bullet : bullets)
            bullet->update(tick);
        const auto removed = std::erase_if(bullets, [this](const auto& bullet) {
            const auto shape = bullet->bounds();
            return shape.maximum.y < 0.0f || shape.minimum.y > world_size.y;
        });
        if (removed) report_count();
        // Actor collision responses remain unfinished.
    }
    void append_draws(CE::Rendering::PassWriter& pass) const {
        for (const auto& bullet : bullets)
            bullet->append_draws(pass);
    }
};
