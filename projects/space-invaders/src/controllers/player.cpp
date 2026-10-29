#include <space_invaders/controllers/player.h>
#include <space_invaders/game-speed.h>

void PlayerController::update(const CE::Game::TickContext& tick) {
    time_since_shot += tick.delta_seconds;
    if (!actor || !actor->is_alive()) return;
    const auto actions = tick.input.actions(controls);
    const float direction = actions.axis(ActorActions::move).current
        + (actions.button(ActorActions::left).held() ? -1.0f : 0.0f)
        + (actions.button(ActorActions::right).held() ? 1.0f : 0.0f);
    actor->move(tick.delta_seconds, direction * horizontal * game_speed);
    if (time_since_shot > ActorActions::fire_cd / game_speed && actions.button(ActorActions::fire).pressed()) {
        time_since_shot = 0;
        actor->fire();
    }
}
