#pragma once

#include <cheryl/assets/types/2d/sprite.h>
#include <cheryl/core/game-framework/tick-context.h>
#include <cheryl/core/resources/asset-management/sprite-mgr.h>

#include <glm/vec2.hpp>

#include <algorithm>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <utility>

#include <space_invaders/events.h>
#include <space_invaders/game-objects/actors/actor-base.h>
#include <space_invaders/game-objects/actors/enemy-type.h>

class EnemyActor final : public ActorBase {
    CE::Assets::Sprites::Animation animation;
    static constexpr double explosion_duration_seconds = 1.0;
    double explosion_elapsed = explosion_duration_seconds;
    glm::vec2 explosion_position{};

public:
    const EnemyType type;
    const int id;
    double animation_speed = 1.0;

    EnemyActor(const EnemyType type, const glm::vec2 position, const int id, CE::Events::EventBus::Publisher publisher)
    : ActorBase(GetEnemySprite(type), std::move(publisher), position),
      animation(sprite->animation("default")), type(type), id(id) {}
    [[nodiscard]] bool is_exploding() const noexcept {
        return is_shot && explosion_elapsed < explosion_duration_seconds;
    }
    [[nodiscard]] Appearance appearance() const override {
        if (is_shot) {
            return {
                CE::Assets::Sprites::Manager::get().get_asset("invaders", "explosion"), 0,
                static_cast<float>(1.0 - explosion_elapsed / explosion_duration_seconds)
            };
        }
        return {sprite, animation.cell()};
    }
    void hit() override {
        if (is_shot || is_dead) return;
        explosion_position = position;
        explosion_elapsed = 0.0;
        is_shot = true;
        event_publisher.notify(
            SpaceInvadersEvents::enemy_killed_event,
            SpaceInvadersEvents::EnemyKilled{ static_cast<int>(animation_speed * points_for(type))}
        );
    }
    void update(const CE::Game::TickContext& tick) override {
        ActorBase::update(tick);
        if (is_alive()) {
            animation.advance(std::chrono::duration<double>{animation_speed * tick.delta_seconds});
        } else {
            explosion_elapsed = std::clamp(explosion_elapsed + tick.delta_seconds, 0.0, explosion_duration_seconds);
        }
    }
    void append_draws(CE::Rendering::PassWriter& pass) const override {
        if (is_alive()) {
            WorldObject::append_draws(pass);
        } else if (explosion_elapsed < explosion_duration_seconds) {
            append_sprite(pass, appearance(), explosion_position);
        }
    }
};
