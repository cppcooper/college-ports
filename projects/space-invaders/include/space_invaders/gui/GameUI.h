#pragma once

#include <cheryl/core/engine/engine-context.h>
#include <cheryl/core/engine/platform-dispatcher.h>
#include <cheryl/core/game-framework/tick-context.h>
#include <cheryl/core/rendering/render-frame.h>
#include <cheryl/ui/tgui/scene.h>
#include <cheryl/ui/tgui/session.h>

#include <future>
#include <memory>
#include <optional>

#include <space_invaders/gui/HUD.h>

class GameUI final {
    CE::Runtime::Context& engine;
    std::optional<CE::Runtime::PlatformDispatcher::Submission> platform;
    std::optional<CE::UI::TGUI::SceneUploader> uploader;
    std::unique_ptr<CE::UI::TGUI::Session> session;
    std::unique_ptr<HUD> hud;
    std::future<CE::UI::TGUI::Scene> pending;
    CE::UI::TGUI::Scene scene;
    int score = 0;

public:
    explicit GameUI(CE::Runtime::Context& engine);
    GameUI(const GameUI&) = delete;
    GameUI& operator=(const GameUI&) = delete;

    void init_platform();
    void init_simulation();
    void deinit_simulation();
    void deinit_platform();

    void process_input(const CE::Game::TickContext& tick) const;
    void set_score(int value);
    void update(const CE::Game::TickContext& tick);
    void append_passes(CE::Rendering::FrameWriter& frame) const;
};
