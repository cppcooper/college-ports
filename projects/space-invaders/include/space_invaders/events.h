#pragma once

#include <cheryl/core/subsystems/event-channel.h>

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
}
