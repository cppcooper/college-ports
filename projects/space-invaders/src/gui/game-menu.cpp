#include <space_invaders/gui/game-menu.h>

#include <cheryl/core/engine/engine-context.h>
#include <cheryl/core/game-framework/tick-context.h>

#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementFormControlInput.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/StringUtilities.h>

#include <array>
#include <format>
#include <functional>
#include <stdexcept>
#include <utility>

namespace {
    class Listener final : public Rml::EventListener {
        std::function<void(Rml::Event&)> callback;
    public:
        explicit Listener(std::function<void(Rml::Event&)> callback) : callback(std::move(callback)) {}
        void ProcessEvent(Rml::Event& event) override { callback(event); }
    };

    Rml::Element& element(Rml::ElementDocument& document, const char* id) {
        auto* found = document.GetElementById(id);
        if (!found)
            throw std::runtime_error("The game menu is missing element: " + std::string(id));
        return *found;
    }

    Rml::ElementFormControlInput& name_field(Rml::ElementDocument& document) {
        auto* field = dynamic_cast<Rml::ElementFormControlInput*>(&element(document, "player-name"));
        if (!field)
            throw std::runtime_error("The high-score name field must be a text input.");
        return *field;
    }
}

GameMenu::GameMenu(
    CE::Runtime::Context& engine, const std::filesystem::path& asset_root, std::filesystem::path high_score_file
)
: engine(engine), document_file(asset_root / "graphics/ui/game-menu.rml"), high_scores(std::move(high_score_file)),
  events(engine.events().publisher()) {}

GameMenu::~GameMenu() { unregister_listeners(); }

void GameMenu::init_platform(CE::Runtime::Context&) {
    platform.emplace(engine.platform_dispatcher().submission());
    uploader.emplace(engine.resources());
}

void GameMenu::init_simulation() {
    if (!platform || !uploader)
        throw std::logic_error("Initialize the menu platform before simulation.");
    if (session)
        return;
    high_scores.load();
    current_score = 0;
    score_resolved = quit_after_score = false;
    session = std::make_unique<CE::UI::RmlUi::Session>(engine, 3, CE::UI::RmlUi::SessionOptions{.automatic_view = true});
    const auto source = document_file.u8string();
    document = session->context().LoadDocument(Rml::String(source.begin(), source.end()));
    if (!document)
        throw std::runtime_error("Cannot load the game menu: " + document_file.string());
    for (const auto* id : {"pause-page", "scores-page", "entry-page", "resume", "new-game", "high-scores", "quit", "back", "score-list", "entry-score", "entry-help", "menu-error"})
        static_cast<void>(element(*document, id));
    static_cast<void>(name_field(*document));
    listener = std::make_unique<Listener>([this](Rml::Event& event) { process_event(event); });
    document->AddEventListener("click", listener.get());
    document->AddEventListener("submit", listener.get());
    document->AddEventListener("change", listener.get());
    document->Hide();
    pause_toggle_listener = engine.events().register_listener(
        SpaceInvadersEvents::pause_toggle_event,
        [this](const SpaceInvadersEvents::PauseToggle&) { toggle_pause(); }
    );
    menu_action_listener = engine.events().register_listener(
        SpaceInvadersEvents::menu_action_event,
        [this](const SpaceInvadersEvents::MenuActionRequested& request) { handle_menu_action(request); }
    );
    game_over_listener = engine.events().register_listener(
        SpaceInvadersEvents::game_over_event,
        [this](const SpaceInvadersEvents::GameOver&) { finish_run(false); }
    );
}

void GameMenu::toggle_pause() {
    // Score entry must be saved or explicitly skipped before leaving this page.
    if (current_page == Page::ScoreEntry) return;
    const bool paused = current_page == Page::Hidden;
    events.notify(SpaceInvadersEvents::pause_requested_event, SpaceInvadersEvents::PauseRequested{paused});
    if (paused) show_pause();
    else hide();
}

void GameMenu::finish_run(const bool quitting) {
    quit_after_score = quitting;
    events.notify(SpaceInvadersEvents::pause_requested_event, SpaceInvadersEvents::PauseRequested{true});
    if (!score_resolved && high_scores.qualifies(current_score))
        show_score_entry(current_score, quitting);
    else if (quitting)
        events.notify(SpaceInvadersEvents::quit_requested_event, SpaceInvadersEvents::QuitRequested{});
    else
        show_scores(high_scores.entries());
}

void GameMenu::handle_menu_action(const SpaceInvadersEvents::MenuActionRequested& request) {
    using namespace SpaceInvadersEvents;
    switch (request.action) {
    case MenuAction::NewGame:
        if (current_page != Page::ScoreEntry)
            events.notify(new_game_requested_event, NewGameRequested{});
        break;
    case MenuAction::HighScores:
        if (current_page != Page::ScoreEntry) {
            events.notify(pause_requested_event, PauseRequested{true});
            show_scores(high_scores.entries());
        }
        break;
    case MenuAction::Back:
        if (current_page != Page::ScoreEntry) {
            events.notify(pause_requested_event, PauseRequested{true});
            show_pause();
        }
        break;
    case MenuAction::Quit:
        finish_run(true);
        break;
    case MenuAction::SubmitScore:
        if (current_page != Page::ScoreEntry || score_resolved) break;
        try {
            if (!high_scores.add(current_score, request.name))
                throw std::runtime_error("This score no longer qualifies for the top 10.");
            score_resolved = true;
            if (quit_after_score) events.notify(quit_requested_event, QuitRequested{});
            else show_scores(high_scores.entries());
        } catch (const std::exception& error) {
            show_error(error.what());
        }
        break;
    case MenuAction::SkipScore:
        if (current_page != Page::ScoreEntry) break;
        score_resolved = true;
        if (quit_after_score) events.notify(quit_requested_event, QuitRequested{});
        else show_scores(high_scores.entries());
        break;
    }
}

void GameMenu::unregister_listeners() {
    pause_toggle_listener.disconnect();
    pause_toggle_listener = {};
    menu_action_listener.disconnect();
    menu_action_listener = {};
    game_over_listener.disconnect();
    game_over_listener = {};
}

void GameMenu::deinit_simulation() { unregister_listeners(); }

void GameMenu::reset_run() {
    current_score = 0;
    score_resolved = quit_after_score = false;
    requests.clear();
    hide();
}

void GameMenu::show(const Page page, const char* focus) {
    if (!document)
        return;
    const std::array pages{
        std::pair{"pause-page", Page::Pause}, std::pair{"scores-page", Page::HighScores}, std::pair{"entry-page", Page::ScoreEntry}
    };
    for (const auto& [id, value] : pages)
        element(*document, id).SetProperty("display", value == page ? "block" : "none");
    current_page = page;
    show_error("");
    document->Show(Rml::ModalFlag::Modal, Rml::FocusFlag::None);
    // Ordinary menu hotkeys retain their existing action-context mappings.
    // Name entry captures keyboard/text exclusively while the world is paused.
    session->request_keyboard_focus(page == Page::ScoreEntry
        ? CE::Input::Routing::KeyboardPolicy::Exclusive
        : CE::Input::Routing::KeyboardPolicy::PassThrough);
    static_cast<void>(element(*document, focus).Focus(true));
}

void GameMenu::show_pause() { show(Page::Pause, "resume"); }

void GameMenu::show_scores(const HighScores::Table& scores) {
    if (!document)
        return;
    std::string rows;
    for (std::size_t rank = 0; rank < scores.size(); ++rank) {
        const auto& entry = scores[rank];
        rows += std::format(
            "<div class='score-row'><span class='rank'>{}</span><span class='name'>{}</span><span class='points'>{}</span></div>",
            rank + 1, entry.name.empty() ? "—" : Rml::StringUtilities::EncodeRml(entry.name), entry.score
        );
    }
    element(*document, "score-list").SetInnerRML(rows);
    show(Page::HighScores, "back");
}

void GameMenu::show_score_entry(const int score, const bool quit_after_save) {
    if (!document)
        return;
    element(*document, "entry-score").SetInnerRML(std::format("Score: {}", score));
    element(*document, "entry-help").SetInnerRML(quit_after_save
        ? "You made the top 10. Save your name before quitting."
        : "You made the top 10. Enter your name.");
    name_field(*document).SetValue("");
    element(*document, "save-score").SetAttribute("value", std::string{quit_after_save ? "Save and quit" : "Save score"});
    element(*document, "skip-score").SetAttribute("value", std::string{quit_after_save ? "Quit without saving" : "Skip"});
    show(Page::ScoreEntry, "player-name");
}

void GameMenu::show_error(const std::string& error) {
    if (!document)
        return;
    auto& label = element(*document, "menu-error");
    label.SetInnerRML(Rml::StringUtilities::EncodeRml(error));
    label.SetProperty("display", error.empty() ? "none" : "block");
}

void GameMenu::hide() {
    if (!document)
        return;
    document->Hide();
    session->release_keyboard_focus();
    current_page = Page::Hidden;
}

void GameMenu::process_event(Rml::Event& event) {
    using namespace SpaceInvadersEvents;
    const bool enter_in_name = event.GetType() == "change" && event.GetParameter<bool>("linebreak", false)
        && event.GetTargetElement() == &name_field(*document);
    if ((event.GetType() == "submit" || enter_in_name) && current_page == Page::ScoreEntry) {
        requests.push_back({false, {MenuAction::SubmitScore, name_field(*document).GetValue()}});
        event.StopPropagation();
        return;
    }
    if (event.GetType() != "click")
        return;
    for (auto* target = event.GetTargetElement(); target && target != document; target = target->GetParentNode()) {
        const auto action = target->GetAttribute<Rml::String>("data-action", "");
        if (action.empty())
            continue;
        if (action == "resume") requests.push_back({true, {}});
        else if (action == "new-game") requests.push_back({false, {MenuAction::NewGame, {}}});
        else if (action == "high-scores") requests.push_back({false, {MenuAction::HighScores, {}}});
        else if (action == "back") requests.push_back({false, {MenuAction::Back, {}}});
        else if (action == "quit") requests.push_back({false, {MenuAction::Quit, {}}});
        else if (action == "skip-score") requests.push_back({false, {MenuAction::SkipScore, {}}});
        return;
    }
}

bool GameMenu::process_input(const CE::Game::TickContext& tick) {
    if (!session)
        return false;
    session->handle_input(tick.input.records(), current_page != Page::Hidden);
    auto actions = std::exchange(requests, {});
    for (auto& request : actions) {
        if (request.resume)
            events.notify(SpaceInvadersEvents::pause_toggle_event, SpaceInvadersEvents::PauseToggle{});
        else
            events.notify(SpaceInvadersEvents::menu_action_event, std::move(request.action));
    }
    return !actions.empty();
}

void GameMenu::update(const CE::Game::TickContext& tick) {
    if (!session)
        return;
    session->update_time(tick.delta_seconds);
    CE::UI::RmlUi::adopt_scene(pending, scene);
    if (!pending.valid() && current_page != Page::Hidden)
        pending = uploader->submit(*platform, session->record());
}

void GameMenu::append_passes(CE::Rendering::FrameWriter& frame) const {
    if (current_page != Page::Hidden)
        scene.write(frame);
}

void GameMenu::deinit_platform() {
    unregister_listeners();
    pending = {};
    scene = {};
    requests.clear();
    if (document) {
        document->RemoveEventListener("click", listener.get());
        document->RemoveEventListener("submit", listener.get());
        document->RemoveEventListener("change", listener.get());
        document = nullptr;
    }
    listener.reset();
    if (session)
        session->close_after_quiescence();
    session.reset();
    uploader.reset();
    platform.reset();
    current_page = Page::Hidden;
}
