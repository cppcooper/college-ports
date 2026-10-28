#pragma once

#include <cheryl/core/engine/engine-context.h>
#include <cheryl/core/game-framework/abstract-game.h>
#include <cheryl/core/rendering/camera.h>

#include <filesystem>
#include <memory>
#include <utility>

#include <space_invaders/controllers/invasion.h>
#include <space_invaders/controllers/player.h>
#include <space_invaders/controllers/ui.h>
#include <space_invaders/gui/GameUI.h>
#include <space_invaders/systems/bullet-mgr.h>
#include <space_invaders/systems/invasion.h>

class SpaceInvaders final : public CE::Game::AbstractGame {
    CE::Runtime::Context& engine_context;
    CE::Rendering::Camera2D camera;
    std::filesystem::path asset_root;

    std::unique_ptr<PlayerActor> player;
    Invasion invasion;
    BulletManager bullets;
    GameUI ui;

    UIController ui_controller;
    PlayerController player_controller;
    InvasionController invasion_controller;

protected:
    void load_score();
    void save_score();
    void increase_score(int delta);
    void player_death();
    void game_over();

public:
    explicit SpaceInvaders(CE::Runtime::Context& engine, std::filesystem::path asset_root)
    : engine_context(engine), asset_root(std::move(asset_root)),
      bullets(player_controller, [this](const int count) { invasion_controller.onBulletCountChanged(count); }),
      ui(engine), ui_controller(ui), invasion_controller(invasion) {}
    ~SpaceInvaders() override = default;

    void init_platform() override;
    void init_simulation() override;
    void prepare_render_frame(CE::Rendering::FrameWriter& frame) const override;
    void update(const CE::Game::TickContext& tick) override {
        ui_controller.update(tick);
        player_controller.update(tick);
        invasion_controller.update(tick);
        if (player) player->update(tick);
        invasion.update(tick);
        bullets.update(tick);
        bullets.detect_collisions(player.get(), &invasion);
        ui.update(tick);
    }
    void deinit_platform() override {
        invasion_controller.deinit_platform();
        player_controller.deinit_platform();
        ui_controller.deinit_platform();
        ui.deinit_platform();
        bullets.deinit_platform();
        invasion.deinit_platform();
    }
    void deinit_simulation() override {
        ui_controller.deinit_simulation();
        player_controller.deinit_simulation();
        invasion_controller.deinit_simulation();
        bullets.deinit_simulation();
        invasion.deinit_simulation();
        if (player) player->deinit_simulation();
        player.reset();
        ui.deinit_simulation();
    }
};
