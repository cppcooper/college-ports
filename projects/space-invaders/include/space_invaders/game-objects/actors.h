#pragma once
#include <cheryl/core/resources/asset-management/sprite-mgr.h>
#include <cheryl/core/game-framework/tick-context.h>
#include <cheryl/core/display/window-interface.h>
#include <cheryl/core/rendering/render-frame.h>
#include <cheryl/core/rendering/draw-packet.h>
#include <cheryl/core/engine/engine-context.h>
#include <cheryl/assets/definitions/grid.h>
#include <cheryl/assets/submission/draw2d.h>
#include <cheryl/assets/types/2d/sprite.h>


#include <algorithm>
#include <chrono>
#include <memory>
#include <utility>

#include <glm/gtc/matrix_transform.hpp>

#include <space_invaders/game-objects/abstract-game-object.h>
#include <space_invaders/events.h>

#define PLAYER_Y 100.f

namespace CEAsset {
    using Sprite = CE::Assets::Sprites::Asset;
    using CellIndex = CE::Assets::Grids::CellIndex;
    struct Appearance {
        std::shared_ptr<const Sprite> sprite;
        CellIndex cell{};
    };
}

struct ActorBase : AbstractGameObject {
    CE::Events::EventBus::Publisher event_publisher;
    std::shared_ptr<const CEAsset::Sprite> sprite;
    glm::vec2 position{};
    double vertical = -200.0;
    double scale = 0.45;
    int screen_width = 1;
    bool is_dead = false;
    bool is_shot = false;

    ~ActorBase() override = default;
    ActorBase() = default;
    ActorBase(std::shared_ptr<const CEAsset::Sprite> resource, const glm::vec2 initial_position)
    : sprite(std::move(resource)), position(initial_position) {}

    void init_platform(CE::Runtime::Context& engine) override {
        screen_width = engine.window().framebuffer_size().width;
        event_publisher = engine.events().publisher();
    }

    virtual void fire() {
        const double half_height = sprite->definition().grid.frame.height * scale * 0.5;
        auto origin = position;
        origin.y += static_cast<float>(vertical > 0.0 ? half_height : -half_height);

        event_publisher.notify(
            SpaceInvadersEvents::shot_requested_event,
            SpaceInvadersEvents::ShotRequested{
                .position = origin,
                .speed = vertical
            }
        );
    }

    [[nodiscard]] virtual CEAsset::Appearance appearance() const = 0;

    void move(const double dt, const double horizontal) {
        const auto width = static_cast<float>(screen_width);
        const double half_actor = sprite->definition().grid.frame.width * scale * 0.5;
        const double margin = std::min(half_actor, width * 0.5);
        position.x = static_cast<float>(std::clamp(position.x + horizontal * dt, margin, width - margin));
    };

    void append(CE::Rendering::PassWriter& pass) const override {
        const CE::Rendering::Submission::Context2D context{
            pass.semantics(), pass.parameters(), pass.constraints()
        };
        auto A = appearance();
        const CE::Rendering::DrawStyle2D image_style {
            .material = A.sprite->material,
            .model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f)),
            .scale = static_cast<float>(scale)
        };
        pass.add(CE::Rendering::Submission::resolve_sprite(*A.sprite, A.cell, image_style, context));
    }
};

class PlayerActor : public ActorBase {
public:
    PlayerActor()
    : ActorBase(CE::Assets::Sprites::Manager::get().get_asset("invaders", "player"), {0,0}) {
        vertical = 600;
    }

    [[nodiscard]] CEAsset::Appearance appearance() const override {
        if (is_shot) [[unlikely]] {
            return {.sprite = sprite,  .cell = 1};
        }
        return {.sprite = sprite, .cell = 0};
    }
};

enum class EnemyType {
    MALOLRIE, //enemy_1
    ARCHER,   //enemy_2
    LANA,     //enemy_3
    CHERYL,   //enemy_4
    CYRIL,    //enemy_5
    RAY,      //enemy_6
    KRIEGER,  //enemy_7
    WOODIE    //enemy_8
};

inline auto GetEnemySprite(const EnemyType type) {
    switch (type) {
        case EnemyType::MALOLRIE: // front line
            return CE::Assets::Sprites::Manager::get().get_asset("invaders", "enemy_1");
        case EnemyType::ARCHER:
            return CE::Assets::Sprites::Manager::get().get_asset("invaders", "enemy_2");
        case EnemyType::LANA:
            return CE::Assets::Sprites::Manager::get().get_asset("invaders", "enemy_3");
        case EnemyType::CHERYL:
            return CE::Assets::Sprites::Manager::get().get_asset("invaders", "enemy_4");
        case EnemyType::CYRIL:
            return CE::Assets::Sprites::Manager::get().get_asset("invaders", "enemy_5");
        case EnemyType::RAY:
            return CE::Assets::Sprites::Manager::get().get_asset("invaders", "enemy_6");
        case EnemyType::KRIEGER: // back line
            return CE::Assets::Sprites::Manager::get().get_asset("invaders", "enemy_7");
        // UFO / saucer
        case EnemyType::WOODIE:
            return CE::Assets::Sprites::Manager::get().get_asset("invaders", "enemy_8");
    }
    return CE::Assets::Sprites::Manager::get().get_asset("invaders", "death");
}

namespace EnemyDetail {
    extern float time_scale;
}

class EnemyActor : public ActorBase {
    int id = -1;
    CE::Assets::Sprites::Animation animation;

public:
    EnemyActor(const EnemyType type, const glm::vec2& position, const int id)
    : ActorBase(GetEnemySprite(type), position), id {id}, animation(sprite->animation("default")) {}

    [[nodiscard]] CEAsset::Appearance appearance() const override {
        if (is_shot) [[unlikely]] {
            return {.sprite = CE::Assets::Sprites::Manager::get().get_asset("invaders", "explosion"), .cell = 0};
        }
        return {.sprite = sprite, .cell = animation.cell()};
    }

    void update(const CE::Game::TickContext& tick) override {
        if (!is_dead) {
            const std::chrono::duration<double> elapsed {EnemyDetail::time_scale * tick.delta_seconds};
            animation.advance(elapsed);
        }
    }

    [[nodiscard]] bool is_alive() const { return !is_dead; }
};
