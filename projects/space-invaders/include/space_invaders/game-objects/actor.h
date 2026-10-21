#pragma once
#include "abstract-game-object.h"
#include <glm/gtc/matrix_transform.hpp>

namespace CEAsset {
    using Sprite = CE::Assets::Sprites::Asset;
}

struct ActorBase : AbstractGameObject {
    std::shared_ptr<const CEAsset::Sprite> sprite;
    CE::Assets::Grids::CellIndex cell = 0;
    glm::vec2 position{};
    float scale = 1.f;
    int screen_width = 1;

    ~ActorBase() override = default;
    ActorBase() = default;
    ActorBase(std::shared_ptr<const CEAsset::Sprite> resource, const glm::vec2 initial_position)
    : sprite(std::move(resource)), position(initial_position) {}

    void init_platform(CE::Runtime::Context& engine) override {
        screen_width = engine.window().framebuffer_size().width;
    }

    void move(const double dt, const float horizontal) {
        const auto width = static_cast<float>(screen_width);
        const float half_actor = static_cast<float>(sprite->definition().grid.frame.width) * scale * 0.5f;
        const float margin = std::min(half_actor, width * 0.5f);
        position.x = std::clamp(position.x + horizontal * 160.0f * static_cast<float>(dt), margin, width - margin);
    };

    void append(CE::Rendering::PassWriter& pass) const {
        const CE::Rendering::Submission::Context2D context{
            pass.semantics(), pass.parameters(), pass.constraints()
        };
        const CE::Rendering::DrawStyle2D image_style {
            .material = sprite->material,
            .model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f)),
            .scale = scale
        };
        pass.add(CE::Rendering::Submission::resolve_sprite(*sprite, cell, image_style, context));
    }
};

class PlayerActor : public ActorBase {
public:
    PlayerActor()
    : ActorBase(CE::Assets::Sprites::Manager::get().get_asset("invaders", "player"), {0,0}) {}
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
        case EnemyType::MALOLRIE:
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
        case EnemyType::KRIEGER:
            return CE::Assets::Sprites::Manager::get().get_asset("invaders", "enemy_7");
        case EnemyType::WOODIE:
            return CE::Assets::Sprites::Manager::get().get_asset("invaders", "enemy_8");
    }
    return CE::Assets::Sprites::Manager::get().get_asset("invaders", "death");
}

namespace EnemyDetail {
    extern float time_scale;
}

class EnemyActor : public ActorBase {
public:
    EnemyActor(const EnemyType type, const glm::vec2& position)
    : ActorBase(GetEnemySprite(type), position) {}

    void update(const CE::Game::TickContext& tick) override {
        if (sprite->has_animation("default")) {
            const std::chrono::duration<double> elapsed {tick.delta_seconds * EnemyDetail::time_scale};
            sprite->animation("default").advance(elapsed);
        }
    }
};
