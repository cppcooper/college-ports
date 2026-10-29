#include <cheryl/backends/opengl/startup.h>

#include <filesystem>

#include <CLI/CLI.hpp>

#include <space_invaders/SpaceInvaders.h>

int main(int argc, char** argv) {
    std::filesystem::path asset_root = "/mnt/projects/assets";
    std::filesystem::path high_score_file = SPACE_INVADERS_SCORE_FILE;
    auto startup = CE::Application::GLFWOpenGL::make_startup(
        "Bullet Hell Space Invaders!!", {.width = 900, .title = "Hell Invaders!! (from space)"}
    );
    startup.add_application_options([&asset_root, &high_score_file](CLI::App& options) {
        options.add_option("--assets", asset_root, "Game asset directory")->default_str(asset_root);
        options.add_option("--highscores", high_score_file, "Top-10 high-score text file")->default_str(high_score_file);
    });

    auto result = startup.initialize(argc, argv);
    if (!result.should_start()) {
        return result.exit_code;
    }

    SpaceInvaders game(*result.context, asset_root, high_score_file);
    auto runtime = result.make_runtime(game);
    runtime.run();

    return result.exit_code;
}
