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

#include <TGUI/Widgets/Label.hpp>

#include <space_invaders/game-objects/abstract-game-object.h>
#include <space_invaders/interfaces/frame-renderable.h>
#include <space_invaders/gui/HUD.h>
#include <space_invaders/gui/game-menu.h>

class GameUI final : public AbstractGameObject, public FrameRenderable {
    CE::Runtime::Context& engine;
    GameMenu menu;
    std::optional<CE::Runtime::PlatformDispatcher::Submission> platform;
    std::optional<CE::UI::TGUI::SceneUploader> uploader;
    std::unique_ptr<CE::UI::TGUI::Session> session;
    std::unique_ptr<HUD> hud;
    tgui::Label::Ptr game_over_label;
    std::future<CE::UI::TGUI::Scene> pending;
    CE::UI::TGUI::Scene scene;

public:
    GameUI(CE::Runtime::Context& engine, const std::filesystem::path& asset_root);
    ~GameUI() override = default;
    GameUI(const GameUI&) = delete;
    GameUI& operator=(const GameUI&) = delete;

    void init_platform(CE::Runtime::Context& engine) override;
    void init_simulation() override;
    void deinit_simulation() override;
    void deinit_platform() override;

    bool process_input(const CE::Game::TickContext& tick);
    [[nodiscard]] GameMenu& game_menu() { return menu; }
    void set_score(int value);
    void set_lives(int value);
    void set_game_over(bool value);
    void update(const CE::Game::TickContext& tick) override;
    void append_passes(CE::Rendering::FrameWriter& frame) const override;
};
