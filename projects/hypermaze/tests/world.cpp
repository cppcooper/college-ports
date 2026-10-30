#include <hypermaze/world/HyperMaze.h>

#include <algorithm>
#include <deque>
#include <random>
#include <stdexcept>

using namespace HypermazeModel;

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void approach(HyperMaze& maze, Position target) {
    auto local = maze.player() - maze.origin();
    // Approach along the interior before stepping onto a boundary door.
    const Position interior{std::clamp(target.x, 1, maze.terrain().width - 2),
        std::clamp(target.y, 1, maze.terrain().height - 2)};
    for (auto goal : {interior, target}) {
        while (local.x != goal.x) {
            check(maze.move(local.x < goal.x ? Direction::Right : Direction::Left), "Approach x blocked");
            local = maze.player() - maze.origin();
        }
        while (local.y != goal.y) {
            check(maze.move(local.y < goal.y ? Direction::Up : Direction::Down), "Approach y blocked");
            local = maze.player() - maze.origin();
        }
    }
}

int main() {
    for (auto direction : {Direction::Left, Direction::Right, Direction::Up, Direction::Down}) {
        for (bool self_loop : {false, true}) {
            Graph graph;
            graph.nodes.resize(self_loop ? 1 : 2);
            const std::size_t target = self_loop ? 0 : 1;
            graph.links.push_back({{0, target}, direction, 5});
            graph.nodes[0].doors[static_cast<int>(direction)].push_back({0, 0});
            graph.nodes[target].doors[static_cast<int>(opposite(direction))].push_back({0, 1});
            HyperMaze maze(graph);
            check(!maze.terrain().can_move({0, 0}), "Wall collision failed");
            approach(maze, graph.nodes[0].doorway(direction, 0));
            const auto doorway = maze.player();
            check(maze.move(direction) && maze.in_corridor(), "Cannot enter corridor");
            for (int i = 0; i < 5; ++i) check(maze.move(direction), "Cannot cross corridor");
            check(!maze.in_corridor() && maze.current_node() == target, "Wrong corridor destination");
            for (int i = 0; i < 6; ++i) check(maze.move(opposite(direction)), "Cannot return through corridor");
            check(!maze.in_corridor() && maze.current_node() == 0 && maze.player() == doorway, "Round trip drift");
            if (direction == Direction::Left || direction == Direction::Down)
                check(maze.discovered_tiles() > 9, "Negative-coordinate terrain lost");
        }
    }
    for (std::uint32_t seed = 0; seed < 64; ++seed) {
        auto graph = Graph::generate(seed);
        check(graph.nodes.size() >= 2 && graph.nodes.size() <= 8192, "Invalid graph size");
        std::vector<bool> seen(graph.nodes.size());
        std::deque<std::size_t> queue{0};
        seen[0] = true;
        while (!queue.empty()) {
            const auto node = queue.front(); queue.pop_front();
            for (const auto& wall : graph.nodes[node].doors)
                for (const auto& door : wall) {
                    const auto& link = graph.links.at(door.link);
                    check(link.nodes.at(door.side) == node, "Door endpoint mismatch");
                    const auto next = link.nodes.at(1 - door.side);
                    if (!seen[next]) { seen[next] = true; queue.push_back(next); }
                }
        }
        check(std::ranges::all_of(seen, [](bool value) { return value; }), "Disconnected graph");
        HyperMaze maze(std::move(graph), seed);
        std::mt19937 random(seed);
        std::vector<Direction> moves;
        for (int i = 0; i < 1000; ++i) {
            const auto d = static_cast<Direction>(random() % 4);
            if (maze.move(d)) moves.push_back(d);
            check(maze.terrain().can_move(maze.player() - maze.origin()), "Player outside walkable terrain");
        }
        for (auto it = moves.rbegin(); it != moves.rend(); ++it)
            check(maze.move(opposite(*it)), "Generated maze traversal cannot be reversed");
        check(maze.player() == Position{1, 1} && maze.current_node() == 0 && !maze.in_corridor(), "Generated maze round trip drift");
    }
}
