#pragma once

#include <cheryl/assets/definitions/grid.h>
#include <cheryl/assets/submission/draw2d.h>
#include <cheryl/assets/types/2d/sprite.h>
#include <cheryl/core/rendering/render-frame.h>
#include <cheryl/math/collision.h>

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec2.hpp>

#include <memory>
#include <stdexcept>
#include <utility>

#include <space_invaders/game-objects/abstract-game-object.h>

struct Appearance {
    std::shared_ptr<const CE::Assets::Sprites::Asset> sprite;
    CE::Assets::Grids::CellIndex cell = 0;
};

template<int X>
struct Registers {
    constexpr static int value = X * 64;=
};

// Axis-aligned world bounds, with Y pointing up.
struct Bounds {
    glm::vec2 minimum{};
    glm::vec2 maximum{};

    [[nodiscard]] glm::vec2 size() const { return maximum - minimum; }
    [[nodiscard]] glm::vec2 center() const { return (minimum + maximum) * 0.5f; }
    [[nodiscard]] bool overlaps(const Bounds& other) const {
        return CE::Math::overlaps(center(), other.center(), size(), other.size());
    }
};

struct WorldObject : AbstractGameObject {
    std::shared_ptr<const CE::Assets::Sprites::Asset> sprite;
    glm::vec2 position{};
    float scale = 0.45f;

    explicit WorldObject(
        std::shared_ptr<const CE::Assets::Sprites::Asset> resource,
        glm::vec2 initial_position = {},
        float initial_scale = 0.45f
    ) : sprite(std::move(resource)), position(initial_position), scale(initial_scale) {
        if (!sprite) {
            throw std::invalid_argument("A world object needs a ready sprite");
        }
    }
    ~WorldObject() override = default;

    [[nodiscard]] virtual Appearance appearance() const { return {sprite, 0}; }
    [[nodiscard]] virtual Bounds bounds() const {
        const auto image = appearance();
        if (!image.sprite) {
            throw std::logic_error("A world appearance needs a sprite");
        }
        const auto& definition = image.sprite->definition();
        const glm::vec2 extent{
            static_cast<float>(definition.grid.frame.width) * scale,
            static_cast<float>(definition.grid.frame.height) * scale
        };
        const glm::vec2 first = position + glm::vec2{
            -definition.pivot.x * extent.x,
            -(1.0f - definition.pivot.y) * extent.y
        };
        const glm::vec2 second = position + glm::vec2{
            (1.0f - definition.pivot.x) * extent.x,
            definition.pivot.y * extent.y
        };
        return {glm::min(first, second), glm::max(first, second)};
    }
    void translate(glm::vec2 displacement) { position += displacement; }
    virtual void append_draws(CE::Rendering::PassWriter& pass) const {
        const auto image = appearance();
        if (!image.sprite) {
            throw std::logic_error("A world appearance needs a sprite");
        }
        const CE::Rendering::Submission::Context2D context{
            pass.semantics(), pass.parameters(), pass.constraints()
        };
        CE::Rendering::DrawStyle2D style;
        style.material = image.sprite->material;
        style.model_matrix = glm::translate(glm::mat4{1.0f}, glm::vec3{position, 0.0f});
        style.scale = scale;
        pass.add(CE::Rendering::Submission::resolve_sprite(*image.sprite, image.cell, style, context));
    }
};
