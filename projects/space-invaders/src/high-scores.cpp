#include <space_invaders/high-scores.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {
    std::string checked_name(std::string name) {
        const auto first = name.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
            throw std::invalid_argument("Enter a name before saving your score.");
        name = name.substr(first, name.find_last_not_of(" \t\r\n") - first + 1);
        std::size_t characters = 0;
        for (const unsigned char byte : name) {
            if (byte < 32 || byte == 127)
                throw std::invalid_argument("Names cannot contain control characters.");
            if ((byte & 0xc0) != 0x80)
                ++characters;
        }
        if (characters > HighScores::maximum_name_length)
            throw std::invalid_argument("Use a name of at most 24 characters.");
        return name;
    }
}

HighScores::HighScores(std::filesystem::path file) : file(std::move(file)) {}

bool HighScores::qualifies(const int score) const {
    return score > 0 && score > table.back().score;
}

void HighScores::load() {
    Table loaded{};
    if (!std::filesystem::exists(file)) {
        save(loaded);
        table = std::move(loaded);
        return;
    }
    std::ifstream input(file);
    if (!input)
        throw std::runtime_error("Cannot read high scores: " + file.string());

    std::string line;
    std::size_t count = 0;
    bool legacy = false;
    while (std::getline(input, line)) {
        if (line.find_first_not_of(" \t\r") == std::string::npos)
            continue;
        if (count == capacity)
            throw std::runtime_error("The high-score file contains more than 10 entries.");
        std::istringstream row(line);
        HighScoreEntry entry;
        if (!(row >> entry.score) || entry.score < 0)
            throw std::runtime_error("Invalid score on high-score line " + std::to_string(count + 1));
        row >> std::ws;
        if (row.eof() && count == 0) {
            // Preserve the original single-number highscore.txt when upgrading.
            entry.name = entry.score > 0 ? "Anonymous" : "";
            legacy = true;
        } else {
            if (legacy || !(row >> std::quoted(entry.name)))
                throw std::runtime_error("Invalid name in the high-score file.");
            row >> std::ws;
            if (!row.eof())
                throw std::runtime_error("Unexpected data in the high-score file.");
            if (entry.score > 0 || !entry.name.empty())
                entry.name = checked_name(std::move(entry.name));
        }
        loaded[count++] = std::move(entry);
    }
    if (input.bad())
        throw std::runtime_error("Cannot finish reading high scores: " + file.string());
    std::stable_sort(loaded.begin(), loaded.end(), [](const auto& left, const auto& right) { return left.score > right.score; });
    if (legacy || count != capacity)
        save(loaded);
    table = std::move(loaded);
}

void HighScores::save(const Table& replacement) const {
    if (const auto parent = file.parent_path(); !parent.empty())
        std::filesystem::create_directories(parent);
    auto temporary = file;
    temporary += ".tmp";
    try {
        std::ofstream output(temporary, std::ios::trunc);
        if (!output)
            throw std::runtime_error("Cannot write the high-score file.");
        for (const auto& entry : replacement)
            output << entry.score << ' ' << std::quoted(entry.name) << '\n';
        output.close();
        if (!output)
            throw std::runtime_error("Cannot finish writing the high-score file.");
        std::filesystem::rename(temporary, file);
    } catch (...) {
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        throw;
    }
}

bool HighScores::add(const int score, std::string name) {
    if (!qualifies(score))
        return false;
    name = checked_name(std::move(name));
    auto replacement = table;
    const auto position = std::find_if(replacement.begin(), replacement.end(), [score](const auto& entry) { return entry.score < score; });
    std::move_backward(position, replacement.end() - 1, replacement.end());
    *position = HighScoreEntry{score, std::move(name)};
    save(replacement);
    table = std::move(replacement);
    return true;
}
