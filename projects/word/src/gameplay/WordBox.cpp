#include <word/gameplay/WordBox.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <set>
#include <sstream>
#include <stdexcept>

namespace WordModel {
    namespace {
        char upper(char ch) {
            if (ch >= 'a' && ch <= 'z') ch = static_cast<char>(ch - 'a' + 'A');
            if (ch < 'A' || ch > 'Z') throw std::runtime_error("Puzzle letters and words must contain only A-Z");
            return ch;
        }
        bool whitespace(char ch) { return std::isspace(static_cast<unsigned char>(ch)) != 0; }
        std::vector<std::string> word_list(std::string text) {
            const auto opening = text.find_first_not_of(" \r\n\t");
            const auto closing = text.find_last_not_of(" \r\n\t\0", std::string::npos, 5);
            if (opening == std::string::npos || closing == std::string::npos || text[opening] != '{' || text[closing] != '}')
                throw std::runtime_error("Puzzle answer list must be enclosed in braces");
            text = text.substr(opening + 1, closing - opening - 1);
            if (std::ranges::all_of(text, whitespace)) return {};
            std::vector<std::string> words;
            std::size_t start = 0;
            do {
                const auto comma = text.find(',', start);
                const auto end = comma == std::string::npos ? text.size() : comma;
                auto word = text.substr(start, end - start);
                const auto first = std::find_if_not(word.begin(), word.end(), whitespace);
                const auto last = std::find_if_not(word.rbegin(), word.rend(), whitespace).base();
                if (first >= last) throw std::runtime_error("Puzzle answer list contains an empty word");
                words.emplace_back(first, last);
                if (comma == std::string::npos) break;
                start = comma + 1;
            } while (true);
            return words;
        }
    }
    WordBox::WordBox(int width, int height, std::string letters, std::vector<std::string> words)
    : width_(width), height_(height), letters_(std::move(letters)) {
        if (width <= 0 || height <= 0 || width > 256 || height > 256 || letters_.size() != static_cast<std::size_t>(width)*height)
            throw std::runtime_error("Puzzle dimensions must be 1..256 and match its letter count");
        for (auto& ch : letters_) ch = upper(ch);
        if (words.size() > 4096) throw std::runtime_error("Puzzle has more than 4096 answer words");
        std::set<std::string> unique;
        for (auto& word : words) {
            if (word.empty() || word.size() > 256) throw std::runtime_error("Answer word length must be 1..256");
            for (auto& ch : word) ch = upper(ch);
            if (unique.insert(word).second) words_.push_back(std::move(word));
        }
    }
    bool WordBox::contains(Position p) const { return p.x >= 0 && p.y >= 0 && p.x < width_ && p.y < height_; }
    char WordBox::letter(Position p) const { return contains(p) ? letters_[p.y * width_ + p.x] : '\0'; }
    WordBox WordBox::read_text(std::istream& input) {
        std::string magic;
        int width = 0, height = 0;
        if (!(input >> magic >> width >> height) || magic != "WORDSEARCH" || width <= 0 || height <= 0 || width > 256 || height > 256)
            throw std::runtime_error("Text puzzle must start with WORDSEARCH followed by width and height (1..256)");
        std::string letters;
        for (int y = 0; y < height; ++y) {
            std::string row;
            if (!(input >> row) || row.size() != static_cast<std::size_t>(width)) throw std::runtime_error("Puzzle row has the wrong width");
            letters += row;
        }
        std::string answers{std::istreambuf_iterator<char>(input), {}};
        if (input.bad()) throw std::runtime_error("Cannot read puzzle answer list");
        return WordBox(width, height, std::move(letters), word_list(std::move(answers)));
    }
    WordBox WordBox::read_legacy(std::span<const unsigned char> bytes) {
        if (bytes.size() < 2) throw std::runtime_error("Legacy puzzle is missing dimensions");
        const int width = bytes[0], height = bytes[1];
        const auto count = static_cast<std::size_t>(width) * height;
        if (width == 0 || height == 0 || bytes.size() < count + 2) throw std::runtime_error("Legacy puzzle grid is truncated or empty");
        std::string letters(bytes.begin()+2, bytes.begin()+2+count);
        std::string answers(bytes.begin()+2+count, bytes.end());
        return WordBox(width, height, std::move(letters), word_list(std::move(answers)));
    }
    WordBox WordBox::load(const std::filesystem::path& path) {
        std::ifstream input(path, std::ios::binary | std::ios::ate);
        if (!input) throw std::runtime_error("Cannot open puzzle: " + path.string());
        const auto length = input.tellg();
        if (length < 0 || length > 2 * 1024 * 1024) throw std::runtime_error("Puzzle exceeds the 2 MiB input limit");
        std::vector<unsigned char> bytes(static_cast<std::size_t>(length));
        input.seekg(0);
        if (!input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())))
            throw std::runtime_error("Cannot read puzzle: " + path.string());
        const std::string magic = "WORDSEARCH";
        if (bytes.size() >= magic.size() && std::equal(magic.begin(), magic.end(), bytes.begin())) {
            std::istringstream text(std::string(bytes.begin(), bytes.end()));
            return read_text(text);
        }
        return read_legacy(bytes);
    }
}
