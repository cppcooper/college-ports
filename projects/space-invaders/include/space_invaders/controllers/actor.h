#pragma once
#include <space_invaders/game-objects/abstract-game-object.h>
#include <space_invaders/game-objects/actor.h>

class ActorController : public AbstractGameObject {
protected:
    std::shared_ptr<ActorBase> actor;

public:
    ~ActorController() override = default;
    virtual void attach(const std::shared_ptr<ActorBase> &an_actor) {actor = an_actor;}
    virtual void detach() {actor = nullptr;}
    virtual void move(double,float) = 0;
    virtual void fire() = 0;
};