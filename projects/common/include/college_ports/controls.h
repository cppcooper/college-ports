#pragma once

#include <cheryl/core/controls/action-context.h>
#include <cheryl/core/controls/button-codes.h>
#include <cheryl/core/controls/input-interface.h>

#include <initializer_list>
#include <utility>

namespace CollegePorts {
    inline CE::Input::Actions::ContextDefinition keyboard_controls(
        const CE::Input::System& input,
        std::initializer_list<std::pair<CE::Input::KeyboardKey, CE::Input::Actions::Id>> keys
    ) {
        CE::Input::Actions::ContextDefinition definition;
        for (const auto& [key, action] : keys)
            definition.buttons.push_back({.chord = {{{input.keyboard_id(), key}}}, .action = action});
        return definition;
    }
    inline void gamepad_controls(
        CE::Input::Actions::ContextDefinition& definition, const CE::Input::System& input,
        std::initializer_list<std::pair<CE::Input::GamepadControl, CE::Input::Actions::Id>> buttons
    ) {
        for (const auto& [button, action] : buttons)
            definition.buttons.push_back({.chord = {{{input.gamepad_id(), button}}}, .action = action});
    }
}
