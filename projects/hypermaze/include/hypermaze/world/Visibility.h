#pragma once

#include <hypermaze/world/Terrain.h>

#include <functional>
#include <map>

namespace HypermazeModel {
    using TileLookup = std::function<Tile(Position)>;
    using VisibleTiles = std::map<Position, Tile>;

    // Rays run between tile centers. The first opaque tile is visible, but
    // unknown tiles and tiles behind it are hidden. Two touching opaque tiles
    // also close the diagonal gap between their corners.
    bool has_line_of_sight(Position eye, Position target, const TileLookup& tile_at);
    VisibleTiles field_of_view(Position eye, int columns, int rows, const TileLookup& tile_at);
}
