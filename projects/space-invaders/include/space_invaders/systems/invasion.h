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

        ~Pool() {
            auto* storage = enemies.data();
            for (auto i = enemies.size(); i > 0; --i)
                Traits::destroy(allocator, storage + i - 1);
            if (storage) Traits::deallocate(allocator, storage, enemy_count);
        }

        EnemyActor& construct(
            const EnemyType type, const glm::vec2 position,
            const CE::Events::EventBus::Publisher& publisher
        ) {
            auto* storage = enemies.data();
            auto* enemy = storage + enemies.size();
            Traits::construct(allocator, enemy, type, position, static_cast<int>(enemies.size()), publisher);
            enemies = {storage, enemies.size() + 1};
            return *enemy;
        }

        // Factory: reserve once, then construct each column from its front row back.
        static std::unique_ptr<Pool> create(
            const CE::Events::EventBus::Publisher& publisher, const glm::vec2 world_size
        ) {
            auto result = std::make_unique<Pool>();
            result->front_line.reserve(columns);
            result->enemies = {Traits::allocate(result->allocator, enemy_count), 0};
            constexpr std::array types{
                EnemyType::MALOLRIE, EnemyType::ARCHER, EnemyType::LANA, EnemyType::CHERYL,
                EnemyType::CYRIL, EnemyType::RAY, EnemyType::KRIEGER
            };

            glm::vec2 largest{};
            for (const auto type : types) {
                const auto sprite = GetEnemySprite(type);
                if (!sprite) {
                    throw std::logic_error("Load invader sprites before spawning the invasion");
                }
                const auto& frame = sprite->definition().grid.frame;
                largest.x = std::max(largest.x, frame.width * 0.45f);
                largest.y = std::max(largest.y, frame.height * 0.45f);
            }
            const float column_spacing = std::min(
                largest.x + 12.0f, std::max(0.0f, world_size.x - largest.x - 48.0f) / (columns - 1)
            );
            const float row_spacing = std::min(
                largest.y + 8.0f, std::max(0.0f, world_size.y - PlayerActor::spawn_height - 150.0f) / (rows - 1)
            );
            const float first_x = (world_size.x - column_spacing * (columns - 1)) * 0.5f;
            const float first_y = world_size.y - 100.0f - row_spacing * (rows - 1);

            for (std::size_t column = 0; column < columns; ++column) {
                for (std::size_t row = 0; row < rows; ++row) {
                    auto& enemy = result->construct(
                        types[row],
                        glm::vec2{first_x + column * column_spacing, first_y + row * row_spacing},
                        publisher
                    );
                    if (row == 0) {
                        result->front_line.push_back(&enemy);
                    }
                }
            }
            // The saucer shares the allocation, but is outside the formation columns.
            result->construct(
                EnemyType::WOODIE, glm::vec2{world_size.x * 0.5f, world_size.y - 45.0f},
                publisher
            );
            return result;
        }

        void refresh_front_line() {
            front_line.clear();
            for (std::size_t column = 0; column < columns; ++column) {
                const auto first = column * rows;
                for (std::size_t row = 0; row < rows; ++row) {
                    auto& enemy = enemies[first + row];
                    if (enemy.is_alive()) {
                        front_line.push_back(&enemy);
                        break;
                    }
                }
            }
        }

        [[nodiscard]] std::optional<Bounds> formation_bounds() const {
            std::optional<Bounds> result;
            for (std::size_t i = 0; i < formation_size; ++i) {
                if (!enemies[i].is_alive()) continue;
                const auto shape = enemies[i].bounds();
                if (!result) {
                    result = shape;
                } else {
                    result->minimum = glm::min(result->minimum, shape.minimum);
                    result->maximum = glm::max(result->maximum, shape.maximum);
                }
            }
            return result;
        }
    };
    std::unique_ptr<Pool> pool;
    CE::Events::EventBus::Publisher event_publisher;
    glm::vec2 world_size;

public:
    static constexpr std::size_t columns = 10;
    static constexpr std::size_t rows = 7;
    static constexpr std::size_t formation_size = columns * rows;
    static constexpr std::size_t enemy_count = formation_size + 1; // Last slot is the saucer.

    explicit Invasion(CE::Events::EventBus::Publisher publisher = {}, const glm::vec2 world_size = {1, 1})
    : event_publisher(std::move(publisher)), world_size(world_size) {}
    ~Invasion() override = default;

    [[nodiscard]] std::span<EnemyActor> enemies() noexcept {
        return pool ? std::span<EnemyActor>{pool->enemies.data(), pool->enemies.size()} : std::span<EnemyActor>{};
    }
    [[nodiscard]] std::span<const EnemyActor> enemies() const noexcept {
        return pool ? std::span<const EnemyActor>{pool->enemies.data(), pool->enemies.size()} : std::span<const EnemyActor>{};
    }
    void spawn() {
        // Keep the current wave intact if allocation or a constructor fails.
        pool = Pool::create(event_publisher, world_size);
    }
    void fire(int shots, std::default_random_engine& random) {
        if (!pool || shots <= 0) return;
        pool->refresh_front_line();
        auto shooters = pool->front_line;
        std::shuffle(shooters.begin(), shooters.end(), random);
        const auto count = std::min(static_cast<std::size_t>(shots), shooters.size());
        for (std::size_t i = 0; i < count; ++i)
            shooters[i]->fire();
    }
    // Returns {edge reached, remaining seconds}; horizontal is signed pixels/second.
    [[nodiscard]] std::tuple<bool, double> move(double delta_seconds, double horizontal) {
        if (!pool || delta_seconds <= 0.0 || horizontal == 0.0) return {false, 0.0};
        const auto shape = pool->formation_bounds();
        if (!shape) return {false, 0.0};
        const bool right = horizontal > 0.0;
        const double gap = right ? world_size.x - shape->maximum.x : shape->minimum.x;
        if (gap <= 0.0) return {true, delta_seconds};

        const double time_to_edge = gap / std::abs(horizontal);
        const double movement_seconds = std::min(delta_seconds, time_to_edge);
        for (std::size_t i = 0; i < formation_size; ++i) {
            auto& enemy = pool->enemies[i];
            if (enemy.is_alive()) enemy.move(movement_seconds, horizontal);
        }
        const bool edge_reached = delta_seconds >= time_to_edge;
        return {edge_reached, edge_reached ? delta_seconds - movement_seconds : 0.0};
    }
    void descend() {
        if (!pool) return;
        for (std::size_t i = 0; i < formation_size; ++i) {
            auto& enemy = pool->enemies[i];
            if (enemy.is_alive()) enemy.translate({0.0f, -10.0f});
        }
        const auto shape = pool->formation_bounds();
        if (shape && shape->minimum.y <= PlayerActor::spawn_height) {
            event_publisher.notify(SpaceInvadersEvents::game_over_event, SpaceInvadersEvents::GameOver{});
        }
    }
    void accelerate_animation(double factor) {
        if (!pool) return;
        for (std::size_t i = 0; i < formation_size; ++i)
            pool->enemies[i].animation_speed *= factor;
    }

    void init_platform(CE::Runtime::Context& engine) override {
        const auto size = engine.window().framebuffer_size();
        world_size = {size.width, size.height};
        event_publisher = engine.events().publisher();
    }
    void init_simulation() override { spawn(); }
    void deinit_platform() override { event_publisher = {}; }
    void deinit_simulation() override { pool.reset(); }
    void update(const CE::Game::TickContext& tick) override {
        if (!pool) return;
        for (auto& enemy : pool->enemies)
            enemy.update(tick);
        pool->refresh_front_line();
    }
    void append_draws(CE::Rendering::PassWriter& pass) const {
        if (!pool) return;
        for (const auto& enemy : pool->enemies) {
            if (enemy.is_alive()) enemy.append_draws(pass);
        }
    }
};
