#ifndef GAME_H_
#define GAME_H_

#include <cheryl.h>
#include <cheryl/backends/opengl/startup.h>
#include <cheryl/core/controls/input-interface.h>
#include <cheryl/core/game-framework/abstract-game.h>
#include <cheryl/core/game-framework/game-runtime.h>
#include <gainput/gainput.h>
#include <space_invaders/objects/Play.h>
#include <space_invaders/systems/BulletManager.h>
#include <space_invaders/systems/invasion_mgr.h>
#include <space_invaders/systems/screen_mgr.h>

class SpaceInvaders final : public CE::Game::AbstractGame {
    CE::Runtime::Context& engine_context;

protected:
    void load_score();
    void save_score();
    void increase_score(int delta);

    void player_death();
    void game_over();

public:
    explicit SpaceInvaders(CE::Runtime::Context& engine) : engine_context(engine) {}
    void init_simulation();
    void init() override;
    void update(const CE::Game::TickContext& tick) override;
    void prepare_render_frame(CE::Rendering::FrameWriter&) const override {}
    void deinit() override {}
};

class OldSpaceInvaders final : public CE::Game::AbstractGame {
    // glEngine& m_engine = glEngine::Instance();
    // Asset_Faculties& m_asset_faculties = Asset_Faculties::Instance();
    //double timeslice = 0.015;
    // std::atomic<bool> needsDraw = false;
    // std::atomic<bool> Updating = true;
    // std::atomic<bool> Drawing = false;
    // GameAssets::Texture* m_fontTex = nullptr;
    // GameAssets::Font* m_font = nullptr;
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

public:
    // OldSpaceInvaders::OldSpaceInvaders(
    //     CE::Runtime::Context& engine,
    //     std::filesystem::path asset_root,
    //     const bool load_all_assets,
    //     CE::Text::FontSelection fonts,
    //     CE::Text::LayoutOptions text_options,
    //     const bool unicode_preview,
    //     const bool text_runs
    // )
    // : game_state_(std::make_unique<GameState>(
    //     engine, std::move(asset_root), load_all_assets, std::move(fonts), std::move(text_options), unicode_preview, text_runs
    // )) {}

protected:
    void Load_Score();
    void Save_Score();

public:
    void init() override;
    void deinit() override;
    void update(const CE::Game::TickContext& tick) override;
    void prepare_render_frame(CE::Rendering::FrameWriter& frame) const override;
    // void DoInput(int& key, int& scancode, int& action, int& mods);
    // void DoMouse(int& button, int& action, int& mod);
    // void DoCursor(double& x, double& y);

    // void Run();
    // void Init();
    // void DeInit();
    // void Update(double& seconds);
    // void Draw();

    void Increase_Score(int delta);
    void Player_Death();
    void Game_Over();
};

#endif
