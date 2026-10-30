#pragma once

#include <cheryl/core/engine/engine-context.h>
#include <cheryl/core/game-framework/tick-context.h>
#include <cheryl/ui/tgui/scene.h>
#include <cheryl/ui/tgui/session.h>

#include <TGUI/Widgets/Label.hpp>

#include <future>
#include <memory>
#include <optional>

namespace CollegePorts {
    // Retain widget/upload dependencies until accepted platform work settles.
    class GameUI {
        CE::Runtime::Context& engine;
        std::optional<CE::Runtime::PlatformDispatcher::Submission> platform;
        std::optional<CE::UI::TGUI::SceneUploader> uploader;
        std::unique_ptr<CE::UI::TGUI::Session> session;
        std::future<CE::UI::TGUI::Scene> pending;
        CE::UI::TGUI::Scene scene;

    protected:
        virtual void create_widgets() = 0;
        virtual void release_widgets() = 0;
        CE::UI::TGUI::Session& ui() { return *session; }
        tgui::Label::Ptr label(const tgui::String& text, unsigned int size = 18) {
            auto widget = tgui::Label::create(text);
            widget->setAutoSize(true);
            widget->setTextSize(size);
            widget->setIgnoreMouseEvents(true);
            widget->getRenderer()->setTextColor(tgui::Color::White);
            session->gui().add(widget);
            return widget;
        }

    public:
        explicit GameUI(CE::Runtime::Context& engine) : engine(engine) {}
        virtual ~GameUI() = default;
        void init_platform() {
            platform.emplace(engine.platform_dispatcher().submission());
            uploader.emplace(engine.resources());
        }
        void init_simulation() {
            session = std::make_unique<CE::UI::TGUI::Session>(
                engine, 1, CE::UI::TGUI::SessionOptions{.automatic_view = true});
            create_widgets();
        }
        void update(const CE::Game::TickContext& tick) {
            session->update_time(tick.delta_seconds);
            CE::UI::TGUI::adopt_scene(pending, scene);
            if (!pending.valid()) pending = uploader->submit(*platform, session->record());
        }
        void handle_input(const CE::Game::TickContext& tick) {
            session->handle_input(tick.input.records(), true);
        }
        void append_passes(CE::Rendering::FrameWriter& frame) const { scene.write(frame); }
        void deinit_platform() {
            pending = {};
            scene = {};
            release_widgets();
            if (session) session->close_after_quiescence();
            session.reset();
            uploader.reset();
            platform.reset();
        }
    };
}
