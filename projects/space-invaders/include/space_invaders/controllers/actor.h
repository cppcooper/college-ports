#pragma once
#include <cheryl/core/game-framework/tick-context.h>

class Actor;

class ActorController {
protected:
    Actor* actor = nullptr;

public:
    virtual ~ActorController() = default;
    virtual void move(float) = 0;
    virtual void fire() = 0;
    virtual void update(const CE::Game::TickContext& tick) = 0;
};