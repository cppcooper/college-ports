#include <cheryl/backends/opengl/startup.h>
#include <CLI/CLI.hpp>
#include <word/WordGame.h>

int main(int argc, char** argv) {
    std::filesystem::path assets = COLLEGE_PORTS_ASSETS;
    std::filesystem::path puzzle = WORD_DEFAULT_PUZZLE;
    auto startup = CE::Application::GLFWOpenGL::make_startup(
        "Word Search", {.width = 1100, .height = 760, .title = "Word Search"});
    startup.add_application_options([&](CLI::App& options) {
        options.add_option("--assets", assets, "Font asset directory")->default_str(assets.string());
        options.add_option("--puzzle", puzzle, "Text or original binary .wrd puzzle")->default_str(puzzle.string())->check(CLI::ExistingFile);
    });
    auto result = startup.initialize(argc, argv);
    if (!result.should_start()) return result.exit_code;
    WordGame game(*result.context, assets, puzzle);
    auto runtime = result.make_runtime(game);
    runtime.run();
    return result.exit_code;
}
