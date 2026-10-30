#pragma once

#include <hypermaze/world/Visibility.h>

#include <cstdint>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <utility>

namespace HypermazeModel::VisibilityDetail {
    // A cursor samples successive neighboring cells. Copies preserve its current
    // location, allowing corner probes to follow separate room/corridor paths.
    template<class Cursor>
    Tile trace_ray(Position eye, Position target, Cursor cursor) {
        const auto source = cursor(eye);
        if (eye == target) return source;
        if (blocks_sight(source)) return Tile::Blank;

        const auto dx = std::abs(static_cast<std::int64_t>(target.x) - eye.x);
        const auto dy = std::abs(static_cast<std::int64_t>(target.y) - eye.y);
        const int step_x = (target.x > eye.x) - (target.x < eye.x);
        const int step_y = (target.y > eye.y) - (target.y < eye.y);
        std::int64_t crossed_x = 0, crossed_y = 0;
        auto current = eye;
        while (current != target) {
            // Check every tile the ray enters, including shallow-angle rays.
            // Floating multiplication avoids overflow for signed world offsets;
            // viewport-sized integer products remain exact.
            const auto crossing = (2 * crossed_x + 1) * static_cast<long double>(dy)
                - (2 * crossed_y + 1) * static_cast<long double>(dx);
            if (crossing == 0) {
                auto horizontal_cursor = cursor;
                auto vertical_cursor = cursor;
                const auto horizontal = horizontal_cursor(Position{current.x + step_x, current.y});
                const auto vertical = vertical_cursor(Position{current.x, current.y + step_y});
                const Position diagonal{current.x + step_x, current.y + step_y};
                if (blocks_sight(horizontal) && blocks_sight(vertical)) {
                    // Show the enclosing wall, without seeing between touching
                    // walls or through unknown space at a diagonal corner.
                    if (horizontal == Tile::Blank || vertical == Tile::Blank || diagonal != target)
                        return Tile::Blank;
                    const auto wall = horizontal_cursor(diagonal);
                    return blocks_sight(wall) ? wall : Tile::Blank;
                }
                // Continue through the open side of the corner, carrying its
                // region placement when that side crosses a doorway.
                cursor = blocks_sight(horizontal) ? std::move(vertical_cursor) : std::move(horizontal_cursor);
                current = diagonal;
                ++crossed_x;
                ++crossed_y;
            } else if (crossing < 0) {
                current.x += step_x;
                ++crossed_x;
            } else {
                current.y += step_y;
                ++crossed_y;
            }
            const auto tile = cursor(current);
            if (current == target) return tile;
            if (blocks_sight(tile)) return Tile::Blank;
        }
        return source;
    }

    template<class Raycast>
    VisibleTiles field_of_view(Position eye, int columns, int rows, const Raycast& raycast) {
        if (columns < 0 || rows < 0) throw std::invalid_argument("Visibility extents must be nonnegative");
        VisibleTiles visible;
        for (std::int64_t y = -static_cast<std::int64_t>(rows); y <= rows; ++y)
            for (std::int64_t x = -static_cast<std::int64_t>(columns); x <= columns; ++x) {
                const auto target_x = eye.x + x, target_y = eye.y + y;
                if (target_x < std::numeric_limits<int>::min() || target_x > std::numeric_limits<int>::max()
                    || target_y < std::numeric_limits<int>::min() || target_y > std::numeric_limits<int>::max()) continue;
                const Position target{static_cast<int>(target_x), static_cast<int>(target_y)};
                const auto tile = raycast(target);
                if (tile != Tile::Blank) visible.emplace(target, tile);
            }
        return visible;
    }
}
