#include <rover/gui/GameUI.h>

#include <array>
#include <format>

void RoverUI::create_widgets() {
    counters = label("", 20);
    counters->getRenderer()->setBackgroundColor({0, 0, 0, 180});
    ui().set_layout(counters, {.anchor = {0, 1}, .offset = {.fixed = {12, -12}}});
    status = label("", 18);
    status->getRenderer()->setBackgroundColor({0, 0, 0, 180});
    ui().set_layout(status, {.offset = {.fixed = {12, 12}}});
    help = label("", 16);
    help->getRenderer()->setBackgroundColor({0, 0, 0, 180});
    ui().set_layout(help, {.offset = {.fixed = {12, 76}}});
}
void RoverUI::show_state(const RoverModel::Rover& rover, const RoverModel::MapEditor& editor,
    bool editing, bool paused, double speed, const std::string& notice) {
    static constexpr std::array names{"Idle", "Searching", "Path found", "No path"};
    const auto p = rover.position(), goal = rover.goal();
    counters->setText(std::format("Recursion Level: {}\nTotal Moves: {}", rover.recursion_level(), rover.total_moves()));
    status->setText(std::format("{}{} | Position: {}, {} | Goal: {}, {} | {:.0f} steps/s\n{}",
        editing ? "Map editor" : names[static_cast<int>(rover.status())], paused && !editing ? " (paused)" : "",
        p.x, p.y, goal.x, goal.y, speed, notice));
    help->setText(editing
        ? std::format("WASD: scroll   1/2/3: rock/hole/dirt   Mouse: paint   Shift: spray ({}, radius {})\n"
            "Up/Down: brush size   F5: save   E: run search   Esc: quit", editor.spray() ? "on" : "off", editor.radius())
        : "Space: pause   N: single step   R: restart   +/-: speed   E: editor   Esc: quit");
}
