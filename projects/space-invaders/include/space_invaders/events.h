#pragma once

#include <cheryl/core/subsystems/event-channel.h>
#include <glm/vec2.hpp>

#include <string>

namespace SpaceInvadersEvents {
    struct PauseToggle {};
    inline const CE::Events::EventChannel<PauseToggle> pause_toggle_event{"space-invaders.pause-toggle"};

    struct PauseRequested { bool paused; };
    inline const CE::Events::EventChannel<PauseRequested> pause_requested_event{"space-invaders.pause-requested"};

    struct QuitRequested {};
    inline const CE::Events::EventChannel<QuitRequested> quit_requested_event{"space-invaders.quit-requested"};

    struct NewGameRequested {};
    inline const CE::Events::EventChannel<NewGameRequested> new_game_requested_event{"space-invaders.new-game-requested"};

    enum class MenuAction { NewGame, HighScores, Back, Quit, SubmitScore, SkipScore };
    struct MenuActionRequested {
        MenuAction action;
        std::string name;
    };
    inline const CE::Events::EventChannel<MenuActionRequested> menu_action_event{"space-invaders.menu-action"};

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
