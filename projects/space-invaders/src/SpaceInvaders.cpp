#include <space_invaders/SpaceInvaders.h>

#include <cheryl/assets/resources/resource-provider.h>
#include <cheryl/assets/resources/shader-asset-builder.h>
#include <cheryl/core/display/window-interface.h>
#include <cheryl/core/game-framework/tick-context.h>
#include <cheryl/core/rendering/render-frame.h>
#include <cheryl/core/resources/asset-management/asset-loader.h>
#include <cheryl/text/font-service.h>

#include <stdexcept>

#include <space_invaders/game-objects/actors.h>
#include <space_invaders/game-speed.h>

namespace {
    std::shared_ptr<const CE::Rendering::Resources::Material> make_enemy_material(CE::Rendering::Resources::Provider& provider) {
        auto* builder = provider.shader_asset_builder();
        if (!builder) throw std::logic_error("The enemy material requires a shader asset builder");

        using CE::Assets::Shaders::Stage;
        const CE::Assets::Shaders::ProgramRecipe program{
            "space-invaders:enemies",
            {
                {Stage::Vertex, "enemies.vert", R"GLSL(#version 330 core
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform float in_Scale;
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_Texcoord;
out vec2 v_texcoord;
void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position * in_Scale, 1.0);
    v_texcoord = in_Texcoord;
}
)GLSL"},
                {Stage::Fragment, "enemies.frag", R"GLSL(#version 330 core
uniform sampler2D mytexture;
uniform float in_Alpha;
uniform vec3 in_Tint;
in vec2 v_texcoord;
out vec4 out_Color;
void main() {
    vec4 texel = texture(mytexture, v_texcoord);
    out_Color = vec4(texel.rgb * in_Tint, texel.a * in_Alpha);
}
)GLSL"}
            }
        };
        using CE::Rendering::Parameters::Semantic;
        using CE::Rendering::Parameters::Type;
        const CE::Assets::Shaders::MaterialRecipe recipe{
            .id = "space-invaders:enemy-material",
            .program = program.id,
            .topology = CE::Rendering::Geometry::Topology::TriangleStrip,
            .parameters = {
                {"projection", Type::Mat4, true, Semantic::Projection},
                {"view", Type::Mat4, true, Semantic::View},
                {"model", Type::Mat4, true, Semantic::Model},
                {"alpha", Type::Float, true, Semantic::Alpha},
                {"scale", Type::Float, true, Semantic::Scale},
                {"image", Type::Sampler2D},
                {"tint", Type::Vec3}
            },
            .bindings = {{"opengl", R"JSON({"parameters":{
                "projection":"projectionMatrix", "view":"viewMatrix", "model":"modelMatrix",
                "alpha":"in_Alpha", "scale":"in_Scale", "image":"mytexture", "tint":"in_Tint"
            }})JSON"}}
        };
        const auto executable = builder->build_program(program);
        return builder->build_material(recipe, program, executable);
    }
}

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
    enemy_material = make_enemy_material(engine_context.resources());

    CE::Text::FontSelection font_selection;
    font_selection.automatic_system_fonts = true;
    font_selection.asset_directories = {asset_root};
    font_selection.preferred = {
        "ProggyVector", "Cantarell", "FreeSans", "Adwaita Mono", "FreeSerif", "DejaVu Sans"
    };
    engine_context.fonts().configure(font_selection);
    camera.set_framebuffer_size(engine_context.window().framebuffer_size());

    invasion.init_platform(engine_context);
    invasion.set_material(enemy_material);
    bullets.init_platform(engine_context);
    ui.init_platform(engine_context);
    ui_controller.init_platform(engine_context);
    player_controller.init_platform(engine_context);
    ufo_controller.init_platform(engine_context);
    invasion_controller.init_platform(engine_context);
}

void SpaceInvaders::init_simulation() {
    unregister_listeners();
    game_speed = 1.0;
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
        glm::vec2{size.width * 0.5f, size.height - SpaceInvadersConfig::ufo_spawn_top_offset},
        static_cast<int>(Invasion::formation_size),
        engine_context.events().publisher(),
        enemy_material
    );
    ufo->init_simulation();
    bullets.init_simulation();
    ui.init_simulation();
    ui.set_score(score.get());
    ui.set_lives(lives.get());
    ui.set_game_over(false);

    player_controller.attach(*player);
    ufo_controller.attach(*ufo);

    ui_controller.init_simulation();
    player_controller.init_simulation();
    ufo_controller.init_simulation();
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
