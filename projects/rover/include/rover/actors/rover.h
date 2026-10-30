#pragma once

#include <rover/levels/tile-map.h>

#include <array>
#include <cstddef>

namespace RoverModel {
    enum class Direction { Down, Right, Up, Left };
    enum class SearchStatus { Idle, Searching, Found, Unreachable };
    class Rover {
        struct Frame {
            Position position;
            std::array<Direction, 4> order;
            std::size_t next = 0;
        };
        TileMap& map;
        std::vector<Frame> stack;
        std::vector<Position> path_;
        Position position_{};
        Position goal_{};
        Direction facing_ = Direction::Down;
        SearchStatus status_ = SearchStatus::Idle;
        std::size_t moves_ = 0;
        std::array<Direction, 4> search_order(Position position) const;
        void visit(Position position);
        void step();

    public:
        explicit Rover(TileMap& map) : map(map) {}
        void start(Position start, Position goal);
        void cancel();
        void advance(std::size_t budget);
        Position position() const { return position_; }
        Position goal() const { return goal_; }
        Direction facing() const { return facing_; }
        SearchStatus status() const { return status_; }
        std::size_t total_moves() const { return moves_; }
        std::size_t recursion_level() const { return stack.size(); }
        const std::vector<Position>& path() const { return path_; }
    };
}
