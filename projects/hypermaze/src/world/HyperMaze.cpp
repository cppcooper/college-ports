#include <hypermaze/world/HyperMaze.h>

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace HypermazeModel {
    namespace {
        Position corridor_end(const Terrain& terrain, Direction direction) {
            switch (direction) {
                case Direction::Left: return {0, 1};
                case Direction::Right: return {terrain.width - 1, 1};
                case Direction::Up: return {1, terrain.height - 1};
                case Direction::Down: return {1, 0};
            }
            throw std::invalid_argument("Invalid corridor direction");
        }
    }

    HyperMaze::HyperMaze(std::uint32_t seed) : HyperMaze(Graph::generate(seed), seed) {}
    HyperMaze::HyperMaze(Graph graph, std::uint32_t seed)
    : graph_(std::move(graph)), rooms(graph_.nodes.size()), corridors(graph_.links.size()), seed_(seed) {
        if (graph_.nodes.empty()) throw std::invalid_argument("Maze needs a root room");
        place(room(0));
    }
    Terrain& HyperMaze::room(std::size_t node) {
        if (!rooms.at(node)) rooms[node] = Terrain::room(graph_.nodes.at(node), seed_ + static_cast<std::uint32_t>(node));
        return *rooms[node];
    }
    Terrain& HyperMaze::corridor(std::size_t link) {
        if (!corridors.at(link)) corridors[link] = Terrain::corridor(graph_.links.at(link), seed_ + static_cast<std::uint32_t>(link));
        return *corridors[link];
    }
    const Terrain& HyperMaze::terrain() const {
        return corridor_ ? *corridors.at(*link_) : *rooms.at(node_);
    }
    HyperMaze::Region HyperMaze::current_region() const {
        return {origin_, corridor_ ? *link_ : node_, corridor_};
    }
    Terrain& HyperMaze::terrain(const Region& region) {
        return region.corridor ? corridor(region.index) : room(region.index);
    }
    void HyperMaze::place(const Terrain& terrain) {
        for (int y = 0; y < terrain.height; ++y)
            for (int x = 0; x < terrain.width; ++x)
                discovered[origin_ + Position{x, y}] = terrain.at({x, y});
    }
    Tile HyperMaze::tile(Position p) const {
        const auto found = discovered.find(p);
        return found == discovered.end() ? Tile::Blank : found->second;
    }
    bool HyperMaze::move(Direction direction) {
        const auto next = player_ + offset(direction);
        const auto local = next - origin_;
        if (terrain().contains(local)) {
            if (!terrain().can_move(local)) return false;
        } else {
            if (!terrain().can_move(player_ - origin_)) return false;
            traverse(direction, next);
        }
        player_ = next;
        travel_ = direction;
        return true;
    }
    void HyperMaze::traverse(Direction direction, Position next) {
        auto region = current_region();
        if (!cross_boundary(region, direction, player_)) throw std::logic_error("Maze boundary has no doorway");
        origin_ = region.origin;
        corridor_ = region.corridor;
        if (corridor_) link_ = region.index;
        else node_ = region.index;
        place(terrain(region));
        traversal_ = next;
    }
    bool HyperMaze::cross_boundary(Region& region, Direction direction, Position from) {
        const auto local = from - region.origin;
        const auto next = from + offset(direction);
        if (!region.corridor) {
            const auto& node = graph_.nodes.at(region.index);
            const auto& doors = node.wall(direction);
            std::size_t index = 0;
            while (index < doors.size() && node.doorway(direction, index) != local) ++index;
            if (index == doors.size()) return false;
            const auto link = doors[index].link;
            auto& target = corridor(link);
            const auto entry = corridor_end(target, opposite(direction));
            region = {next - entry, link, true};
        } else {
            const auto& link = graph_.links.at(region.index);
            if ((direction != link.direction && direction != opposite(link.direction))
                || !terrain(region).can_move(local)) return false;
            const std::size_t side = direction == link.direction ? 1 : 0;
            const auto wall = side == 0 ? link.direction : opposite(link.direction);
            const auto node_index = link.nodes.at(side);
            const auto& node = graph_.nodes.at(node_index);
            const auto& doors = node.wall(wall);
            const auto door = std::find(doors.begin(), doors.end(), Door{region.index, side});
            if (door == doors.end()) throw std::logic_error("Corridor endpoint has no room door");
            region = {next - node.doorway(wall, static_cast<std::size_t>(door - doors.begin())), node_index, false};
        }
        return true;
    }
}
