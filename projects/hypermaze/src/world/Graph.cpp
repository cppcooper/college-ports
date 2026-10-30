#include <hypermaze/world/Graph.h>

#include <algorithm>
#include <cmath>
#include <deque>
#include <numbers>
#include <random>
#include <stdexcept>

namespace HypermazeModel {
    Direction opposite(Direction d) {
        switch (d) {
            case Direction::Left: return Direction::Right;
            case Direction::Right: return Direction::Left;
            case Direction::Up: return Direction::Down;
            case Direction::Down: return Direction::Up;
        }
        throw std::invalid_argument("Invalid maze direction");
    }
    Position offset(Direction d) {
        switch (d) {
            case Direction::Left: return {-1, 0};
            case Direction::Right: return {1, 0};
            case Direction::Up: return {0, 1};
            case Direction::Down: return {0, -1};
        }
        throw std::invalid_argument("Invalid maze direction");
    }
    int Node::width() const {
        return std::max(3, 1 + 2 * static_cast<int>(std::max(wall(Direction::Up).size(), wall(Direction::Down).size())));
    }
    int Node::height() const {
        return std::max(3, 1 + 2 * static_cast<int>(std::max(wall(Direction::Left).size(), wall(Direction::Right).size())));
    }
    Position Node::doorway(Direction d, std::size_t index) const {
        if (index >= wall(d).size()) throw std::out_of_range("Maze door index");
        const auto coordinate = 1 + 2 * static_cast<int>(index);
        switch (d) {
            case Direction::Left: return {0, height() - 1 - coordinate};
            case Direction::Right: return {width() - 1, coordinate};
            case Direction::Up: return {width() - 1 - coordinate, height() - 1};
            case Direction::Down: return {coordinate, 0};
        }
        throw std::invalid_argument("Invalid maze direction");
    }
    Graph Graph::generate(std::uint32_t seed, unsigned generations, double complexity) {
        if (generations == 0 || generations > 64 || !std::isfinite(complexity) || complexity < 0 || complexity > 1)
            throw std::invalid_argument("Maze generations must be 1..64 and complexity 0..1");
        Graph graph;
        graph.nodes.emplace_back();
        std::mt19937 random(seed);
        const auto weight = [complexity](double x) {
            return 5 + std::cos(complexity * std::numbers::pi) * std::sin(x * std::numbers::pi / 5);
        };
        const std::array intervals{3., 4., 5., 6., 7., 8.};
        const std::array weights{weight(3), weight(4)*2, weight(3)*2, weight(5)*3, weight(7)*3, weight(6)*2};
        std::piecewise_linear_distribution<double> big_doors(intervals.begin(), intervals.end(), weights.begin());
        std::bernoulli_distribution big_room(0.1 + 0.5 * complexity);
        std::bernoulli_distribution dead_end(0.87 - 0.62 * complexity);
        std::bernoulli_distribution loop(0.23 + 0.42 * complexity);
        struct Expansion { std::size_t node; unsigned depth; unsigned children; };
        std::deque<Expansion> pending{{0, 0, 1}};
        // Bound probabilistic expansion while keeping every new room connected.
        constexpr std::size_t node_limit = 8192;
        constexpr std::size_t link_limit = 32768;
        while (!pending.empty() && graph.links.size() < link_limit) {
            const auto parent = pending.front();
            pending.pop_front();
            for (unsigned i = 0; i < parent.children && graph.links.size() < link_limit; ++i) {
                if (parent.node != 0 && !pending.empty() && dead_end(random)) continue;
                const auto direction = static_cast<Direction>(std::uniform_int_distribution<int>(0, 3)(random));
                std::size_t child;
                if (graph.nodes.size() >= node_limit || (graph.nodes.size() > 1 && loop(random))) {
                    child = std::uniform_int_distribution<std::size_t>(0, graph.nodes.size() - 1)(random);
                } else {
                    child = graph.nodes.size();
                    graph.nodes.emplace_back();
                    if (parent.depth + 1 < generations) {
                        const unsigned doors = big_room(random) ? static_cast<unsigned>(big_doors(random)) : 2;
                        pending.push_back({child, parent.depth + 1, doors - 1});
                    }
                }
                const auto link = graph.links.size();
                graph.links.push_back({{parent.node, child}, direction, std::uniform_int_distribution<int>(4, 8)(random)});
                graph.nodes[parent.node].doors[static_cast<std::size_t>(direction)].push_back({link, 0});
                graph.nodes[child].doors[static_cast<std::size_t>(opposite(direction))].push_back({link, 1});
            }
        }
        return graph;
    }
}
