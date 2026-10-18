#include <space_invaders/SpaceInvaders.h>

int main(int argc, char** argv) {
    auto startup = CE::Application::GLFWOpenGL::make_startup(
        "Bullet Hell Space Invaders!!", {.title = "Hell Invaders!! (from space)"}
    );
    auto result = startup.initialize(argc, argv);
    if (!result.should_start())
        return result.exit_code;

    SpaceInvaders game(*result.context);
    auto runtime = result.make_runtime(game);
    runtime.run();

    return result.exit_code;
}