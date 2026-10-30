#pragma once

#include <cheryl/core/engine/engine-context.h>
#include <cheryl/core/game-framework/tick-context.h>
#include <rover/levels/tile-map.h>

#include <optional>

struct RoverCommands {
    bool quit = false, pause = false, restart = false, toggle_editor = false;
    bool step = false, save = false, spray = false, faster = false, slower = false;
    int brush_delta = 0;
    RoverModel::Position pan{};
    std::optional<std::uint8_t> tile;
    std::vector<CE::Input::Events::Pointer> strokes;
};
class RoverController {
    CE::Input::Actions::Context controls;
    std::optional<CE::Input::Events::Pointer> pointer;
    bool painting = false;
public:
    void init_platform(CE::Runtime::Context& engine);
    void deinit_platform() { controls = {}; }
    RoverCommands update(const CE::Game::TickContext& tick);
};
