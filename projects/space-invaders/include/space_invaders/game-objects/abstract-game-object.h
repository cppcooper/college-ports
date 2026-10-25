#pragma once

namespace CE::Game { struct TickContext; }
namespace CE::Runtime { class Context; }

// SpaceInvaders must forward these phases; inheritance alone does not schedule them.
struct AbstractGameObject {
    virtual ~AbstractGameObject() = default;
    virtual void init_platform(CE::Runtime::Context&) {}
    virtual void init_simulation() {}
    virtual void deinit_platform() {}
    virtual void deinit_simulation() {}
    virtual void update(const CE::Game::TickContext& tick) = 0;
};
