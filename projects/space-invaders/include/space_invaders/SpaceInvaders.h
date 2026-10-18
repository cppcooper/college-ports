#pragma once
#include <cheryl.h>
#include "systems/BulletManager.h"
#include "systems/invasion_mgr.h"
#include "systems/screen_mgr.h"
#include "hud.h"

class SpaceInvaders final : public CE::Game::AbstractGame {
    CE::Runtime::Context& engine_context;
    std::filesystem::path asset_root;
    CE::Rendering::Camera2D camera;
    std::vector<CE::Input::Actions::BindingId> binding_ids{};
    std::optional<CE::Text::Rendering::Generation> score;
    std::shared_ptr<const CE::Rendering::Resources::Material> images;
    std::shared_ptr<const CE::Rendering::Resources::Material> text;
    HUD hud;

protected:
    void load_score();
    void save_score();
    void increase_score(int delta);

    void player_death();
    void game_over();

public:
    explicit SpaceInvaders(CE::Runtime::Context& engine)
    : engine_context(engine), hud() {}

    void init_simulation();
    void init() override;
    void update(const CE::Game::TickContext& tick) override;
    void prepare_render_frame(CE::Rendering::FrameWriter&) const override {}
    void deinit() override {}
};

class OldSpaceInvaders final : public CE::Game::AbstractGame {
    struct GameState;
    std::unique_ptr<GameState> game_state_;

    bool moveL = false;
    bool moveR = false;
    bool shoots = false;

    std::mutex Var_Mutex;
    unsigned short player_lives{};
    unsigned int highscore{};
    unsigned int score{};

    player* ThePlayer = nullptr;
    bullet_manager bullet_mgr;
    Invasion_Formation invasion_mgr;
    Screen_Mgr screen;

protected:
    void Load_Score();
    void Save_Score();

public:
    void Increase_Score(int delta);
    void Player_Death();
    void Game_Over();
};
