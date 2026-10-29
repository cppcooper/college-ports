#pragma once

#include <cheryl/core/game-framework/tick-context.h>
#include <cheryl/core/resources/asset-management/sprite-mgr.h>

#include <glm/vec2.hpp>

#include <algorithm>
#include <utility>

#include <space_invaders/constants.h>
#include <space_invaders/game-objects/actors/actor-base.h>

class PlayerActor final : public ActorBase {
    float screen_width;
    static constexpr double death_frame_duration_seconds = SpaceInvadersConfig::player_respawn_delay_seconds;
    double death_frame_elapsed = death_frame_duration_seconds;

public:
    static constexpr float spawn_height = 100.0f;

    PlayerActor(CE::Events::EventBus::Publisher publisher, const glm::vec2 initial_position, const float screen_width)
    : ActorBase(CE::Assets::Sprites::Manager::get().get_asset("invaders", "player"),
                std::move(publisher), initial_position, 0.45f, 600.0),
      screen_width(screen_width) {}
    [[nodiscard]] Appearance appearance() const override { return {sprite, is_shot ? 1u : 0u}; }
    void move(double delta_seconds, double horizontal_speed) override {
        ActorBase::move(delta_seconds, horizontal_speed);
        const auto shape = bounds();
        position.x += shape.size().x >= screen_width
            ? screen_width * 0.5f - shape.center().x
            : std::clamp(0.0f, -shape.minimum.x, screen_width - shape.maximum.x);
    }
    void hit() override {
        if (is_shot || is_dead) return;
        death_frame_elapsed = 0.0;
        is_shot = true;
        event_publisher.notify(
            SpaceInvadersEvents::player_hit_event,
            SpaceInvadersEvents::PlayerHit{}
        );
    }
    void respawn() {
        is_shot = false;
        is_dead = false;
        death_frame_elapsed = death_frame_duration_seconds;
    }
    void update(const CE::Game::TickContext& tick) override {
        ActorBase::update(tick);
        if (!is_shot || death_frame_elapsed >= death_frame_duration_seconds) return;
        death_frame_elapsed = std::clamp(death_frame_elapsed + tick.delta_seconds, 0.0, death_frame_duration_seconds);
        if (death_frame_elapsed >= death_frame_duration_seconds) {
            event_publisher.notify(
                SpaceInvadersEvents::player_death_animation_finished_event,
                SpaceInvadersEvents::PlayerDeathAnimationFinished{}
            );
        }
    }
};
