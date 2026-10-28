#include <space_invaders/SpaceInvaders.h>

#include <cheryl/core/display/window-interface.h>
#include <cheryl/core/game-framework/tick-context.h>
#include <cheryl/core/rendering/render-frame.h>
#include <cheryl/core/resources/asset-management/asset-loader.h>
#include <cheryl/text/font-service.h>

#include <space_invaders/game-objects/actors.h>

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
    const auto size = camera.framebuffer_size();
    player = std::make_unique<PlayerActor>(
        engine_context.events().publisher(),
        glm::vec2{size.width * 0.5f, PlayerActor::spawn_height},
        static_cast<float>(size.width)
    );

    player->init_simulation();
    invasion.init_simulation();
    bullets.init_simulation();
    ui.init_simulation();
    ui.set_score(score.get());
    ui.set_lives(lives.get());

    player_controller.attach(*player);

    ui_controller.init_simulation();
    player_controller.init_simulation();
    invasion_controller.init_simulation();
}

void SpaceInvaders::prepare_render_frame(CE::Rendering::FrameWriter& frame) const {
    {
        auto pass = frame.begin_pass(camera.projection_matrix(), camera.view_matrix());
        if (player) player->append_draws(pass);
        invasion.append_draws(pass);
        bullets.append_draws(pass);
    }
    ui.append_passes(frame);
}
