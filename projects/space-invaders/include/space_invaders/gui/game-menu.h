#pragma once

#include <cheryl/core/subsystems/event-bus.h>
#include <cheryl/ui/rmlui/scene.h>
#include <cheryl/ui/rmlui/session.h>

#include <filesystem>
#include <future>
#include <memory>
#include <optional>
#include <vector>

#include <space_invaders/events.h>
#include <space_invaders/game-objects/abstract-game-object.h>
#include <space_invaders/high-scores.h>
#include <space_invaders/interfaces/frame-renderable.h>

namespace Rml { class ElementDocument; class EventListener; class Event; }

class GameMenu final : public AbstractGameObject, public FrameRenderable {
public:
    enum class Page { Hidden, Pause, HighScores, ScoreEntry };

private:
    CE::Runtime::Context& engine;
    std::filesystem::path document_file;
    HighScores high_scores;
    CE::Events::EventBus::Publisher events;
    CE::Events::EventBus::Registration pause_toggle_listener;
    CE::Events::EventBus::Registration menu_action_listener;
    CE::Events::EventBus::Registration game_over_listener;
    std::optional<CE::Runtime::PlatformDispatcher::Submission> platform;
    std::optional<CE::UI::RmlUi::SceneUploader> uploader;
    std::unique_ptr<CE::UI::RmlUi::Session> session;
    Rml::ElementDocument* document = nullptr;
    std::unique_ptr<Rml::EventListener> listener;
    std::future<CE::UI::RmlUi::Scene> pending;
    CE::UI::RmlUi::Scene scene;
    Page current_page = Page::Hidden;
    int current_score = 0;
    bool score_resolved = false;
    bool quit_after_score = false;
    struct Request {
        bool resume = false;
        SpaceInvadersEvents::MenuActionRequested action{};
    };
    std::vector<Request> requests;

    void show(Page page, const char* focus);
    void process_event(Rml::Event& event);
    void toggle_pause();
    void finish_run(bool quitting);
    void handle_menu_action(const SpaceInvadersEvents::MenuActionRequested& request);
    void unregister_listeners();

public:
    GameMenu(CE::Runtime::Context& engine, const std::filesystem::path& asset_root, std::filesystem::path high_score_file);
    ~GameMenu() override;
    void init_platform(CE::Runtime::Context&) override;
    void init_simulation() override;
    void deinit_simulation() override;
    void deinit_platform() override;
    void update(const CE::Game::TickContext& tick) override;
    void append_passes(CE::Rendering::FrameWriter& frame) const override;

    // Dispatch UI actions only after the complete input batch has drained.
    bool process_input(const CE::Game::TickContext& tick);
    [[nodiscard]] Page page() const { return current_page; }
    void set_score(int value) { current_score = value; }
    void show_pause();
    void show_scores(const HighScores::Table& scores);
    void show_score_entry(int score, bool quit_after_save);
    void show_error(const std::string& error);
    void hide();
};
