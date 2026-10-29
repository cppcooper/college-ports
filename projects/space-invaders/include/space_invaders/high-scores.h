#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <string>

struct HighScoreEntry {
    int score = 0;
    std::string name;
    bool operator==(const HighScoreEntry&) const = default;
};

class HighScores final {
public:
    static constexpr std::size_t capacity = 10;
    static constexpr std::size_t maximum_name_length = 24;
    using Table = std::array<HighScoreEntry, capacity>;

private:
    std::filesystem::path file;
    Table table{};

    void save(const Table& replacement) const;

public:
    explicit HighScores(std::filesystem::path file);
    void load();
    [[nodiscard]] const Table& entries() const { return table; }
    [[nodiscard]] bool qualifies(int score) const;
    // Saving must succeed before the in-memory table is replaced.
    bool add(int score, std::string name);
};
