#include <hypermaze/Hypermaze.h>
#include <college_ports/assets.h>
#include <cheryl/core/display/window-interface.h>

#include <cmath>
#include <utility>

HypermazeGame::HypermazeGame(CE::Runtime::Context& engine, std::filesystem::path assets, std::uint32_t seed)
: engine(engine), asset_root(std::move(assets)), seed(seed), maze(seed), ui(engine) {}

void HypermazeGame::init_platform() {
    CollegePorts::load_assets(engine, asset_root);
    tiles = CollegePorts::tileset("hypermaze", "hypermaze");
    player.emplace(CE::Assets::Graphics::Asset::load(
        asset_root / "graphics/textures/misc/player.png", engine.resources(), {0.5f, 0.5f}));
    camera.set_framebuffer_size(engine.window().framebuffer_size());
    controller.init_platform(engine);
    ui.init_platform();
}

void HypermazeGame::init_simulation() {
    ui.init_simulation();
    ui.show_world(maze, seed);
    refresh_visibility();
}

void HypermazeGame::update(const CE::Game::TickContext& tick) {
    const auto previous_size = camera.framebuffer_size();
    bool visibility_changed = previous_size != tick.framebuffer_size;
    camera.set_framebuffer_size(tick.framebuffer_size);
    const auto commands = controller.update(tick);
    if (commands.quit) { tick.request_stop(); return; }
    if (commands.regenerate) {
        maze = HypermazeModel::HyperMaze(++seed);
        visibility_changed = true;
    } else if (commands.move) {
        visibility_changed |= maze.move(*commands.move);
    }
    if (visibility_changed) refresh_visibility();
    if (commands.toggle_debug) ui.toggle_debug();
    ui.show_world(maze, seed);
    ui.update(tick);
}

void HypermazeGame::refresh_visibility() {
    const auto size = camera.framebuffer_size();
    const int columns = static_cast<int>(std::ceil(size.width / (2 * tile_size))) + 1;
    const int rows = static_cast<int>(std::ceil(size.height / (2 * tile_size))) + 1;
    map_tiles = maze.visible_tiles(columns, rows);
}

void HypermazeGame::prepare_render_frame(CE::Rendering::FrameWriter& frame) const {
    const auto size = camera.framebuffer_size();
    const glm::vec2 center{size.width * 0.5f, size.height * 0.5f};
    const auto p = maze.player();
    {
        auto pass = frame.begin_pass(camera.projection_matrix(), camera.view_matrix());
        for (const auto& [position, tile] : map_tiles)
            CollegePorts::append_tile(pass, *tiles, static_cast<std::uint32_t>(tile),
                center + glm::vec2{position.x - p.x - 0.5f, position.y - p.y - 0.5f} * tile_size,
                {tile_size, tile_size});
        pass.add(CE::Rendering::Submission::resolve_graphic(*player,
            CollegePorts::draw_style(center, {1, 1}),
            {pass.semantics(), pass.parameters(), pass.constraints()}));
    }
    ui.append_passes(frame);
}

void HypermazeGame::deinit_platform() {
    ui.deinit_platform();
    controller.deinit_platform();
    player.reset();
    tiles.reset();
}
