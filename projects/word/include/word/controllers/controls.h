#pragma once

#include <cheryl/core/engine/engine-context.h>
#include <cheryl/core/game-framework/tick-context.h>

struct WordCommands { bool quit, previous, next, reveal, reveal_all, rescan; };
class WordController {
    CE::Input::Actions::Context controls;
public:
    void init_platform(CE::Runtime::Context& engine);
    void deinit_platform() { controls = {}; }
    WordCommands update(const CE::Game::TickContext& tick) const;
};
