#pragma once

#include <cheryl/core/subsystems/event-bus.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <random>
#include <tuple>
#include <vector>
#include <span>

#include <space_invaders/events.h>
#include <space_invaders/game-objects/abstract-game-object.h>
#include <space_invaders/game-objects/actors.h>

class Invasion : public AbstractGameObject {
    static constexpr int invaders = 81;

    CE::Events::EventBus::Publisher event_publisher;
    std::uniform_int_distribution<> decide_shooters;
    std::shared_ptr<EnemyActor> invasion_alloc;
    std::vector<EnemyActor*> front_line;
    int screen_width = 1;

public:
    [[nodiscard]] std::span<const EnemyActor> enemies() const {
        return {invasion_alloc.get(), invaders};
    }
    void spawn();
    void fire(int shots, std::default_random_engine& RNG) {
        const auto size = front_line.size();
        shots = std::min(shots, static_cast<int>(size));
        decide_shooters = std::uniform_int_distribution<int>(0, static_cast<int>(size-1));
        std::array<int, 10> column_tracker{};
        column_tracker.fill(-1);
        for (int i = 0; i < shots; ++i) {
            int idx;
            do {
                idx = decide_shooters(RNG);
            } while (std::ranges::any_of(column_tracker, [decision = idx](const int idx) {return idx == decision;}));
            column_tracker[i] = idx;
        }
        for (const auto idx : column_tracker) {
            front_line[idx]->fire();
        }
    }
    // Returns {edge reached, remaining seconds}; horizontal is signed pixels/second.
    [[nodiscard]] std::tuple<bool, double> move(const double& delta, const double& horizontal) const {
        if (front_line.empty()) {
            throw std::logic_error("Empty front-line, the game should have spawned a new wave");
        }
        if (delta <= 0.0 || horizontal == 0.0) {
            return {false, 0.0};
        }
        const bool right = horizontal > 0;
        const double edge = right ? static_cast<double>(screen_width) : 0.0;
        const EnemyActor* lead = right ? front_line.back() : front_line.front();
        const double enemy_half_width = lead->sprite->definition().grid.frame.width * lead->scale * 0.5;
        const double pos = lead->position.x + (right ? enemy_half_width : -enemy_half_width);

        // has the edge already been hit?
        const double gap = right ? edge - pos : pos;
        if (gap <= 0.0) {
            return {true, delta};
        }

        // edge hit check & handling begins here
        const double time_to_edge = std::max(0.0, gap) / std::abs(horizontal);
        const bool edge_reached = delta >= time_to_edge;

        // movement code begins
        const double movement_seconds = std::min(delta, time_to_edge);
        const double remaining_seconds = edge_reached ? std::max(0.0, delta - movement_seconds) : 0.0;

        const auto ptr = invasion_alloc.get();
        for (int i = 0; i < invaders; ++i) {
            if (auto& invader = ptr[i]; invader.is_alive()) {
                invader.move(movement_seconds, horizontal);
            }
        }
        return {edge_reached, remaining_seconds};
    }
    void descend() const {
        double lowest_enemy = std::numeric_limits<double>::max();
        const auto ptr = invasion_alloc.get();
        for (int i = 0; i < invaders; ++i) {
            if (auto& invader = ptr[i]; invader.is_alive()) {
                invader.position.y -= 10.0;
                if (const double bottom = invader.position.y - invader.sprite->definition().grid.frame.height * invader.scale * 0.5; bottom < lowest_enemy) {
                    lowest_enemy = bottom;
                }
            }
        }
        if (lowest_enemy <= PLAYER_Y) {
            event_publisher.notify(SpaceInvadersEvents::game_over_event, SpaceInvadersEvents::GameOver{});
        }
    }

    void init_platform(CE::Runtime::Context& engine) override {
        screen_width = engine.window().framebuffer_size().width;
        event_publisher = engine.events().publisher();
    }
    void init_simulation() override;
    void deinit_platform() override;
    void deinit_simulation() override {
        front_line.clear();
        invasion_alloc.reset();
    }
    void update(const CE::Game::TickContext& tick) override {
        const auto ptr = invasion_alloc.get();
        for (int i = 0; i < invaders; ++i) {
            if (auto& invader = ptr[i]; invader.is_alive()) {
                invader.update(tick);
            }
        }
    };
    void append(CE::Rendering::PassWriter& pass) const override {
        const auto ptr = invasion_alloc.get();
        for (int i = 0; i < invaders; ++i) {
            if (auto& invader = ptr[i]; invader.is_alive()) {
                invader.append(pass);
            }
        }
    }
};
