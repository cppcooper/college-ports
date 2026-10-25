#include <cheryl/backends/opengl/startup.h>

#include <filesystem>

#include <CLI/CLI.hpp>

#include <space_invaders/SpaceInvaders.h>

int main(int argc, char** argv) {
    std::filesystem::path asset_root = "/mnt/projects/assets";
    auto startup = CE::Application::GLFWOpenGL::make_startup(
        "Bullet Hell Space Invaders!!", {.title = "Hell Invaders!! (from space)"}
    );
    startup.add_application_options([&asset_root](CLI::App& options) {
        options.add_option("--assets", asset_root, "Game asset directory")->default_str(asset_root);
    });

    auto result = startup.initialize(argc, argv);
    if (!result.should_start()) {
        return result.exit_code;
    }

    SpaceInvaders game(*result.context, asset_root);
    auto runtime = result.make_runtime(game);
    runtime.run();

    return result.exit_code;
}
