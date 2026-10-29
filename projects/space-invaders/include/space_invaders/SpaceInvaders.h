#pragma once

#include <cheryl/core/engine/engine-context.h>
#include <cheryl/core/game-framework/abstract-game.h>
#include <cheryl/core/rendering/camera.h>
#include <cheryl/core/subsystems/event-bus.h>
#include <cheryl/templates/observed-variables.h>

#include <filesystem>
#include <memory>
#include <utility>

#include <space_invaders/controllers/invasion.h>
#include <space_invaders/controllers/player.h>
#include <space_invaders/controllers/ui.h>
#include <space_invaders/events.h>
#include <space_invaders/gui/GameUI.h>
#include <space_invaders/systems/bullet-mgr.h>
#include <space_invaders/systems/invasion.h>

class SpaceInvaders final : public CE::Game::AbstractGame {
    CE::Runtime::Context& engine_context;
    std::filesystem::path asset_root;

    std::unique_ptr<PlayerActor> player;
    CE::Rendering::Camera2D camera;
    BulletManager bullets;
    Invasion invasion;
    GameUI ui;

    UIController ui_controller;
    PlayerController player_controller;
    InvasionController invasion_controller;

    CE::State::ObservedVariable<int> score{
        0, {[this](const int& value) { ui.set_score(value); }}
    };
    CE::State::ObservedVariable<int> lives{
        3, {[this](const int& value) { ui.set_lives(value); }}
    };
    CE::Events::EventBus::Registration enemy_killed_listener;
    CE::Events::EventBus::Registration player_hit_listener;
    CE::Events::EventBus::Registration player_death_animation_finished_listener;
    CE::Events::EventBus::Registration game_over_listener;
    bool is_game_over = false;

    void unregister_listeners();

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
    ~SpaceInvaders() override { unregister_listeners(); }

    void init_platform() override;
    void init_simulation() override;
    void prepare_render_frame(CE::Rendering::FrameWriter& frame) const override;
    void update(const CE::Game::TickContext& tick) override {
        ui_controller.update(tick);
        if (!is_game_over && player) player->update(tick);
        if (!is_game_over) invasion.update(tick);
        if (!is_game_over) {
            player_controller.update(tick);
            invasion_controller.update(tick);
        }
        if (!is_game_over) {
            bullets.update(tick);
            bullets.detect_collisions(player.get(), &invasion);
        }
        ui.update(tick);
    }
    void deinit_platform() override {
        unregister_listeners();
        invasion_controller.deinit_platform();
        player_controller.deinit_platform();
        ui_controller.deinit_platform();
        ui.deinit_platform();
        bullets.deinit_platform();
        invasion.deinit_platform();
    }
    void deinit_simulation() override {
        unregister_listeners();
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
