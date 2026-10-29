#pragma once

#include <cheryl/core/display/window-interface.h>
#include <cheryl/core/engine/engine-context.h>
#include <cheryl/core/game-framework/tick-context.h>
#include <cheryl/core/resources/memory/allocators/object-pool-allocator.hpp>
#include <cheryl/core/subsystems/event-bus.h>

#include <glm/common.hpp>
#include <glm/vec2.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <memory>
#include <optional>
#include <random>
#include <span>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include <space_invaders/events.h>
#include <space_invaders/game-objects/abstract-game-object.h>
#include <space_invaders/game-objects/actors.h>

class Invasion final : public AbstractGameObject {
    struct Pool {
        using Allocator = CE::Memory::Allocators::ObjectPoolAllocator<EnemyActor>;
        using Traits = std::allocator_traits<Allocator>;

        Allocator allocator;
        std::span<EnemyActor> enemies;
        std::vector<EnemyActor*> front_line;

        Pool() = default;
        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;
        ~Pool();

        EnemyActor& construct(
            EnemyType type, glm::vec2 position, const CE::Events::EventBus::Publisher& publisher,
            const std::shared_ptr<const CE::Rendering::Resources::Material>& material
        );
        // Factory: reserve once, then construct each column from its front row back.
        static std::unique_ptr<Pool> create(
            const CE::Events::EventBus::Publisher& publisher, glm::vec2 world_size,
            const std::shared_ptr<const CE::Rendering::Resources::Material>& material
        );
        void refresh_front_line();
        [[nodiscard]] std::optional<Bounds> formation_bounds() const;
    };
    std::unique_ptr<Pool> pool;
    std::shared_ptr<const CE::Rendering::Resources::Material> material;
    CE::Events::EventBus::Publisher event_publisher;
    glm::vec2 world_size;

public:
    static constexpr std::size_t columns = 10;
    static constexpr std::size_t rows = 7;
    static constexpr std::size_t formation_size = columns * rows;

    explicit Invasion(CE::Events::EventBus::Publisher publisher = {}, const glm::vec2 world_size = {1, 1})
    : event_publisher(std::move(publisher)), world_size(world_size) {}
    ~Invasion() override = default;

    void fire(int shots, std::default_random_engine& random) const;
    // Returns {edge reached, remaining seconds}; horizontal is signed pixels/second.
    [[nodiscard]] std::tuple<bool, double> move(double delta_seconds, double horizontal) const;
    void descend() const;
    void set_material(std::shared_ptr<const CE::Rendering::Resources::Material> value) { material = std::move(value); }

    void init_platform(CE::Runtime::Context& engine) override {
        const auto size = engine.window().framebuffer_size();
        world_size = {size.width, size.height};
        event_publisher = engine.events().publisher();
    }
    void init_simulation() override { spawn(); }
    void deinit_platform() override {
        event_publisher = {};
        material.reset();
    }
    void deinit_simulation() override { pool.reset(); }
    void update(const CE::Game::TickContext& tick) override;
    void append_draws(CE::Rendering::PassWriter& pass) const;

    [[nodiscard]] std::span<EnemyActor> enemies() noexcept {
        return pool ? std::span<EnemyActor>{pool->enemies.data(), pool->enemies.size()} : std::span<EnemyActor>{};
    }
    [[nodiscard]] std::span<const EnemyActor> enemies() const noexcept {
        return pool ? std::span<const EnemyActor>{pool->enemies.data(), pool->enemies.size()} : std::span<const EnemyActor>{};
    }
    void spawn() {
        // Keep the current wave intact if allocation or a constructor fails.
        pool = Pool::create(event_publisher, world_size, material);
    }
};
