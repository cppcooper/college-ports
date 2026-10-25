#pragma once

#include <space_invaders/game-objects/abstract-game-object.h>
#include <space_invaders/systems/invasion.h>

class InvasionController : public AbstractGameObject {
    Invasion& invasion;
    std::default_random_engine RNG;
    std::bernoulli_distribution decide_to_fire;
    std::discrete_distribution<> decide_volley_size;
    std::chrono::time_point<std::chrono::system_clock> last_time_shot;
    double horizontal = 50.0;
    int num_bullets = 0;

protected:
    void onEdgeHit() const {
        invasion.descend();
        //horizontal = -1.11 * horizontal;
        EnemyDetail::time_scale *= 1.11;
        // TODO: trigger event
    }

public:
    explicit InvasionController(Invasion& invasion)
    : invasion(invasion), RNG(std::chrono::system_clock::now().time_since_epoch().count()) {
        decide_volley_size = std::discrete_distribution<>({11,22,44,24,43,37,21,7});
        last_time_shot = std::chrono::system_clock::now();
    }
    void onBulletCountChanged(const int count) {
        num_bullets = count;
    }
    void update(const CE::Game::TickContext& tick) override {
        if (auto result = invasion.move(EnemyDetail::time_scale * tick.delta_seconds, horizontal); get<0>(result)) {
            onEdgeHit();
            result = invasion.move(EnemyDetail::time_scale * get<1>(result), horizontal);
        }
        const auto now = std::chrono::system_clock::now();
        const std::chrono::duration<double> elapsed = now - last_time_shot;
        double P = ((elapsed.count()) / 500);
        // todo: acquire num bullets
        P = P + ((1 - ((static_cast<double>(num_bullets) + 1) / 30)) / 1000) + 0.02;
        if (0 < P && P < 1) {
            decide_to_fire = std::bernoulli_distribution(P);
        } else if (P > 1) {
            decide_to_fire = std::bernoulli_distribution(0.5);
        } else {
            decide_to_fire = std::bernoulli_distribution(0.00001);
        }
        if (decide_to_fire(RNG)) {
            last_time_shot = now;
            invasion.fire(decide_volley_size(RNG), RNG);
        }
    }
};
