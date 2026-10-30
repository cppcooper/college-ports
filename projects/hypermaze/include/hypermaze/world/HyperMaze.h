#pragma once

#include <hypermaze/world/Visibility.h>

#include <map>
#include <optional>

namespace HypermazeModel {
    class HyperMaze {
        struct Region {
            Position origin;
            std::size_t index;
            bool corridor;
        };
        Graph graph_;
        std::vector<std::optional<Terrain>> rooms;
        std::vector<std::optional<Terrain>> corridors;
        std::map<Position, Tile> discovered;
        Position player_{1, 1};
        Position origin_{};
        Position traversal_{};
        Direction travel_ = Direction::Up;
        std::size_t node_ = 0;
        std::optional<std::size_t> link_;
        bool corridor_ = false;
        std::uint32_t seed_;
        Terrain& room(std::size_t node);
        Terrain& corridor(std::size_t link);
        Region current_region() const;
        std::vector<Region> adjacent_regions();
        Terrain& terrain(const Region& region);
        bool cross_boundary(Region& region, Direction direction, Position from);
        void place(const Terrain& terrain);
        void traverse(Direction direction, Position next);

    public:
        explicit HyperMaze(std::uint32_t seed = 1);
        explicit HyperMaze(Graph graph, std::uint32_t seed = 1);
        bool move(Direction direction);
        const Terrain& terrain() const;
        Tile tile(Position p) const;
        // Reveal the current region and rooms one connection away. Farther
        // terrain requires sight from the player; looking never marks it visited.
        VisibleTiles visible_tiles(int columns, int rows);
        const Graph& graph() const { return graph_; }
        Position player() const { return player_; }
        Position origin() const { return origin_; }
        Position traversal() const { return traversal_; }
        Direction travel_direction() const { return travel_; }
        std::size_t current_node() const { return node_; }
        std::optional<std::size_t> current_link() const { return link_; }
        bool in_corridor() const { return corridor_; }
        std::size_t discovered_tiles() const { return discovered.size(); }
    };
}
