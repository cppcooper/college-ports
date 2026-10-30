#include <rover/actors/rover.h>
#include <rover/levels/map-editor.h>
#include <rover/levels/viewport.h>

#include <array>
#include <cmath>
#include <deque>
#include <limits>
#include <sstream>
#include <stdexcept>

using namespace RoverModel;

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
template<class Callback> void rejects(Callback callback) {
    bool rejected = false;
    try { callback(); } catch (const std::exception&) { rejected = true; }
    check(rejected, "Invalid map input accepted");
}
bool reachable(const TileMap& map, Position start, Position goal) {
    std::vector<bool> seen(static_cast<std::size_t>(map.width()) * map.height());
    std::deque<Position> queue{start};
    seen[start.y * map.width() + start.x] = true;
    while (!queue.empty()) {
        const auto p = queue.front(); queue.pop_front();
        if (p == goal) return true;
        for (auto delta : {Position{1, 0}, Position{-1, 0}, Position{0, 1}, Position{0, -1}}) {
            const auto next = p + delta;
            if (!map.can_move(next)) continue;
            const auto index = next.y * map.width() + next.x;
            if (!seen[index]) { seen[index] = true; queue.push_back(next); }
        }
    }
    return false;
}
void verify_search(TileMap& map, Position start, Position goal) {
    const bool expected = reachable(map, start, goal);
    Rover rover(map);
    rover.start(start, goal);
    const auto cells = static_cast<std::size_t>(map.width()) * map.height();
    const auto before = rover.total_moves();
    rover.advance(1);
    check(rover.total_moves() <= before + 1, "Search exceeded simulation budget");
    rover.advance(cells * 6);
    check(rover.status() == (expected ? SearchStatus::Found : SearchStatus::Unreachable), "DFS disagrees with reachability");
    check(rover.total_moves() <= cells && rover.recursion_level() == 0, "Search did not terminate within map bounds");
    if (expected) {
        check(rover.path().front() == start && rover.path().back() == goal, "Path endpoints wrong");
        for (std::size_t i = 1; i < rover.path().size(); ++i) {
            const auto a = rover.path()[i-1], b = rover.path()[i];
            check(std::abs(a.x-b.x) + std::abs(a.y-b.y) == 1 && map.can_move(b), "Invalid path step");
        }
    }
    rover.start(start, start);
    check(rover.status() == SearchStatus::Found && rover.path().size() == 1, "Restart/start-equals-goal failed");
    rover.cancel();
    check(rover.status() == SearchStatus::Idle && rover.total_moves() == 0 && map.at(start) == 2, "Cancel did not clear search state");
}

int main() {
    TileMap packed(8, 8);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) packed.set({x, y}, (x + 3*y) % 4);
    std::stringstream serialized;
    packed.write(serialized);
    const auto loaded = TileMap::read(serialized);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) check(packed.at({x, y}) == loaded.at({x, y}), "Packed map round trip changed tiles");
    for (const auto* bad : {"", "0 1", "-1 1", "1025 1", "1 1", "1 1 4294967296", "1 1 0 extra"})
        rejects([&] { std::istringstream input(bad); TileMap::read(input); });
    rejects([] { TileMap invalid(3, 4); });
    TileMap small(8, 8);
    for (int y = 0; y < 7; ++y) small.set({4, y}, 0);
    verify_search(small, {0, 0}, {7, 0});
    small.set({4, 7}, 1);
    verify_search(small, {0, 0}, {7, 0});

    MapEditor editor(small);
    editor.select(0); editor.toggle_spray(); editor.resize_brush(50);
    editor.paint({0, 0});
    check(small.at({0, 0}) == 0 && small.at({6, 6}) == 0 && small.at({7, 7}) == 2, "Brush clipping or size failed");
    editor.select(2); editor.toggle_spray(); editor.paint({0, 0});
    check(small.at({0, 0}) == 2 && small.at({1, 0}) == 0, "Single tile painting failed");
    editor.paint({-1, -1});
    Viewport view(small, 4, 4);
    view.follow({7, 7});
    check(view.origin() == Position{4, 4}, "Viewport cannot reach map edge");
    check(view.screen_tile(0, 0, 400, 400) == Position{4, 7}, "Pointer y orientation wrong");
    check(view.screen_tile(399, 399, 400, 400) == Position{7, 4}, "Pointer conversion at last tile failed");
    check(!view.screen_tile(400, 0, 400, 400) && !view.screen_tile(0, 0, 0, 0)
        && !view.screen_tile(std::numeric_limits<double>::quiet_NaN(), 0, 400, 400), "Invalid pointer accepted");
    view.scroll(-100, -100);
    check(view.origin() == Position{0, 0}, "Viewport scroll clipping failed");

    for (auto name : {"Map1.map", "Map2.map", "Map3.map"}) {
        auto map = TileMap::load(std::filesystem::path(ROVER_DATA_DIR) / name);
        check(map.width() == 1024 && map.height() == 1024, "Original map dimensions changed");
        verify_search(map, {0, 0}, {1023, 1023});
    }
}
