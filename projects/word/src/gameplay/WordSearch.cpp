#include <word/gameplay/WordSearch.h>

#include <algorithm>
#include <array>
#include <set>
#include <tuple>

namespace WordModel {
    Position FoundWord::end() const {
        const auto steps = static_cast<int>(word.size()) - 1;
        return {start.x + direction.x * steps, start.y + direction.y * steps};
    }
    std::vector<Position> FoundWord::cells() const {
        std::vector<Position> result;
        for (int i = 0; i < static_cast<int>(word.size()); ++i)
            result.push_back({start.x + direction.x * i, start.y + direction.y * i});
        return result;
    }
    void WordSearch::scan() {
        found_.clear(); selected_.reset();
        struct TrieNode {
            std::array<int, 26> next;
            std::optional<std::size_t> answer;
            TrieNode() { next.fill(-1); }
        };
        std::vector<TrieNode> trie(1);
        for (std::size_t answer = 0; answer < box.words().size(); ++answer) {
            int node = 0;
            for (char ch : box.words()[answer]) {
                const auto letter = static_cast<std::size_t>(ch - 'A');
                if (trie[node].next[letter] == -1) {
                    const auto next = static_cast<int>(trie.size());
                    trie[node].next[letter] = next;
                    trie.emplace_back();
                }
                node = trie[node].next[letter];
            }
            trie[node].answer = answer;
        }
        const std::array directions{Position{1,0}, Position{-1,0}, Position{0,1}, Position{0,-1},
            Position{1,1}, Position{-1,-1}, Position{1,-1}, Position{-1,1}};
        std::vector<std::vector<FoundWord>> matches(box.words().size());
        std::set<std::tuple<std::size_t, Position, Position>> seen;
        for (int y = 0; y < box.height(); ++y)
            for (int x = 0; x < box.width(); ++x)
                for (auto direction : directions) {
                    int node = 0;
                    for (Position p{x,y}; box.contains(p); p = {p.x+direction.x, p.y+direction.y}) {
                        node = trie[node].next[static_cast<std::size_t>(box.letter(p)-'A')];
                        if (node == -1) break;
                        if (const auto answer = trie[node].answer) {
                            const Position start{x,y};
                            const auto first = std::min(start, p), last = std::max(start, p);
                            // A one-letter word or palindrome has one occurrence per
                            // set of cells, even when both directions match it.
                            if (seen.emplace(*answer, first, last).second)
                                matches[*answer].push_back({box.words()[*answer], start, direction});
                        }
                    }
                }
        for (auto& answer : matches)
            for (auto& match : answer) found_.push_back(std::move(match));
        if (!found_.empty()) selected_ = 0;
    }
    void WordSearch::select_next() {
        if (selected_ && *selected_ + 1 < found_.size()) ++*selected_;
    }
    void WordSearch::select_previous() {
        if (selected_ && *selected_ > 0) --*selected_;
    }
    void WordSearch::toggle_solution() {
        if (selected_) found_[*selected_].revealed = !found_[*selected_].revealed;
    }
    void WordSearch::toggle_all() {
        const bool show = std::ranges::any_of(found_, [](const FoundWord& word) { return !word.revealed; });
        for (auto& word : found_) word.revealed = show;
    }
}
