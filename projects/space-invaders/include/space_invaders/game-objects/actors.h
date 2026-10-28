#pragma once

#include <cheryl/assets/types/2d/sprite.h>
#include <cheryl/core/game-framework/tick-context.h>
#include <cheryl/core/resources/asset-management/sprite-mgr.h>
#include <cheryl/core/subsystems/event-bus.h>

#include <algorithm>
#include <chrono>
#include <stdexcept>
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

class PlayerActor final : public ActorBase {
    float screen_width;

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
        is_shot = true;
        // Reserve lives and respawning remain game-level work.
    }
    void update(const CE::Game::TickContext&) override {
        // The player currently has no timed animation or other autonomous state.
    }
};

enum class EnemyType {
    MALOLRIE,
    ARCHER,
    LANA,
    CHERYL,
    CYRIL,
    RAY,
    KRIEGER,
    WOODIE
};

[[nodiscard]] std::shared_ptr<const CE::Assets::Sprites::Asset> GetEnemySprite(EnemyType type);

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
        if (!is_alive()) return;
        explosion_position = position;
        explosion_elapsed = 0.0;
        is_shot = true;
        is_dead = true;
    }
    void update(const CE::Game::TickContext& tick) override {
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

inline std::shared_ptr<const CE::Assets::Sprites::Asset> GetEnemySprite(const EnemyType type) {
    auto& assets = CE::Assets::Sprites::Manager::get();
    switch (type) {
        case EnemyType::MALOLRIE: return assets.get_asset("invaders", "enemy_1");
        case EnemyType::ARCHER: return assets.get_asset("invaders", "enemy_2");
        case EnemyType::LANA: return assets.get_asset("invaders", "enemy_3");
        case EnemyType::CHERYL: return assets.get_asset("invaders", "enemy_4");
        case EnemyType::CYRIL: return assets.get_asset("invaders", "enemy_5");
        case EnemyType::RAY: return assets.get_asset("invaders", "enemy_6");
        case EnemyType::KRIEGER: return assets.get_asset("invaders", "enemy_7");
        case EnemyType::WOODIE: return assets.get_asset("invaders", "enemy_8");
    }
    throw std::invalid_argument("Unknown enemy type");
}
