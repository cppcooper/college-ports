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
    CE::Events::EventBus::Publisher events;
    std::optional<CE::Runtime::PlatformDispatcher::Submission> platform;
    std::optional<CE::UI::RmlUi::SceneUploader> uploader;
    std::unique_ptr<CE::UI::RmlUi::Session> session;
    Rml::ElementDocument* document = nullptr;
    std::unique_ptr<Rml::EventListener> listener;
    std::future<CE::UI::RmlUi::Scene> pending;
    CE::UI::RmlUi::Scene scene;
    Page current_page = Page::Hidden;
    struct Request {
        bool resume = false;
        SpaceInvadersEvents::MenuActionRequested action{};
    };
    std::vector<Request> requests;

    void show(Page page, const char* focus);
    void process_event(Rml::Event& event);

public:
    GameMenu(CE::Runtime::Context& engine, const std::filesystem::path& asset_root);
    ~GameMenu() override;
    void init_platform(CE::Runtime::Context&) override;
    void init_simulation() override;
    void deinit_platform() override;
    void update(const CE::Game::TickContext& tick) override;
    void append_passes(CE::Rendering::FrameWriter& frame) const override;

    // Dispatch UI actions only after the complete input batch has drained.
    bool process_input(const CE::Game::TickContext& tick);
    [[nodiscard]] Page page() const { return current_page; }
    void show_pause();
    void show_scores(const HighScores::Table& scores);
    void show_score_entry(int score, bool quit_after_save);
    void show_error(const std::string& error);
    void hide();
};
