#include <hypermaze/world/Terrain.h>

#include <random>

namespace HypermazeModel {
    bool Terrain::contains(Position p) const { return p.x >= 0 && p.y >= 0 && p.x < width && p.y < height; }
    Tile Terrain::at(Position p) const { return contains(p) ? tiles[p.y * width + p.x] : Tile::Blank; }
    bool Terrain::can_move(Position p) const {
        switch (at(p)) {
            case Tile::Blank: case Tile::Wall1: case Tile::Wall2:
            case Tile::DoorClosed: case Tile::GateClosed: return false;
            default: return true;
        }
    }
    Terrain Terrain::room(const Node& node, std::uint32_t seed) {
        Terrain terrain{node.width(), node.height(), {}};
        std::mt19937 random(seed);
        for (int y = 0; y < terrain.height; ++y)
            for (int x = 0; x < terrain.width; ++x) {
                const bool wall = x == 0 || y == 0 || x == terrain.width-1 || y == terrain.height-1;
                terrain.tiles.push_back(static_cast<Tile>((wall ? 3 : 1) + random() % 2));
            }
        for (auto d : {Direction::Left, Direction::Right, Direction::Up, Direction::Down})
            for (std::size_t i = 0; i < node.wall(d).size(); ++i) {
                const auto p = node.doorway(d, i);
                terrain.tiles[p.y * terrain.width + p.x] = Tile::DoorOpen;
            }
        return terrain;
    }
    Terrain Terrain::corridor(const Link& link, std::uint32_t seed) {
        const bool vertical = link.direction == Direction::Up || link.direction == Direction::Down;
        Terrain terrain{vertical ? 3 : link.length, vertical ? link.length : 3, {}};
        std::mt19937 random(seed);
        for (int y = 0; y < terrain.height; ++y)
            for (int x = 0; x < terrain.width; ++x) {
                const bool wall = vertical ? x != 1 : y != 1;
                terrain.tiles.push_back(wall ? static_cast<Tile>(3 + random() % 2) : vertical ? Tile::Vertical : Tile::Horizontal);
            }
        return terrain;
    }
}
