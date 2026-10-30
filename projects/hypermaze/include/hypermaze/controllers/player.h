#pragma once

#include <cheryl/core/engine/engine-context.h>
#include <cheryl/core/game-framework/tick-context.h>
#include <hypermaze/world/Graph.h>

#include <optional>

struct PlayerCommands {
    std::optional<HypermazeModel::Direction> move;
    bool quit = false;
    bool regenerate = false;
    bool toggle_debug = false;
};

class PlayerController {
    CE::Input::Actions::Context controls;
    std::optional<HypermazeModel::Direction> previous;
    double cooldown = 0;

public:
    void init_platform(CE::Runtime::Context& engine);
    void deinit_platform() { controls = {}; }
    PlayerCommands update(const CE::Game::TickContext& tick);
};
