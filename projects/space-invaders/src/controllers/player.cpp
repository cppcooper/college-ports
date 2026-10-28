#include <space_invaders/controllers/player.h>

void PlayerController::update(const CE::Game::TickContext& tick) {
    if (!actor) return;
    const auto actions = tick.input.actions(controls);
    const float direction = actions.axis(ActorActions::move).current
        + (actions.button(ActorActions::left).held() ? -1.0f : 0.0f)
        + (actions.button(ActorActions::right).held() ? 1.0f : 0.0f);
    actor->move(tick.delta_seconds, direction * 160.0f);
    if (actions.button(ActorActions::fire).pressed()) {
        actor->fire();
    }
}
