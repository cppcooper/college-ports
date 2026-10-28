#pragma once

#include <cheryl/assets/types/2d/sprite.h>
#include <cheryl/core/subsystems/event-bus.h>

#include <glm/vec2.hpp>

#include <memory>
#include <utility>

#include <space_invaders/events.h>
#include <space_invaders/game-objects/world-object.h>

struct ActorBase : WorldObject {
    CE::Events::EventBus::Publisher event_publisher;
    double projectile_speed = -200.0;
    bool is_dead = false;
    bool is_shot = false;

    ActorBase(
        std::shared_ptr<const CE::Assets::Sprites::Asset> resource,
        CE::Events::EventBus::Publisher publisher,
        glm::vec2 initial_position = {},
        float initial_scale = 0.45f,
        double projectile_speed = -200.0
    ) : WorldObject(std::move(resource), initial_position, initial_scale),
        event_publisher(std::move(publisher)), projectile_speed(projectile_speed) {}
    ~ActorBase() override = default;

    virtual void move(double delta_seconds, double horizontal_speed) {
        translate({static_cast<float>(delta_seconds * horizontal_speed), 0.0f});
    }
    virtual void fire() {
        const auto shape = bounds();
        auto origin = shape.center();
        origin.y = projectile_speed > 0.0 ? shape.maximum.y : shape.minimum.y;
        event_publisher.notify(
            SpaceInvadersEvents::shot_requested_event,
            SpaceInvadersEvents::ShotRequested{.position = origin, .speed = projectile_speed}
        );
    }
    virtual void hit() = 0;
    [[nodiscard]] bool is_alive() const noexcept { return !is_dead; }
};
