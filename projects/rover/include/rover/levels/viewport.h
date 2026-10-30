#pragma once

#include <rover/levels/tile-map.h>

#include <optional>

namespace RoverModel {
    class Viewport {
        int map_width;
        int map_height;
        int columns_;
        int rows_;
        Position origin_{};
    public:
        explicit Viewport(const TileMap& map, int columns = 100, int rows = 60);
        void follow(Position target);
        void scroll(int dx, int dy);
        Position origin() const { return origin_; }
        int columns() const { return columns_; }
        int rows() const { return rows_; }
        std::optional<Position> screen_tile(double x, double y, int logical_width, int logical_height) const;
    };
}
