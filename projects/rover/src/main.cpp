#include <cheryl/backends/opengl/startup.h>
#include <CLI/CLI.hpp>
#include <rover/RoverGame.h>

int main(int argc, char** argv) {
    RoverOptions options{.assets = COLLEGE_PORTS_ASSETS,
        .map = std::filesystem::path(ROVER_DATA_DIR) / "Map1.map", .save = ROVER_SAVE_FILE};
    std::vector<int> start, goal;
    auto startup = CE::Application::GLFWOpenGL::make_startup(
        "Rover pathfinding and map editor", {.width = 1000, .height = 600, .title = "Rover"});
    startup.add_application_options([&](CLI::App& app) {
        app.add_option("--assets", options.assets, "Game asset directory")->default_str(options.assets.string());
        app.add_option("--map", options.map, "Original packed map file")->default_str(options.map.string())->check(CLI::ExistingFile);
        app.add_option("--save-map", options.save, "Editor save destination (F5)")->default_str(options.save.string());
        app.add_option("--start", start, "Start tile: X Y")->expected(2);
        app.add_option("--goal", goal, "Goal tile: X Y")->expected(2);
        app.add_flag("--editor", options.editor, "Start in map editor");
        app.add_option("--steps-per-second", options.steps_per_second, "Search operations per second")
            ->check(CLI::Range(1., 100000.))->default_val(options.steps_per_second);
    });
    auto result = startup.initialize(argc, argv);
    if (!result.should_start()) return result.exit_code;
    if (!start.empty()) options.start = RoverModel::Position{start[0], start[1]};
    if (!goal.empty()) options.goal = RoverModel::Position{goal[0], goal[1]};
    RoverGame game(*result.context, std::move(options));
    auto runtime = result.make_runtime(game);
    runtime.run();
    return result.exit_code;
}
