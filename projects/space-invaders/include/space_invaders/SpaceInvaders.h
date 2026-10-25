#pragma once
#include <cheryl/core/engine/engine-context.h>
#include <cheryl/core/game-framework/abstract-game.h>
#include <cheryl/core/game-framework/tick-context.h>
#include <cheryl/core/rendering/camera.h>
#include <cheryl/core/rendering/render-frame.h>

#include <array>
#include <filesystem>
#include <memory>
#include <utility>

#include <space_invaders/controllers/invasion.h>
#include <space_invaders/controllers/player.h>
#include <space_invaders/controllers/ui.h>
#include <space_invaders/game-objects/abstract-game-object.h>

class SpaceInvaders final : public CE::Game::AbstractGame {
    CE::Runtime::Context& engine_context;
    CE::Rendering::Camera2D camera;
    std::filesystem::path asset_root;
    struct CtrID {
        enum {
            ui = 0,
            player,
            invasion
        };
    };
    std::array<std::unique_ptr<AbstractGameObject>, 3> controllers; //ui, player, invasion
    std::unique_ptr<PlayerActor> player;
    Invasion invasion;


protected:
    void load_score();
    void save_score();
    void increase_score(int delta);

    void player_death();
    void game_over();

public:
    explicit SpaceInvaders(CE::Runtime::Context& engine, std::filesystem::path  asset_root)
    : engine_context(engine), asset_root(std::move(asset_root)) {
        auto TheUI = std::make_unique<UIController>(engine_context);
        auto ThePlayer = std::make_unique<PlayerController>();
        auto TheInvasion = std::make_unique<InvasionController>();
        controllers[CtrID::ui] = std::move(TheUI);
        controllers[CtrID::player] = std::move(ThePlayer);
        controllers[CtrID::invasion] = std::move(TheInvasion);
    }

    void init_platform() override;
    void init_simulation() override;
    void update(const CE::Game::TickContext& tick) override;
    void prepare_render_frame(CE::Rendering::FrameWriter&) const override;

    void deinit_platform() override {
        // TODO: Forward object deinit_platform() after work settles, then release objects.
    }
    void deinit_simulation() override {
        // TODO: Forward object deinit_simulation(); retain resources still used by accepted work.
    }
};
