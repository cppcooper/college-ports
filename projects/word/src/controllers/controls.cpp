#include <word/controllers/controls.h>
#include <college_ports/controls.h>

namespace {
    constexpr CE::Input::Actions::Id quit{1}, previous{2}, next{3}, reveal{4}, all{5}, rescan{6};
}
void WordController::init_platform(CE::Runtime::Context& engine) {
    using K = CE::Input::KeyboardKey;
    using G = CE::Input::GamepadControl;
    auto& input = engine.input();
    auto definition = CollegePorts::keyboard_controls(input, {
        {K::Escape, quit}, {K::Up, previous}, {K::Left, previous}, {K::Down, next}, {K::Right, next},
        {K::Space, reveal}, {K::A, all}, {K::R, rescan}});
    CollegePorts::gamepad_controls(definition, input, {
        {G::DPadUp, previous}, {G::DPadLeft, previous}, {G::DPadDown, next}, {G::DPadRight, next}, {G::A, reveal}});
    controls = input.action_contexts().attach(definition);
}
WordCommands WordController::update(const CE::Game::TickContext& tick) const {
    const auto actions = tick.input.actions(controls);
    return {actions.button(quit).pressed(), actions.button(previous).pressed(), actions.button(next).pressed(),
        actions.button(reveal).pressed(), actions.button(all).pressed(), actions.button(rescan).pressed()};
}
