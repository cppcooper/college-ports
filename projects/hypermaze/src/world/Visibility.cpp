#include <hypermaze/world/Visibility.h>
#include "VisibilityRay.h"

namespace HypermazeModel {
    bool has_line_of_sight(Position eye, Position target, const TileLookup& tile_at) {
        return VisibilityDetail::trace_ray(eye, target, std::cref(tile_at)) != Tile::Blank;
    }

    VisibleTiles field_of_view(Position eye, int columns, int rows, const TileLookup& tile_at) {
        return VisibilityDetail::field_of_view(eye, columns, rows, [&](Position target) {
            return VisibilityDetail::trace_ray(eye, target, std::cref(tile_at));
        });
    }
}
