#include <rover/levels/map-editor.h>

#include <algorithm>
#include <stdexcept>

namespace RoverModel {
    void MapEditor::select(std::uint8_t tile) {
        if (tile > 2) throw std::invalid_argument("Editor tile must be rock, hole or dirt (0..2)");
        tile_ = tile;
    }
    void MapEditor::resize_brush(int delta) { radius_ = std::clamp(radius_ + delta, 2, 6); }
    void MapEditor::paint(Position center) {
        if (!map.contains(center)) return;
        const int radius = spray_ ? radius_ : 0;
        for (int y = std::max(0, center.y-radius); y <= std::min(map.height()-1, center.y+radius); ++y)
            for (int x = std::max(0, center.x-radius); x <= std::min(map.width()-1, center.x+radius); ++x)
                map.set({x, y}, tile_);
    }
}
