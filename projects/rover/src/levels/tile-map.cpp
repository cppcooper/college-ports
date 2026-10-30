#include <rover/levels/tile-map.h>

#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>

namespace RoverModel {
    TileMap::TileMap(int width, int height, std::uint8_t fill) : width_(width), height_(height) {
        if (width <= 0 || height <= 0 || width > 4096 || height > 4096 || width % 4 || height % 4 || fill > 3)
            throw std::invalid_argument("Map dimensions must be multiples of four in 4..4096; tile must be 0..3");
        tiles.assign(static_cast<std::size_t>(width) * height, fill);
    }
    bool TileMap::contains(Position p) const { return p.x >= 0 && p.y >= 0 && p.x < width_ && p.y < height_; }
    bool TileMap::can_move(Position p) const { return contains(p) && at(p) >= 2; }
    std::uint8_t TileMap::at(Position p) const {
        if (!contains(p)) throw std::out_of_range("Map tile outside bounds");
        return tiles[static_cast<std::size_t>(p.y) * width_ + p.x];
    }
    void TileMap::set(Position p, std::uint8_t tile) {
        if (!contains(p)) throw std::out_of_range("Map tile outside bounds");
        if (tile > 3) throw std::invalid_argument("Map tile must be 0..3");
        tiles[static_cast<std::size_t>(p.y) * width_ + p.x] = tile;
    }
    void TileMap::clear_visits() {
        for (auto& tile : tiles) if (tile == 3) tile = 2;
    }
    TileMap TileMap::read(std::istream& input) {
        std::uint64_t columns = 0, rows = 0;
        if (!(input >> columns >> rows) || columns == 0 || rows == 0 || columns > 1024 || rows > 1024)
            throw std::runtime_error("Invalid packed-map region dimensions");
        TileMap result(static_cast<int>(columns * 4), static_cast<int>(rows * 4));
        for (int ry = 0; ry < static_cast<int>(rows); ++ry)
            for (int rx = 0; rx < static_cast<int>(columns); ++rx) {
                std::uint64_t packed;
                if (!(input >> packed) || packed > std::numeric_limits<std::uint32_t>::max())
                    throw std::runtime_error("Missing or invalid packed-map region");
                for (int y = 0; y < 4; ++y)
                    for (int x = 0; x < 4; ++x)
                        result.set({rx * 4 + x, ry * 4 + y}, (packed >> (2 * (y * 4 + x))) & 3);
            }
        std::string extra;
        if (input >> extra) throw std::runtime_error("Unexpected data after packed-map regions");
        if (input.bad()) throw std::runtime_error("Failed reading packed map");
        return result;
    }
    TileMap TileMap::load(const std::filesystem::path& path) {
        std::ifstream input(path);
        if (!input) throw std::runtime_error("Cannot open map: " + path.string());
        return read(input);
    }
    void TileMap::write(std::ostream& output) const {
        if (tiles.empty()) throw std::logic_error("Cannot save an empty map");
        output << width_ / 4 << ' ' << height_ / 4 << '\n';
        for (int ry = 0; ry < height_ / 4; ++ry) {
            for (int rx = 0; rx < width_ / 4; ++rx) {
                std::uint32_t packed = 0;
                for (int y = 0; y < 4; ++y)
                    for (int x = 0; x < 4; ++x)
                        packed |= static_cast<std::uint32_t>(at({rx * 4 + x, ry * 4 + y})) << (2 * (y * 4 + x));
                output << packed << ' ';
            }
            output << '\n';
        }
        if (!output) throw std::runtime_error("Failed writing packed map");
    }
    void TileMap::save(const std::filesystem::path& path) const {
        std::ofstream output(path);
        if (!output) throw std::runtime_error("Cannot write map: " + path.string());
        write(output);
        output.close();
        if (!output) throw std::runtime_error("Cannot finish writing map: " + path.string());
    }
}
