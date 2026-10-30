#pragma once

#include <cheryl/core/game-framework/abstract-game.h>
#include <word/controllers/controls.h>
#include <word/gui/GameUI.h>

class WordGame final : public CE::Game::AbstractGame {
    CE::Runtime::Context& engine;
    std::filesystem::path assets;
    WordModel::WordBox box;
    WordModel::WordSearch search;
    WordController controller;
    WordUI ui;
public:
    WordGame(CE::Runtime::Context& engine, std::filesystem::path assets, const std::filesystem::path& puzzle);
    void init_platform() override;
    void init_simulation() override;
    void update(const CE::Game::TickContext& tick) override;
    void prepare_render_frame(CE::Rendering::FrameWriter& frame) const override;
    void deinit_platform() override;
};
