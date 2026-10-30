#include <rover/RoverGame.h>
#include <college_ports/assets.h>
#include <cheryl/core/display/window-interface.h>

#include <algorithm>
#include <cmath>
#include <format>
#include <utility>

namespace {
    RoverModel::Position endpoint(const RoverModel::TileMap& map, bool goal) {
        if (!goal && map.can_move({37, 32})) return {37, 32};
        for (int i = 0; i < map.width() * map.height(); ++i) {
            const int index = goal ? map.width() * map.height() - 1 - i : i;
            const RoverModel::Position p{index % map.width(), index / map.width()};
            if (map.can_move(p)) return p;
        }
        throw std::runtime_error("Map has no traversable tiles");
    }
}
RoverGame::RoverGame(CE::Runtime::Context& engine, RoverOptions options)
: engine(engine), options(std::move(options)), map(RoverModel::TileMap::load(this->options.map)),
  rover(map), editor(map), viewport(map),
  start(this->options.start.value_or(endpoint(map, false))), goal(this->options.goal.value_or(endpoint(map, true))),
  ui(engine), editing(this->options.editor) {
    if (!std::isfinite(this->options.steps_per_second) || this->options.steps_per_second < 1 || this->options.steps_per_second > 100000)
        throw std::invalid_argument("Search speed must be finite and in 1..100000");
    if (!map.can_move(start) || !map.can_move(goal)) throw std::invalid_argument("Start and goal must be traversable map tiles");
    viewport.follow(start);
}
void RoverGame::init_platform() {
    CollegePorts::load_assets(engine, options.assets);
    tiles = CollegePorts::tileset("tileset", "tileset");
    for (std::size_t i = 0; i < sprites.size(); ++i)
        sprites[i] = CollegePorts::sprite("rover", std::format("frame_{:02}", i));
    camera.set_framebuffer_size(engine.window().framebuffer_size());
    controller.init_platform(engine);
    ui.init_platform();
}
void RoverGame::start_search() {
    credit = 0;
    paused = false;
    notice.clear();
    if (map.can_move(start) && map.can_move(goal)) rover.start(start, goal);
    else { rover.cancel(); notice = "Start or goal is blocked; edit those tiles and press R."; }
}
void RoverGame::init_simulation() {
    ui.init_simulation();
    if (!editing) start_search();
    else { rover.start(start, goal); rover.cancel(); }
    ui.show_state(rover, editor, editing, paused, options.steps_per_second, notice);
}
void RoverGame::update(const CE::Game::TickContext& tick) {
    camera.set_framebuffer_size(tick.framebuffer_size);
    const auto commands = controller.update(tick);
    if (commands.quit) { tick.request_stop(); return; }
    if (commands.toggle_editor) {
        editing = !editing;
        if (editing) { rover.cancel(); notice.clear(); }
        else start_search();
    }
    if (commands.faster) options.steps_per_second = std::min(100000., options.steps_per_second * 2);
    if (commands.slower) options.steps_per_second = std::max(1., options.steps_per_second * 0.5);
    if (editing) {
        if (commands.tile) editor.select(*commands.tile);
        if (commands.spray) editor.toggle_spray();
        editor.resize_brush(commands.brush_delta);
        pan_cooldown -= tick.delta_seconds;
        if (commands.pan != RoverModel::Position{} && pan_cooldown <= 0) {
            viewport.scroll(commands.pan.x * std::max(1, viewport.columns()/5), commands.pan.y * std::max(1, viewport.rows()/5));
            pan_cooldown = 0.1;
        }
        for (auto p : commands.strokes)
            if (auto tile = viewport.screen_tile(p.x, p.y, tick.logical_size.width, tick.logical_size.height)) editor.paint(*tile);
        if (commands.save) {
            try { map.save(options.save); notice = "Saved: " + options.save.string(); }
            catch (const std::exception& error) { notice = error.what(); }
        }
    } else {
        if (commands.restart) start_search();
        if (commands.pause) paused = !paused;
        if (commands.step) { paused = true; rover.advance(1); }
        else if (!paused) {
            credit = std::min(4096., credit + tick.delta_seconds * options.steps_per_second);
            const auto budget = static_cast<std::size_t>(credit);
            rover.advance(budget);
            credit -= static_cast<double>(budget);
        }
        viewport.follow(rover.position());
    }
    ui.show_state(rover, editor, editing, paused, options.steps_per_second, notice);
    ui.update(tick);
}
void RoverGame::prepare_render_frame(CE::Rendering::FrameWriter& frame) const {
    const auto size = camera.framebuffer_size();
    const glm::vec2 tile_size{float(size.width)/viewport.columns(), float(size.height)/viewport.rows()};
    const auto origin = viewport.origin();
    {
        auto pass = frame.begin_pass(camera.projection_matrix(), camera.view_matrix());
        static constexpr std::array<std::uint32_t, 4> cells{2, 1, 0, 3};
        for (int y = 0; y < viewport.rows(); ++y)
            for (int x = 0; x < viewport.columns(); ++x)
                CollegePorts::append_tile(pass, *tiles, cells[map.at({origin.x+x, origin.y+y})],
                    glm::vec2{x, y} * tile_size, tile_size);
        if (!editing && rover.status() != RoverModel::SearchStatus::Idle) {
            const auto p = rover.position();
            CollegePorts::append_sprite(pass, *sprites[static_cast<int>(rover.facing())],
                (glm::vec2{p.x-origin.x, p.y-origin.y} + 0.5f) * tile_size, tile_size * glm::vec2{1.7f, 2.2f});
        }
    }
    ui.append_passes(frame);
}
void RoverGame::deinit_platform() {
    ui.deinit_platform();
    controller.deinit_platform();
    for (auto& sprite : sprites) sprite.reset();
    tiles.reset();
}
