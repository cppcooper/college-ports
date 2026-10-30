#pragma once

#include <compare>
#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <vector>

namespace RoverModel {
    struct Position {
        int x = 0;
        int y = 0;
        auto operator<=>(const Position&) const = default;
        Position operator+(Position p) const { return {x + p.x, y + p.y}; }
    };
    class TileMap {
        int width_ = 0;
        int height_ = 0;
        std::vector<std::uint8_t> tiles;

    public:
        TileMap() = default;
        TileMap(int width, int height, std::uint8_t fill = 2);
        static TileMap read(std::istream& input);
        static TileMap load(const std::filesystem::path& path);
        void write(std::ostream& output) const;
        void save(const std::filesystem::path& path) const;
        int width() const { return width_; }
        int height() const { return height_; }
        bool contains(Position p) const;
        bool can_move(Position p) const;
        std::uint8_t at(Position p) const;
        void set(Position p, std::uint8_t tile);
        void clear_visits();
    };
}
