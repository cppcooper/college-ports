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

class PlayerActor;
class PlayerController;
class Invasion;

class BulletManager final : public AbstractGameObject {
    std::vector<std::unique_ptr<Bullet>> bullets;
    CE::Events::EventBus::Registration shot_listener;
    CE::Events::EventBus* events = nullptr;
    std::function<void(int)> count_changed;
    glm::vec2 world_size{1, 1};
    bool clear_requested = false;

    void report_count() const {
        if (count_changed) count_changed(static_cast<int>(bullets.size()));
    }
    void clear_bullets() {
        bullets.clear();
        clear_requested = false;
        report_count();
    }
protected:
    void unregister_listener() const {
        if (!shot_listener.disconnect()) {
            // todo: throw exception
        }
    }

public:
    explicit BulletManager(PlayerController& pc, std::function<void(int)> observer = {})
    : count_changed(std::move(observer)) {}

    ~BulletManager() override { unregister_listener(); }

    [[nodiscard]] std::size_t count() const noexcept { return bullets.size(); }
    // PlayerHit can fire during collision iteration; clear after that pass finishes.
    void onPlayerHit() { clear_requested = true; }
    void init_platform(CE::Runtime::Context& engine) override {
        events = &engine.events();
        const auto size = engine.window().framebuffer_size();
        world_size = {size.width, size.height};
    }
    void init_simulation() override;
    void append_draws(CE::Rendering::PassWriter& pass) const;
    void update(const CE::Game::TickContext& tick) override;
    void detect_collisions(PlayerActor* p, Invasion* i);

    void deinit_simulation() override {
        unregister_listener();
        shot_listener = {};
        clear_bullets();
    }
    void deinit_platform() override {
        unregister_listener();
        events = nullptr;
    }
};
