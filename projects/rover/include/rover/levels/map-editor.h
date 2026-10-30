#pragma once

#include <rover/levels/tile-map.h>

namespace RoverModel {
    class MapEditor {
        TileMap& map;
        std::uint8_t tile_ = 2;
        bool spray_ = false;
        int radius_ = 2;
    public:
        explicit MapEditor(TileMap& map) : map(map) {}
        void select(std::uint8_t tile);
        void toggle_spray() { spray_ = !spray_; }
        void resize_brush(int delta);
        void paint(Position center);
        int radius() const { return radius_; }
        bool spray() const { return spray_; }
        std::uint8_t tile() const { return tile_; }
    };
}
