#pragma once

#include <compare>
#include <filesystem>
#include <iosfwd>
#include <span>
#include <string>
#include <vector>

namespace WordModel {
    struct Position {
        int x = 0;
        int y = 0;
        auto operator<=>(const Position&) const = default;
    };
    class WordBox {
        int width_, height_;
        std::string letters_;
        std::vector<std::string> words_;
    public:
        WordBox(int width, int height, std::string letters, std::vector<std::string> words);
        static WordBox read_text(std::istream& input);
        static WordBox read_legacy(std::span<const unsigned char> bytes);
        static WordBox load(const std::filesystem::path& path);
        int width() const { return width_; }
        int height() const { return height_; }
        bool contains(Position p) const;
        char letter(Position p) const;
        const std::vector<std::string>& words() const { return words_; }
    };
}
