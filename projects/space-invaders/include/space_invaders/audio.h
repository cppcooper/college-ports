#pragma once

#include <cheryl/core/subsystems/event-bus.h>

#include <cstddef>
#include <filesystem>
#include <memory>

class GameAudio final {
    struct State;
    std::unique_ptr<State> state;

public:
    GameAudio();
    ~GameAudio();
    GameAudio(const GameAudio&) = delete;
    GameAudio& operator=(const GameAudio&) = delete;

    void load(const std::filesystem::path& asset_root, std::size_t maximum_enemy_volley);
    void start(CE::Events::EventBus& events);
    void update(bool paused, double game_speed);
    void stop();
    void close();
};
