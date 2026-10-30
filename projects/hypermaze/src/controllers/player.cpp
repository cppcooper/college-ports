#include <hypermaze/controllers/player.h>
#include <college_ports/controls.h>

#include <array>

namespace {
    constexpr CE::Input::Actions::Id left{1}, right{2}, up{3}, down{4}, quit{5}, regenerate{6}, debug{7};
}

void PlayerController::init_platform(CE::Runtime::Context& engine) {
    using K = CE::Input::KeyboardKey;
    using G = CE::Input::GamepadControl;
    auto& input = engine.input();
    auto definition = CollegePorts::keyboard_controls(input, {
        {K::A, left}, {K::Left, left}, {K::D, right}, {K::Right, right},
        {K::W, up}, {K::Up, up}, {K::S, down}, {K::Down, down},
        {K::Escape, quit}, {K::R, regenerate}, {K::F3, debug}});
    CollegePorts::gamepad_controls(definition, input, {
        {G::DPadLeft, left}, {G::DPadRight, right}, {G::DPadUp, up}, {G::DPadDown, down}});
    controls = input.action_contexts().attach(definition);
}

PlayerCommands PlayerController::update(const CE::Game::TickContext& tick) {
    using D = HypermazeModel::Direction;
    const auto actions = tick.input.actions(controls);
    PlayerCommands result{.quit = actions.button(quit).pressed(),
        .regenerate = actions.button(regenerate).pressed(), .toggle_debug = actions.button(debug).pressed()};
    cooldown -= tick.delta_seconds;
    const std::array directions{std::pair{left, D::Left}, std::pair{right, D::Right},
        std::pair{up, D::Up}, std::pair{down, D::Down}};
    for (const auto& [action, direction] : directions) {
        const auto state = actions.button(action);
        if (!state.pressed() && !state.held()) continue;
        if (state.pressed() || previous != direction || cooldown <= 0) {
            result.move = direction;
            cooldown = 0.12;
        }
        previous = direction;
        return result;
    }
    previous.reset();
    cooldown = 0;
    return result;
}
