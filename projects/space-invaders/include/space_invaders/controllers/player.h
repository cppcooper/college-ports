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

class PlayerController final : ActorController {
    CE::Input::Actions::Context controls;

public:
    explicit PlayerController(CE::Input::Actions::Context bound_actions)
    : controls(std::move(bound_actions)) {}

    [[nodiscard]] static CE::Input::Actions::ContextDefinition defined_controls(const CE::Input::System& system) {
        using namespace CE::Input;
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

    void move(float horizontal) override {};
    void fire() override {};
    void update(const CE::Game::TickContext& tick) override {
        const auto actions_view = tick.input.actions(controls);
        float horizontal = (actions_view.button(ActorActions::left).held() ? -1.0f : 0.0f)
            + (actions_view.button(ActorActions::right).held() ? 1.0f : 0.0f);
        horizontal += actions_view.axis(ActorActions::move).current;
        move(horizontal);
        if (actions_view.button(ActorActions::fire).pressed()) {
            fire();
        }
    };
};
