#pragma once

#include <algorithm>

inline double game_speed = 1.0;

// Color and bullet scaling reach their maximum at 10; game speed stays uncapped.
[[nodiscard]] inline double game_speed_fraction() noexcept {
    return std::clamp((game_speed - 1.0) / 9.0, 0.0, 1.0);
}
