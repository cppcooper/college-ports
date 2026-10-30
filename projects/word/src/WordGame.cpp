#include <word/WordGame.h>
#include <college_ports/assets.h>

#include <utility>

WordGame::WordGame(CE::Runtime::Context& engine, std::filesystem::path assets, const std::filesystem::path& puzzle)
: engine(engine), assets(std::move(assets)), box(WordModel::WordBox::load(puzzle)), search(box), ui(engine, box, search) {}
void WordGame::init_platform() {
    CollegePorts::configure_fonts(engine, assets);
    controller.init_platform(engine);
    ui.init_platform();
}
void WordGame::init_simulation() {
    search.scan();
    ui.init_simulation();
}
void WordGame::update(const CE::Game::TickContext& tick) {
    ui.handle_input(tick);
    const auto commands = controller.update(tick);
    if (commands.quit) { tick.request_stop(); return; }
    if (commands.rescan) search.scan();
    if (commands.previous) search.select_previous();
    if (commands.next) search.select_next();
    if (commands.reveal) search.toggle_solution();
    if (commands.reveal_all) search.toggle_all();
    ui.resize(tick.logical_size.width, tick.logical_size.height);
    if (commands.rescan || commands.previous || commands.next || commands.reveal || commands.reveal_all) ui.show_state();
    ui.update(tick);
}
void WordGame::prepare_render_frame(CE::Rendering::FrameWriter& frame) const { ui.append_passes(frame); }
void WordGame::deinit_platform() {
    ui.deinit_platform();
    controller.deinit_platform();
}
