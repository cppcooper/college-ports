#include <word/gameplay/WordSearch.h>

#include <array>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <tuple>

using namespace WordModel;

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
template<class Callback> void rejects(Callback callback) {
    bool rejected = false;
    try { callback(); } catch (const std::exception&) { rejected = true; }
    check(rejected, "Malformed puzzle accepted");
}
using Match = std::tuple<std::string, Position, Position>;
std::set<Match> naive(const WordBox& box) {
    std::set<Match> result;
    for (const auto& word : box.words())
        for (int y = 0; y < box.height(); ++y)
            for (int x = 0; x < box.width(); ++x)
                for (int dy = -1; dy <= 1; ++dy)
                    for (int dx = -1; dx <= 1; ++dx) {
                        if (!dx && !dy) continue;
                        bool matches = true;
                        for (int i = 0; i < static_cast<int>(word.size()); ++i)
                            if (box.letter({x+dx*i, y+dy*i}) != word[i]) { matches = false; break; }
                        if (matches) {
                            const Position start{x,y}, end{x+dx*(static_cast<int>(word.size())-1), y+dy*(static_cast<int>(word.size())-1)};
                            result.emplace(word, std::min(start,end), std::max(start,end));
                        }
                    }
    return result;
}
int main() {
    for (auto direction : {Position{1,0}, Position{-1,0}, Position{0,1}, Position{0,-1},
                           Position{1,1}, Position{-1,-1}, Position{1,-1}, Position{-1,1}}) {
        const Position start{direction.x < 0 ? 4 : 1, direction.y < 0 ? 4 : 1};
        std::string letters(36, 'X');
        const std::string word = "ABCDE";
        for (int i = 0; i < 5; ++i) letters[(start.y+direction.y*i)*6+start.x+direction.x*i] = word[i];
        WordBox box(6,6,letters,{word});
        WordSearch search(box); search.scan();
        check(search.found().size() == 1 && search.found()[0].start == start && search.found()[0].direction == direction,
            "Search missed an orientation or returned wrong coordinates");
    }
    WordBox overlap(5,1,"ababa",{"aba","BAB","ABABABA","A","MISSING"});
    WordSearch search(overlap); search.scan();
    check(search.found().size() == 6, "Overlapping matches or palindrome deduplication failed");
    search.select_previous(); check(search.selected() == 0, "Selection underflow");
    for (int i = 0; i < 100; ++i) search.select_next();
    check(search.selected() == 5, "Selection overflow");
    search.toggle_solution(); check(search.found().back().revealed, "Selected reveal failed");
    search.toggle_all();
    for (const auto& match : search.found()) check(match.revealed, "Reveal all failed");
    search.toggle_all();
    for (const auto& match : search.found()) check(!match.revealed, "Hide all failed");
    search.scan(); check(search.found().size() == 6 && search.selected() == 0, "Rescan accumulated results or retained selection");

    WordBox absent(1,1,"A",{"B"}); WordSearch empty(absent); empty.scan();
    empty.select_next(); empty.select_previous(); empty.toggle_solution(); empty.toggle_all();
    check(empty.found().empty() && !empty.selected(), "Empty results cannot be selected safely");
    WordBox duplicates(2,1,"AA",{"A","a"}); WordSearch singles(duplicates); singles.scan();
    check(duplicates.words().size() == 1 && singles.found().size() == 2, "One-letter matches or duplicate answer handling failed");

    std::istringstream text("WORDSEARCH 3 2\nabc\ndef\n{ab, fed}\n");
    const auto parsed = WordBox::read_text(text);
    const std::vector<unsigned char> bytes{3,2,'a','b','c','d','e','f','{','a','b',',','f','e','d','}',0};
    const auto legacy = WordBox::read_legacy(bytes);
    check(parsed.words() == legacy.words() && parsed.letter({2,1}) == legacy.letter({2,1}), "Legacy puzzle compatibility failed");
    WordSearch legacy_search(legacy); legacy_search.scan();
    check(legacy_search.found().size() == 2, "Loaded puzzle search failed");
    check(legacy.letter({-1,0}) == '\0' && legacy.letter({3,0}) == '\0', "Puzzle boundary lookup failed");
    for (const auto* bad : {"", "WORDSEARCH -1 1", "WORDSEARCH 257 1", "WORDSEARCH 2 2\nAB\nC\n{}",
        "WORDSEARCH 1 1\n1\n{}", "WORDSEARCH 1 1\nA\n{A,}", "WORDSEARCH 1 1\nA\n{A", "WORDSEARCH 1 1\nA\n{A}junk"})
        rejects([&] { std::istringstream input(bad); WordBox::read_text(input); });
    rejects([] { WordBox::read_legacy(std::vector<unsigned char>{}); });
    rejects([] { WordBox::read_legacy(std::vector<unsigned char>{0,1}); });
    rejects([] { WordBox::read_legacy(std::vector<unsigned char>{4,4,'A'}); });

    for (unsigned seed = 0; seed < 32; ++seed) {
        std::mt19937 random(seed);
        std::string letters(30, 'A');
        for (auto& ch : letters) ch = static_cast<char>('A' + random()%3);
        WordBox box(5,6,letters,{"A","AA","AB","ABA","ABC","BAC","CAB","CA","BBBB"});
        WordSearch trie(box); trie.scan();
        std::set<Match> actual;
        for (const auto& match : trie.found()) actual.emplace(match.word,std::min(match.start,match.end()),std::max(match.start,match.end()));
        check(actual == naive(box), "Trie search disagrees with independent exhaustive scan");
    }
    const auto sample = WordBox::load(WORD_DEFAULT_PUZZLE);
    WordSearch example(sample); example.scan();
    check(sample.words().size() == 8 && example.found().size() == 8, "Sample puzzle does not contain its eight answers");
}
