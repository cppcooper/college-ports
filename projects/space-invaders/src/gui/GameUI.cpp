#include <space_invaders/gui/GameUI.h>

#include <stdexcept>
#include <utility>

GameUI::GameUI(
    CE::Runtime::Context& engine, const std::filesystem::path& asset_root, std::filesystem::path high_score_file
)
: engine(engine), menu(engine, asset_root, std::move(high_score_file)) {}

void GameUI::init_platform(CE::Runtime::Context&) {
    platform.emplace(engine.platform_dispatcher().submission());
    uploader.emplace(engine.resources());
    menu.init_platform(engine);
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
        game_over_label = tgui::Label::create("Game Over");
        game_over_label->setTextSize(48);
        game_over_label->setAutoSize(true);
        game_over_label->setIgnoreMouseEvents(true);
        game_over_label->setVisible(false);
        session->gui().add(game_over_label);
        session->set_layout(game_over_label, {.anchor = {0.5f, 0.5f}});
    }
    menu.init_simulation();
}

bool GameUI::process_input(const CE::Game::TickContext& tick) {
    if (session) session->handle_input(tick.input.records(), false);
    return menu.process_input(tick);
}

void GameUI::set_score(const int value) {
    menu.set_score(value);
    if (hud) hud->set_score(value);
}

void GameUI::set_lives(const int value) {
    if (hud) hud->set_lives(value);
}

void GameUI::set_game_over(const bool value) {
    if (game_over_label) game_over_label->setVisible(value);
}

void GameUI::reset_run(const int score, const int lives) {
    menu.reset_run();
    set_score(score);
    set_lives(lives);
    set_game_over(false);
}

void GameUI::update(const CE::Game::TickContext& tick) {
    if (!hud) return;
    session->update_time(tick.delta_seconds);

    CE::UI::TGUI::adopt_scene(pending, scene);
    if (!pending.valid()) {
        pending = uploader->submit(*platform, session->record());
    }
    menu.update(tick);
}

void GameUI::append_passes(CE::Rendering::FrameWriter& frame) const {
    scene.write(frame);
    menu.append_passes(frame);
}

void GameUI::deinit_simulation() {
    menu.deinit_simulation();
    // Retain session/upload dependencies until accepted platform work settles.
}

void GameUI::deinit_platform() {
    menu.deinit_platform();
    pending = {};
    scene = {};
    game_over_label.reset();
    hud.reset();
    if (session) session->close_after_quiescence();
    session.reset();
    uploader.reset();
    platform.reset();
}
