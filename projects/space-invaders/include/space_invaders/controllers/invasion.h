#pragma once

#include <cheryl/core/game-framework/tick-context.h>

#include <algorithm>
#include <chrono>
#include <random>

#include <space_invaders/constants.h>
#include <space_invaders/game-speed.h>
#include <space_invaders/game-objects/abstract-game-object.h>
#include <space_invaders/systems/invasion.h>

class InvasionController final : public AbstractGameObject {
    Invasion& invasion;
    std::default_random_engine random;
    std::bernoulli_distribution decide_to_fire;
    std::discrete_distribution<> decide_volley_size;
    std::chrono::steady_clock::time_point last_time_shot;
    double horizontal = 50.0;
    int num_bullets = 0;
    double respawn_pause_elapsed = SpaceInvadersConfig::player_respawn_delay_seconds;

    void onEdgeHit() {
        invasion.descend();
        horizontal = -horizontal;
        game_speed *= 1.11;
    }

public:
    explicit InvasionController(Invasion& invasion)
    : invasion(invasion),
      random(static_cast<std::default_random_engine::result_type>(std::chrono::steady_clock::now().time_since_epoch().count())),
      decide_volley_size({11, 22, 44, 24, 43, 37, 21, 7}),
      last_time_shot(std::chrono::steady_clock::now()) {}
    void onBulletCountChanged(int count) { num_bullets = count; }
    void onPlayerHit() { respawn_pause_elapsed = 0.0; }
    void init_simulation() override {
        horizontal = 50.0;
        num_bullets = 0;
        respawn_pause_elapsed = SpaceInvadersConfig::player_respawn_delay_seconds;
        last_time_shot = std::chrono::steady_clock::now();
    }
    void update(const CE::Game::TickContext& tick) override {
        if (respawn_pause_elapsed < SpaceInvadersConfig::player_respawn_delay_seconds) {
            respawn_pause_elapsed = std::clamp(
                respawn_pause_elapsed + tick.delta_seconds, 0.0,
                SpaceInvadersConfig::player_respawn_delay_seconds
            );
            if (respawn_pause_elapsed < SpaceInvadersConfig::player_respawn_delay_seconds) return;
        }
        const auto [edge, remaining] = invasion.move(tick.delta_seconds, horizontal * game_speed);
        if (edge) {
            onEdgeHit();
            static_cast<void>(invasion.move(remaining, horizontal * game_speed));
        }

        const auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<double> elapsed = now - last_time_shot;
        const double probability = elapsed.count() / 500.0
            + (1.0 - (static_cast<double>(num_bullets) + 1.0) / 30.0) / 1000.0 + 0.02;
        if (probability > 0.0 && probability < 1.0) {
            decide_to_fire = std::bernoulli_distribution(probability);
        } else if (probability > 1.0) {
            decide_to_fire = std::bernoulli_distribution(0.5);
        } else {
            decide_to_fire = std::bernoulli_distribution(0.00001);
        }
        if (decide_to_fire(random)) {
            last_time_shot = now;
            invasion.fire(decide_volley_size(random), random);
        }
    }
};
