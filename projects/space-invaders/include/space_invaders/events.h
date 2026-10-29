#pragma once

#include <cheryl/core/subsystems/event-channel.h>
#include <glm/vec2.hpp>

namespace SpaceInvadersEvents {
    struct GameOver {};

    inline const CE::Events::EventChannel<GameOver> game_over_event{
        "space-invaders.game-over"
    };

    struct EdgeHit {};

    inline const CE::Events::EventChannel<EdgeHit> edge_hit_event{
        "space-invaders.edge_hit"
    };

    struct ShotRequested {
        glm::vec2 position;
        double speed;
    };

    inline const CE::Events::EventChannel<ShotRequested> shot_requested_event{
        "space-invaders.actor-fired"
    };

    struct EnemyKilled {
        int points;
    };

    inline const CE::Events::EventChannel<EnemyKilled> enemy_killed_event{
        "space-invaders.enemy-killed"
    };

    struct PlayerHit {};

    inline const CE::Events::EventChannel<PlayerHit> player_hit_event{
        "space-invaders.player-hit"
    };

    struct PlayerDeathAnimationFinished {};

    inline const CE::Events::EventChannel<PlayerDeathAnimationFinished> player_death_animation_finished_event{
        "space-invaders.player-death-animation-finished"
    };
}
