#include <space_invaders/SpaceInvaders.h>

#include <cheryl/core/display/window-interface.h>
#include <cheryl/core/game-framework/tick-context.h>
#include <cheryl/core/rendering/render-frame.h>
#include <cheryl/core/resources/asset-management/asset-loader.h>
#include <cheryl/text/font-service.h>

#include <space_invaders/game-objects/actors.h>

void SpaceInvaders::increase_score(const int delta) {
    score.set(score.get() + delta);
}

void SpaceInvaders::player_death() {
    if (is_game_over) return;
    const int remaining = lives.get();
    if (remaining > 0) lives.set(remaining - 1);
}

void SpaceInvaders::game_over() {
    if (is_game_over) return;
    is_game_over = true;
    ui.set_game_over(true);
}

void SpaceInvaders::unregister_listeners() {
    enemy_killed_listener.disconnect();
    enemy_killed_listener = {};
    player_hit_listener.disconnect();
    player_hit_listener = {};
    player_death_animation_finished_listener.disconnect();
    player_death_animation_finished_listener = {};
    game_over_listener.disconnect();
    game_over_listener = {};
}

void SpaceInvaders::init_platform() {
    CE::Assets::Loading::Loader loader(asset_root);
    loader.load_assets(engine_context.resources());

    CE::Text::FontSelection font_selection;
    font_selection.automatic_system_fonts = true;
    font_selection.asset_directories = {asset_root};
    font_selection.preferred = {
        "ProggyVector", "Cantarell", "FreeSans", "Adwaita Mono", "FreeSerif", "DejaVu Sans"
    };
    engine_context.fonts().configure(font_selection);
    camera.set_framebuffer_size(engine_context.window().framebuffer_size());

    invasion.init_platform(engine_context);
    bullets.init_platform(engine_context);
    ui.init_platform(engine_context);
    ui_controller.init_platform(engine_context);
    player_controller.init_platform(engine_context);
    invasion_controller.init_platform(engine_context);
}

void SpaceInvaders::init_simulation() {
    unregister_listeners();
    is_game_over = false;
    const auto size = camera.framebuffer_size();
    player = std::make_unique<PlayerActor>(
        engine_context.events().publisher(),
        glm::vec2{size.width * 0.5f, PlayerActor::spawn_height},
        static_cast<float>(size.width)
    );

    player->init_simulation();
    invasion.init_simulation();
    ufo = std::make_unique<EnemyActor>(
        EnemyType::WOODIE,
        glm::vec2{size.width * 0.5f, size.height - 45.0f},
        static_cast<int>(Invasion::formation_size),
        engine_context.events().publisher()
    );
    ufo->init_simulation();
    bullets.init_simulation();
    ui.init_simulation();
    ui.set_score(score.get());
    ui.set_lives(lives.get());
    ui.set_game_over(false);

    player_controller.attach(*player);

    ui_controller.init_simulation();
    player_controller.init_simulation();
    invasion_controller.init_simulation();

    enemy_killed_listener = engine_context.events().register_listener(
        SpaceInvadersEvents::enemy_killed_event,
        [this](const SpaceInvadersEvents::EnemyKilled& enemy) {
            increase_score(enemy.points);
        }
    );
    player_hit_listener = engine_context.events().register_listener(
        SpaceInvadersEvents::player_hit_event,
        [this](const SpaceInvadersEvents::PlayerHit&) {
            invasion_controller.onPlayerHit();
            bullets.onPlayerHit();
            player_death();
        }
    );
    player_death_animation_finished_listener = engine_context.events().register_listener(
        SpaceInvadersEvents::player_death_animation_finished_event,
        [this](const SpaceInvadersEvents::PlayerDeathAnimationFinished&) {
            if (is_game_over || !player || player->is_alive()) return;
            if (lives.get() > 0) {
                player->respawn();
            } else {
                engine_context.events().notify(
                    SpaceInvadersEvents::game_over_event,
                    SpaceInvadersEvents::GameOver{}
                );
            }
        }
    );
    game_over_listener = engine_context.events().register_listener(
        SpaceInvadersEvents::game_over_event,
        [this](const SpaceInvadersEvents::GameOver&) {
            game_over();
        }
    );
}

void SpaceInvaders::prepare_render_frame(CE::Rendering::FrameWriter& frame) const {
    {
        auto pass = frame.begin_pass(camera.projection_matrix(), camera.view_matrix());
        if (player && !is_game_over) player->append_draws(pass);
        invasion.append_draws(pass);
        if (ufo) ufo->append_draws(pass);
        bullets.append_draws(pass);
    }
    ui.append_passes(frame);
}
