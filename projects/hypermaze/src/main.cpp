#include <cheryl/backends/opengl/startup.h>
#include <CLI/CLI.hpp>
#include <hypermaze/Hypermaze.h>

#include <chrono>

int main(int argc, char** argv) {
    std::filesystem::path assets = COLLEGE_PORTS_ASSETS;
    auto seed = static_cast<std::uint32_t>(std::chrono::system_clock::now().time_since_epoch().count());
    auto startup = CE::Application::GLFWOpenGL::make_startup(
        "Hypermaze", {.width = 960, .height = 640, .title = "Hypermaze"});
    startup.add_application_options([&](CLI::App& options) {
        options.add_option("--assets", assets, "Game asset directory")->default_str(assets.string());
        options.add_option("--seed", seed, "Maze generation seed");
    });
    auto result = startup.initialize(argc, argv);
    if (!result.should_start()) return result.exit_code;
    HypermazeGame game(*result.context, assets, seed);
    auto runtime = result.make_runtime(game);
    runtime.run();
    return result.exit_code;
}
