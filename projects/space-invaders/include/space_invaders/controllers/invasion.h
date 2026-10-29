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
    double time_since_shot = 0.0;
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
      decide_volley_size({11, 22, 44, 24, 43, 37, 21, 7}) {}
    void onBulletCountChanged(int count) { num_bullets = count; }
    void onPlayerHit() { respawn_pause_elapsed = 0.0; }
    void init_simulation() override {
        horizontal = 50.0;
        num_bullets = 0;
        respawn_pause_elapsed = SpaceInvadersConfig::player_respawn_delay_seconds;
        time_since_shot = 0.0;
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

        time_since_shot += tick.delta_seconds;
        const double probability = time_since_shot / 500.0
            + (1.0 - (static_cast<double>(num_bullets) + 1.0) / 30.0) / 1000.0 + 0.02;
        if (probability > 0.0 && probability < 1.0) {
            decide_to_fire = std::bernoulli_distribution(probability);
        } else if (probability > 1.0) {
            decide_to_fire = std::bernoulli_distribution(0.5);
        } else {
            decide_to_fire = std::bernoulli_distribution(0.00001);
        }
        if (decide_to_fire(random)) {
            time_since_shot = 0.0;
            invasion.fire(decide_volley_size(random), random);
        }
    }
};
