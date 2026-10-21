#pragma once
#include <cheryl/core/controls/input-interface.h>
#include <cheryl/core/controls/action-context.h>
#include <cheryl/core/controls/button-codes.h>
#include <cheryl/core/controls/tick-input.h>

#include <array>
#include <utility>

#include "actor.h"

struct ActorActions {
    static constexpr CE::Input::Actions::Id move{1};
    static constexpr CE::Input::Actions::Id left{2};
    static constexpr CE::Input::Actions::Id right{3};
    static constexpr CE::Input::Actions::Id fire{4};
};

class PlayerController final : public ActorController {
    CE::Input::Actions::Context controls;

public:
    PlayerController() = default;

    [[nodiscard]] static CE::Input::Actions::ContextDefinition defined_controls(const CE::Input::System& system) {
        using namespace CE::Input;
        // GLFW reads stored IDs here; wrong-thread access does not inherently throw.
        const auto keyboard = system.keyboard_id();
        const auto gamepad = system.gamepad_id();
        const std::array controls_array{
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
            .modifiers = {}, .control = {gamepad, GamepadControl::LeftStickX},.action = ActorActions::move, .options = {}
        });
        for (const auto& [control, action] : controls_array) {
            definition.buttons.push_back(Actions::ContextButton{
                .chord = {{control}}, .action = action
            });
        }
        return definition;
    }

    // These stubs do not move an actor or create a shot, even when input is active.
    void move(const double delta, const float horizontal) override {
        if (actor) {
            actor->move(delta, horizontal);
        }
    };
    void fire() override {
        //actor->position;
        // todo: dispatch a player fire event
    };
    void update(const CE::Game::TickContext& tick) override {
        const auto actions_view = tick.input.actions(controls);
        const float horizontal = actions_view.axis(ActorActions::move).current
            + (actions_view.button(ActorActions::left).held() ? -1.0f : 0.0f)
            + (actions_view.button(ActorActions::right).held() ? 1.0f : 0.0f);
        move(tick.delta_seconds, horizontal);
        if (actions_view.button(ActorActions::fire).pressed()) {
            fire();
        }
    }

    // TODO (bindings): Remove bound_actions and its constructor initializer;
    // controls (CE::Input::Actions::Context) can default-construct empty.
    // In init_platform(), get CE::Input::Actions::ContextSubmission from
    // engine.input().action_contexts(), attach defined_controls(engine.input())
    // (CE::Input::Actions::ContextDefinition), storing the returned Context in controls.
    // Submission/definition can be local; remove prebinding from SpaceInvaders.
    void init_platform(CE::Runtime::Context& engine) override {
        auto& input = engine.input();
        controls = input.action_contexts().attach(defined_controls(input));
        // First acquisition writes mapper state without a lock; overlap with polling
        // is a data race. Normal startup polling starts after this hook, so it may work here.
        // attach() is synchronized; it queues mappings for the next platform poll.
    }

    void init_simulation() override {
        // TODO: Set up the Actor* link on simulation before movement/fire use it.
    }

    void deinit_platform() override {
        // TODO: Reset controls (CE::Input::Actions::Context) while engine input still exists.
    }

    void deinit_simulation() override {
        // TODO: Clear simulation-side actor state before this owner exits.
    }
};
