#include <space_invaders/gui/GameUI.h>

#include <stdexcept>

GameUI::GameUI(CE::Runtime::Context& engine) : engine(engine) {}

void GameUI::init_platform(CE::Runtime::Context&) {
    platform.emplace(engine.platform_dispatcher().submission());
    uploader.emplace(engine.resources());
}

void GameUI::init_simulation() {
    if (!platform || !uploader) {
        throw std::logic_error("Initialize GameUI platform before simulation");
    }
    if (!session) {
        session = std::make_unique<CE::UI::TGUI::Session>(
            engine, 2, CE::UI::TGUI::SessionOptions{.automatic_view = true}
        );
        hud = std::make_unique<HUD>(*session);
    }
}

void GameUI::process_input(const CE::Game::TickContext& tick) const {
    if (session) session->handle_input(tick.input.records(), false);
}

void GameUI::set_score(const int value) { score = value; }

void GameUI::update(const CE::Game::TickContext& tick) {
    if (!hud) return;
    session->update_time(tick.delta_seconds);
    hud->set_score(score);
    hud->update(tick);

    CE::UI::TGUI::adopt_scene(pending, scene);
    if (!pending.valid()) {
        pending = uploader->submit(*platform, session->record());
    }
}

void GameUI::append_passes(CE::Rendering::FrameWriter& frame) const { scene.write(frame); }

void GameUI::deinit_simulation() {
    // Retain session/upload dependencies until accepted platform work settles.
}

void GameUI::deinit_platform() {
    pending = {};
    scene = {};
    hud.reset();
    if (session) session->close_after_quiescence();
    session.reset();
    uploader.reset();
    platform.reset();
}
