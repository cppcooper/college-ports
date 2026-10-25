#pragma once

#include <cheryl/core/controls/action-context.h>
#include <cheryl/core/controls/button-codes.h>
#include <cheryl/core/controls/input-interface.h>
#include <cheryl/core/controls/tick-input.h>
#include <cheryl/core/engine/engine-context.h>
#include <cheryl/core/game-framework/tick-context.h>

#include <array>
#include <utility>

#include <space_invaders/game-objects/abstract-game-object.h>
#include <space_invaders/game-objects/actors.h>

class ActorController : public AbstractGameObject {
protected:
    ActorBase* actor = nullptr;

public:
    ~ActorController() override = default;
    virtual void attach(ActorBase& target) { actor = &target; }
    virtual void detach() { actor = nullptr; }
};

struct ActorActions {
    static constexpr CE::Input::Actions::Id move{1};
    static constexpr CE::Input::Actions::Id left{2};
    static constexpr CE::Input::Actions::Id right{3};
    static constexpr CE::Input::Actions::Id fire{4};
};

class PlayerController final : public ActorController {
    CE::Input::Actions::Context controls;

public:
    [[nodiscard]] static CE::Input::Actions::ContextDefinition defined_controls(const CE::Input::System& system) {
        using namespace CE::Input;
        const auto keyboard = system.keyboard_id();
        const auto gamepad = system.gamepad_id();
        const std::array buttons{
            std::pair{Control{keyboard, KeyboardKey::A}, ActorActions::left},
            std::pair{Control{keyboard, KeyboardKey::D}, ActorActions::right},
            std::pair{Control{keyboard, KeyboardKey::Left}, ActorActions::left},
            std::pair{Control{keyboard, KeyboardKey::Right}, ActorActions::right},
            std::pair{Control{keyboard, KeyboardKey::Space}, ActorActions::fire},
            std::pair{Control{gamepad, GamepadControl::A}, ActorActions::fire},
            std::pair{Control{gamepad, GamepadControl::RightTrigger}, ActorActions::fire},
            std::pair{Control{gamepad, GamepadControl::RightShoulder}, ActorActions::fire}
        };

        Actions::ContextDefinition definition;
        definition.axes.push_back(Actions::ContextAxis{
            .modifiers = {}, .control = {gamepad, GamepadControl::LeftStickX},
            .action = ActorActions::move, .options = {}
        });
        for (const auto& [control, action] : buttons)
            definition.buttons.push_back(Actions::ContextButton{
                .chord = {{control}}, .action = action
            });
        return definition;
    }
    void init_platform(CE::Runtime::Context& engine) override {
        auto& input = engine.input();
        controls = input.action_contexts().attach(defined_controls(input));
    }
    void deinit_platform() override { controls = {}; }
    void deinit_simulation() override { detach(); }
    void update(const CE::Game::TickContext& tick) override {
        if (!actor) return;
        const auto actions = tick.input.actions(controls);
        const float direction = actions.axis(ActorActions::move).current
            + (actions.button(ActorActions::left).held() ? -1.0f : 0.0f)
            + (actions.button(ActorActions::right).held() ? 1.0f : 0.0f);
        actor->move(tick.delta_seconds, direction * 160.0f);
        if (actions.button(ActorActions::fire).pressed()) {
            actor->fire();
        }
    }
};
