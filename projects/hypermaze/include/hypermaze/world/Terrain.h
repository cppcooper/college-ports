#pragma once

#include <hypermaze/world/Graph.h>

namespace HypermazeModel {
    enum class Tile : std::uint16_t {
        Blank, Floor1, Floor2, Wall1, Wall2, TopLeft, TopRight, BottomLeft,
        BottomRight, Stairs, DoorOpen, DoorClosed, GateClosed, Horizontal, Vertical
    };
    bool blocks_sight(Tile tile);
    struct Terrain {
        int width = 0;
        int height = 0;
        std::vector<Tile> tiles;
        bool contains(Position p) const;
        bool can_move(Position p) const;
        Tile at(Position p) const;
        static Terrain room(const Node& node, std::uint32_t seed);
        static Terrain corridor(const Link& link, std::uint32_t seed);
    };
}
