#include <rover/actors/rover.h>

#include <cstdlib>
#include <stdexcept>

namespace RoverModel {
    namespace {
        Position offset(Direction d) {
            switch (d) {
                case Direction::Down: return {0, -1};
                case Direction::Right: return {1, 0};
                case Direction::Up: return {0, 1};
                case Direction::Left: return {-1, 0};
            }
            throw std::invalid_argument("Invalid rover direction");
        }
    }
    std::array<Direction, 4> Rover::search_order(Position p) const {
        const bool x_major = std::abs(p.x - goal_.x) > std::abs(p.y - goal_.y);
        const auto toward_x = p.x > goal_.x ? Direction::Left : Direction::Right;
        const auto toward_y = p.y > goal_.y ? Direction::Down : Direction::Up;
        const auto away_x = p.x > goal_.x ? Direction::Right : Direction::Left;
        const auto away_y = p.y > goal_.y ? Direction::Up : Direction::Down;
        return x_major ? std::array{toward_x, toward_y, away_y, away_x}
                       : std::array{toward_y, toward_x, away_x, away_y};
    }
    void Rover::cancel() {
        stack.clear(); path_.clear(); moves_ = 0; status_ = SearchStatus::Idle;
        map.clear_visits();
    }
    void Rover::start(Position start, Position goal) {
        if (!map.can_move(start) || !map.can_move(goal))
            throw std::invalid_argument("Rover start and goal must be traversable map tiles");
        cancel();
        goal_ = goal;
        status_ = SearchStatus::Searching;
        visit(start);
    }
    void Rover::visit(Position p) {
        position_ = p;
        map.set(p, 3);
        ++moves_;
        stack.push_back({p, search_order(p)});
        if (p == goal_) {
            status_ = SearchStatus::Found;
            for (const auto& frame : stack) path_.push_back(frame.position);
            stack.clear();
        }
    }
    void Rover::step() {
        auto& frame = stack.back();
        if (frame.next == frame.order.size()) {
            stack.pop_back();
            if (stack.empty()) status_ = SearchStatus::Unreachable;
            else position_ = stack.back().position;
            return;
        }
        facing_ = frame.order[frame.next++];
        const auto next = frame.position + offset(facing_);
        if (map.can_move(next) && map.at(next) != 3) visit(next);
    }
    void Rover::advance(std::size_t budget) {
        while (budget-- > 0 && status_ == SearchStatus::Searching) step();
    }
}
