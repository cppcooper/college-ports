#pragma once

#include <word/gameplay/WordBox.h>

#include <optional>

namespace WordModel {
    struct FoundWord {
        std::string word;
        Position start;
        Position direction;
        bool revealed = false;
        Position end() const;
        std::vector<Position> cells() const;
    };
    class WordSearch {
        const WordBox& box;
        std::vector<FoundWord> found_;
        std::optional<std::size_t> selected_;
    public:
        explicit WordSearch(const WordBox& box) : box(box) {}
        void scan();
        void select_next();
        void select_previous();
        void toggle_solution();
        void toggle_all();
        const std::vector<FoundWord>& found() const { return found_; }
        std::optional<std::size_t> selected() const { return selected_; }
    };
}
