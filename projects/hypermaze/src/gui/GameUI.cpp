#include <hypermaze/gui/GameUI.h>

#include <format>

void HypermazeUI::create_widgets() {
    readout = label("");
    readout->getRenderer()->setBackgroundColor({0, 0, 0, 170});
    ui().set_layout(readout, {.offset = {.fixed = {12, 12}}});
    help = label("WASD / arrows / D-pad: move   R: new maze   F3: debug   Esc: quit", 16);
    help->getRenderer()->setBackgroundColor({0, 0, 0, 170});
    ui().set_layout(help, {.anchor = {0, 1}, .offset = {.fixed = {12, -12}}});
}

void HypermazeUI::toggle_debug() {
    debug_visible = !debug_visible;
    readout->setVisible(debug_visible);
}

void HypermazeUI::show_world(const HypermazeModel::HyperMaze& maze, std::uint32_t seed) {
    const auto p = maze.player(), t = maze.traversal(), local = p - maze.origin();
    readout->setText(std::format(
        "Player: {}, {}\nTraversal: {}, {}\nTerrain: {}, {}\nTravel direction: {}\n"
        "Space: {}\nSeed: {}\nNode: {} / {}\nLink: {} / {}",
        p.x, p.y, t.x, t.y, local.x, local.y, static_cast<int>(maze.travel_direction()),
        maze.in_corridor() ? "corridor" : "room", seed, maze.current_node(), maze.graph().nodes.size(),
        maze.current_link() ? std::to_string(*maze.current_link()) : "none", maze.graph().links.size()));
}
