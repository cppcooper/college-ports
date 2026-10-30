#include <rover/controllers/controls.h>
#include <college_ports/controls.h>

namespace {
    constexpr CE::Input::Actions::Id quit{1}, pause{2}, restart{3}, editor{4}, step{5}, save{6}, spray{7};
    constexpr CE::Input::Actions::Id larger{8}, smaller{9}, left{10}, right{11}, up{12}, down{13};
    constexpr CE::Input::Actions::Id rock{14}, hole{15}, dirt{16}, faster{17}, slower{18};
}
void RoverController::init_platform(CE::Runtime::Context& engine) {
    using K = CE::Input::KeyboardKey;
    auto& input = engine.input();
    controls = input.action_contexts().attach(CollegePorts::keyboard_controls(input, {
        {K::Escape, quit}, {K::Space, pause}, {K::R, restart}, {K::E, editor}, {K::N, step},
        {K::F5, save}, {K::LeftShift, spray}, {K::RightShift, spray}, {K::Up, larger}, {K::Down, smaller},
        {K::A, left}, {K::D, right}, {K::W, up}, {K::S, down},
        {K::Digit1, rock}, {K::Digit2, hole}, {K::Digit3, dirt}, {K::Equal, faster}, {K::Minus, slower}}));
}
RoverCommands RoverController::update(const CE::Game::TickContext& tick) {
    const auto actions = tick.input.actions(controls);
    const auto pressed = [&](auto id) { return actions.button(id).pressed(); };
    const auto active = [&](auto id) { const auto s = actions.button(id); return s.held() || s.pressed(); };
    RoverCommands result{.quit = pressed(quit), .pause = pressed(pause), .restart = pressed(restart),
        .toggle_editor = pressed(editor), .step = pressed(step), .save = pressed(save), .spray = pressed(spray),
        .faster = pressed(faster), .slower = pressed(slower), .brush_delta = int(pressed(larger))-int(pressed(smaller)),
        .pan = {int(active(right))-int(active(left)), int(active(up))-int(active(down))}};
    if (pressed(rock)) result.tile = 0;
    if (pressed(hole)) result.tile = 1;
    if (pressed(dirt)) result.tile = 2;
    for (const auto& record : tick.input.gameplay_events()) {
        if (const auto* motion = std::get_if<CE::Input::Events::Pointer>(&record.data)) {
            pointer = *motion;
            if (painting) result.strokes.push_back(*motion);
        } else if (const auto* button = std::get_if<CE::Input::Events::Button>(&record.data);
                   button && button->mouse_button == CE::Input::MouseButton::Left) {
            if (button->position) pointer = button->position;
            if (button->phase == CE::Input::Events::ButtonPhase::Press) {
                painting = true;
                if (pointer) result.strokes.push_back(*pointer);
            } else if (button->phase == CE::Input::Events::ButtonPhase::Release) painting = false;
        }
    }
    return result;
}
