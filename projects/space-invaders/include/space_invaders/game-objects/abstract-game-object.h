#pragma once

#include <cheryl/core/engine/engine-context.h>
#include <cheryl/core/game-framework/tick-context.h>
#include <cheryl/core/rendering/render-frame.h>

// SpaceInvaders must forward these phases; inheritance alone does not schedule them.
struct AbstractGameObject {
    virtual ~AbstractGameObject() = default;
    virtual void init_platform(CE::Runtime::Context& engine) {};
    virtual void init_simulation() {};
    virtual void deinit_platform() {};
    virtual void deinit_simulation() {};
    virtual void update(const CE::Game::TickContext& tick) = 0;
    virtual void append(CE::Rendering::PassWriter& pass) const {};
    virtual void append(CE::Rendering::FrameWriter& frame) const {};
};
