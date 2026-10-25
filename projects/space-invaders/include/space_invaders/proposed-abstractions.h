#pragma once

#include <cheryl/assets/submission/draw2d.h>
#include <cheryl/assets/types/2d/sprite.h>
#include <cheryl/core/game-framework/tick-context.h>
#include <cheryl/core/rendering/render-frame.h>
#include <cheryl/core/subsystems/event-bus.h>
#include <cheryl/math/collision.h>

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec2.hpp>

#include <memory>
#include <stdexcept>
#include <utility>

#include <space_invaders/events.h>

// Proposal only: existing game types do not use this header.
// Intended inheritance (endpoints remain unmodified):
//   AbstractGameObject
//     WorldObject
//       ActorBase -> PlayerActor, EnemyActor
//       Bullet
//     GameUI (also implements the separate FrameRenderable interface)
// Invasion and the bullet manager advance their owned objects exactly once.
// Controllers make decisions and call operations; owners schedule object updates.
// Runtime lifecycle interfaces are deferred.
namespace Proposed {
    // The common per-tick contract; each endpoint advances its own state.
    struct AbstractGameObject {
        virtual ~AbstractGameObject() = default;
        virtual void update(const CE::Game::TickContext& tick) = 0;
    };

    // UI contributes prepared passes after the world pass. Toolkit state and
    // upload machinery remain owned by GameUI.
    struct FrameRenderable {
        virtual ~FrameRenderable() = default;
        virtual void append_passes(CE::Rendering::FrameWriter& frame) const = 0;
    };

    struct Appearance {
        std::shared_ptr<const CE::Assets::Sprites::Asset> sprite;
        CE::Assets::Grids::CellIndex cell = 0;
    };

    // Axis-aligned bounds in world coordinates, with Y pointing up.
    struct Bounds {
        glm::vec2 minimum{};
        glm::vec2 maximum{};

        [[nodiscard]] glm::vec2 size() const { return maximum - minimum; }
        [[nodiscard]] glm::vec2 center() const { return (minimum + maximum) * 0.5f; }

        [[nodiscard]] bool overlaps(const Bounds& other) const {
            return CE::Math::overlaps(center(), other.center(), size(), other.size());
        }
    };

    // Shared world representation for ships and bullets. Ready sprite resources
    // are supplied by the owner; position is the sprite's authored pivot.
    struct WorldObject : AbstractGameObject {
        std::shared_ptr<const CE::Assets::Sprites::Asset> sprite;
        glm::vec2 position{};
        float scale = 0.45f;

        explicit WorldObject(
            std::shared_ptr<const CE::Assets::Sprites::Asset> resource,
            const glm::vec2 initial_position = {},
            const float initial_scale = 0.45f
        ) : sprite(std::move(resource)), position(initial_position), scale(initial_scale) {
            if (!sprite)
                throw std::invalid_argument("A world object needs a ready sprite");
        }

        ~WorldObject() override = default;

        // Static sprites use cell zero. Animated/death appearances can override it.
        [[nodiscard]] virtual Appearance appearance() const { return {sprite, 0}; }

        void translate(const glm::vec2 displacement) { position += displacement; }

        // The default hitbox follows the displayed sprite's pivot and scale.
        // Endpoints can override it when their collision shape differs.
        [[nodiscard]] virtual Bounds bounds() const {
            const auto image = appearance();
            if (!image.sprite)
                throw std::logic_error("A world appearance needs a sprite");
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

        // Copy render values into an existing world pass; advance no simulation.
        virtual void append_draws(CE::Rendering::PassWriter& pass) const {
            const auto image = appearance();
            if (!image.sprite)
                throw std::logic_error("A world appearance needs a sprite");
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

    // Shared player/enemy operations extend the same world state and rendering.
    // Bullets inherit WorldObject directly without acquiring actor operations.
    struct ActorBase : WorldObject {
        CE::Events::EventBus::Publisher event_publisher;
        double projectile_speed = -200.0;
        bool is_dead = false;
        bool is_shot = false;

        ActorBase(
            std::shared_ptr<const CE::Assets::Sprites::Asset> resource,
            CE::Events::EventBus::Publisher publisher,
            const glm::vec2 initial_position = {},
            const float initial_scale = 0.45f,
            const double projectile_speed = -200.0
        ) : WorldObject(std::move(resource), initial_position, initial_scale),
            event_publisher(std::move(publisher)), projectile_speed(projectile_speed) {}

        ~ActorBase() override = default;

        // Basic horizontal translation. Player clamping and formation boundary
        // handling belong to their specific operations; bullets integrate velocity.
        virtual void move(const double delta_seconds, const double horizontal_speed) {
            translate({static_cast<float>(delta_seconds * horizontal_speed), 0.0f});
        }

        // Produce a shot request at the upper/lower edge of the displayed sprite.
        // The bullet manager creates and owns the resulting projectile.
        virtual void fire() {
            const auto shape = bounds();
            auto origin = shape.center();
            origin.y = projectile_speed > 0.0 ? shape.maximum.y : shape.minimum.y;
            event_publisher.notify(
                SpaceInvadersEvents::shot_requested_event,
                SpaceInvadersEvents::ShotRequested{.position = origin, .speed = projectile_speed}
            );
        }

        [[nodiscard]] bool is_alive() const noexcept { return !is_dead; }

        // Player life loss and enemy death/scoring need different responses.
        virtual void hit() = 0;
    };
}
