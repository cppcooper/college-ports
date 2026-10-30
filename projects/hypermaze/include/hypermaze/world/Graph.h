#pragma once

#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace HypermazeModel {
    enum class Direction { Left, Right, Up, Down };
    struct Position {
        int x = 0;
        int y = 0;
        auto operator<=>(const Position&) const = default;
        Position operator+(Position p) const { return {x + p.x, y + p.y}; }
        Position operator-(Position p) const { return {x - p.x, y - p.y}; }
    };
    Direction opposite(Direction direction);
    Position offset(Direction direction);
    struct Door {
        std::size_t link;
        std::size_t side;
        auto operator<=>(const Door&) const = default;
    };
    struct Node {
        std::array<std::vector<Door>, 4> doors;
        const std::vector<Door>& wall(Direction d) const { return doors[static_cast<std::size_t>(d)]; }
        int width() const;
        int height() const;
        Position doorway(Direction wall, std::size_t index) const;
    };
    struct Link {
        std::array<std::size_t, 2> nodes;
        Direction direction;
        int length;
    };
    struct Graph {
        std::vector<Node> nodes;
        std::vector<Link> links;
        static Graph generate(std::uint32_t seed, unsigned generations = 44, double complexity = .43);
    };
}
