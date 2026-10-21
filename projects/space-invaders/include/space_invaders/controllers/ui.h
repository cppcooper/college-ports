#pragma once
#include <cheryl/core/engine/engine-context.h>
#include <cheryl/ui/tgui/scene.h>
#include <cheryl/ui/rmlui/scene.h>
#include <cheryl/ui/tgui/session.h>
#include <cheryl/ui/rmlui/session.h>

#include <future>
#include <memory>
#include <optional>

#include <space_invaders/HUD.h>
#include <space_invaders/game-objects/abstract-game-object.h>

class UIController : public AbstractGameObject {
    static constexpr CE::Input::Actions::Id Menu{5};

    CE::Runtime::Context& engine;
    CE::Input::Actions::Context controls;
    std::optional<CE::Runtime::PlatformDispatcher::Submission> platform;

    // TGUI
    std::unique_ptr<CE::UI::TGUI::Session> tgui_session;
    std::optional<CE::UI::TGUI::SceneUploader> tgui_uploader;
    std::future<CE::UI::TGUI::Scene> tgui_pending;
    CE::UI::TGUI::Scene tgui_scene;
    std::unique_ptr<HUD> hud;

    // RmlUi
    std::unique_ptr<CE::UI::RmlUi::Session> rmlui_session;
    std::optional<CE::UI::TGUI::SceneUploader> rmlui_uploader;
    std::future<CE::UI::RmlUi::Scene> rmlui_pending;
    CE::UI::TGUI::Scene rmlui_scene;
    // todo: menu class

    CE::Math::BitArray<2> initialized;

public:
    explicit UIController(CE::Runtime::Context& engine)
    : engine(engine) {
        initialized.words()[0] = 0; //probably redundant
    }

    void init_platform(CE::Runtime::Context& engine_context) override {
        auto& input = engine.input();
        controls = input.action_contexts().attach(defined_controls(input));
        platform.emplace(engine.platform_dispatcher().submission());
        tgui_uploader.emplace(engine.resources());
        // TODO: Use CE::UI::RmlUi::SceneUploader/Scene for the menu; initialize its uploader here.
        initialized[0] = true;
    }

    void init_simulation() override {
        // TODO: Create CE::UI::RmlUi::Session/menu with a distinct routing target;
        // wire visibility, focus, Escape closure and pause/resume.
        if (!tgui_session) {
            tgui_session = std::make_unique<CE::UI::TGUI::Session>(
                engine, 2, CE::UI::TGUI::SessionOptions{.automatic_view = true}
            );
            hud = std::make_unique<HUD>(*tgui_session);
        }
        initialized[1] = true;
    }

    // Only the TGUI scene is appended; an RmlUi menu will need a later pass.
    void append(CE::Rendering::FrameWriter& frame) const { tgui_scene.write(frame); }

    void update(const CE::Game::TickContext& tick) override {
        if (initialized.words()[0] != 3) {
            return;
        }
        if (rmlui_session) {
            // Exclusive UI focus gates semantic Escape; menu closure must handle raw/key events.
            if (tick.input.actions(controls).button(Menu).pressed()) {
                // todo: have codex develop a menu with options={resume, highscore, exit} + highscore screen
            }
        }

        if (tgui_session) {
            tgui_session->handle_input(tick.input.records(), false);
            tgui_session->update_time(tick.delta_seconds);

            hud->update(tick);

            CE::UI::TGUI::adopt_scene(tgui_pending, tgui_scene);
            if (!tgui_pending.valid()) {
                tgui_pending = tgui_uploader->submit(platform.value(), tgui_session->record());
            }
        }
    }

    void deinit_platform() override {
        hud.reset(); // Release widget references before closing the shared session.
        if (tgui_session) tgui_session->close_after_quiescence();
        // TODO: Release menu document/listener references before closing its session.
        if (rmlui_session) rmlui_session->close_after_quiescence();
        // TODO: Clear scenes/uploaders after work settles.
    }

    void deinit_simulation() override {
        // TODO: Release menu focus on simulation; retain upload dependencies for platform cleanup.
    }

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
};

