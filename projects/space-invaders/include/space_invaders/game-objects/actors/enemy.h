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
#include <space_invaders/game-speed.h>
#include <space_invaders/game-objects/actors/actor-base.h>
#include <space_invaders/game-objects/actors/enemy-type.h>

class EnemyActor final : public ActorBase {
    CE::Assets::Sprites::Animation animation;
    std::shared_ptr<const CE::Rendering::Resources::Material> material;
    static constexpr double explosion_duration_seconds = 1.0;
    double explosion_elapsed = explosion_duration_seconds;
    glm::vec2 explosion_position{};

public:
    const EnemyType type;
    const int id;

    EnemyActor(
        const EnemyType type, const glm::vec2 position, const int id, CE::Events::EventBus::Publisher publisher,
        std::shared_ptr<const CE::Rendering::Resources::Material> material = {}
    )
    : ActorBase(GetEnemySprite(type), std::move(publisher), position),
      animation(sprite->animation("default")), material(std::move(material)), type(type), id(id) {}
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
        const float redness = type == EnemyType::WOODIE ? 1.0f : static_cast<float>(game_speed_fraction());
        return {sprite, animation.cell(), 1.0f, material, {1.0f, 1.0f - redness, 1.0f - redness}};
    }
    void hit() override {
        if (is_shot || is_dead) return;
        explosion_position = position;
        explosion_elapsed = 0.0;
        is_shot = true;
        event_publisher.notify(
            SpaceInvadersEvents::enemy_killed_event,
            SpaceInvadersEvents::EnemyKilled{static_cast<int>(game_speed * points_for(type))}
        );
    }
    void update(const CE::Game::TickContext& tick) override {
        ActorBase::update(tick);
        if (is_alive()) {
            animation.advance(std::chrono::duration<double>{game_speed * tick.delta_seconds});
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
