#include <hypermaze/world/HyperMaze.h>
#include "VisibilityRay.h"

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
    std::vector<HyperMaze::Region> HyperMaze::adjacent_regions() {
        std::vector<Region> adjacent;
        const auto active = current_region();
        const auto add_room = [this, &adjacent](Region passage, Direction direction) {
            const auto end = passage.origin + corridor_end(terrain(passage), direction);
            if (!cross_boundary(passage, direction, end)) throw std::logic_error("Corridor has no room endpoint");
            adjacent.push_back(passage);
        };
        if (active.corridor) {
            const auto direction = graph_.links.at(active.index).direction;
            add_room(active, direction);
            add_room(active, opposite(direction));
        } else {
            const auto& node = graph_.nodes.at(active.index);
            for (auto direction : {Direction::Left, Direction::Right, Direction::Up, Direction::Down})
                for (std::size_t index = 0; index < node.wall(direction).size(); ++index) {
                    auto passage = active;
                    const auto door = active.origin + node.doorway(direction, index);
                    if (!cross_boundary(passage, direction, door)) throw std::logic_error("Room has no corridor doorway");
                    adjacent.push_back(passage);
                    add_room(passage, direction);
                }
        }
        return adjacent;
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
    VisibleTiles HyperMaze::visible_tiles(int columns, int rows) {
        const auto adjacent = adjacent_regions();
        return VisibilityDetail::field_of_view(player_, columns, rows, [this, &adjacent](Position target) {
            const auto local = target - origin_;
            if (terrain().contains(local)) return terrain().at(local);
            TileLookup sample = [this, region = current_region(), current = player_](Position p) mutable {
                if (!terrain(region).contains(p - region.origin)) {
                    const auto delta = p - current;
                    Direction direction;
                    if (delta == Position{1, 0}) direction = Direction::Right;
                    else if (delta == Position{-1, 0}) direction = Direction::Left;
                    else if (delta == Position{0, 1}) direction = Direction::Up;
                    else if (delta == Position{0, -1}) direction = Direction::Down;
                    else return Tile::Blank;
                    if (!cross_boundary(region, direction, current)) return Tile::Blank;
                }
                current = p;
                return terrain(region).at(p - region.origin);
            };
            // Player sight takes precedence where graph placements overlap.
            // Revealed neighboring rooms never become additional ray origins.
            const auto seen = VisibilityDetail::trace_ray(player_, target, sample);
            if (seen != Tile::Blank) return seen;
            for (const auto& region : adjacent) {
                const auto p = target - region.origin;
                if (terrain(region).contains(p)) return terrain(region).at(p);
            }
            return Tile::Blank;
        });
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
