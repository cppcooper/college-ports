#include <rover/levels/viewport.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace RoverModel {
    Viewport::Viewport(const TileMap& map, int columns, int rows)
    : map_width(map.width()), map_height(map.height()),
      columns_(std::min(columns, map.width())), rows_(std::min(rows, map.height())) {
        if (columns_ <= 0 || rows_ <= 0) throw std::invalid_argument("Viewport needs a nonempty map and positive dimensions");
    }
    void Viewport::follow(Position p) {
        origin_ = {std::clamp(p.x - columns_ / 2, 0, map_width - columns_),
            std::clamp(p.y - rows_ / 2, 0, map_height - rows_)};
    }
    void Viewport::scroll(int dx, int dy) {
        origin_ = {std::clamp(origin_.x + dx, 0, map_width - columns_),
            std::clamp(origin_.y + dy, 0, map_height - rows_)};
    }
    std::optional<Position> Viewport::screen_tile(double x, double y, int width, int height) const {
        if (width <= 0 || height <= 0 || !std::isfinite(x) || !std::isfinite(y) || x < 0 || y < 0 || x >= width || y >= height)
            return std::nullopt;
        return Position{origin_.x + static_cast<int>(x / width * columns_),
            origin_.y + rows_ - 1 - static_cast<int>(y / height * rows_)};
    }
}
