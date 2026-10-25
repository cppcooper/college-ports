#pragma once

#include <cheryl/core/controls/action-context.h>
#include <cheryl/core/controls/button-codes.h>
#include <cheryl/core/controls/input-interface.h>
#include <cheryl/core/engine/engine-context.h>

#include <space_invaders/game-objects/abstract-game-object.h>
#include <space_invaders/gui/GameUI.h>

class UIController final : public AbstractGameObject {
    static constexpr CE::Input::Actions::Id Menu{5};

    GameUI& ui;
    CE::Input::Actions::Context controls;

public:
    explicit UIController(GameUI& ui) : ui(ui) {}

    [[nodiscard]] static CE::Input::Actions::ContextDefinition defined_controls(const CE::Input::System& system) {
        using namespace CE::Input;
        const auto keyboard = system.keyboard_id();
        const auto gamepad = system.gamepad_id();
        Actions::ContextDefinition definition;
        definition.buttons.push_back(Actions::ContextButton{
            .chord = {{{keyboard, KeyboardKey::Escape}}}, .action = Menu
        });
        definition.buttons.push_back(Actions::ContextButton{
            .chord = {{{gamepad, GamepadControl::Start}}}, .action = Menu
        });
        return definition;
    }
    void init_platform(CE::Runtime::Context& engine) override {
        auto& input = engine.input();
        controls = input.action_contexts().attach(defined_controls(input));
    }
    void deinit_platform() override { controls = {}; }
    void update(const CE::Game::TickContext& tick) override {
        ui.process_input(tick);
        // Menu visibility, focus, and pause/resume decisions remain unfinished.
    }
};
