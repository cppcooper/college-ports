#pragma once

#include <cheryl/assets/types/2d/graphic.h>
#include <cheryl/assets/types/2d/tileset.h>
#include <cheryl/core/game-framework/abstract-game.h>
#include <cheryl/core/rendering/camera.h>
#include <hypermaze/controllers/player.h>
#include <hypermaze/gui/GameUI.h>
#include <hypermaze/world/HyperMaze.h>

#include <filesystem>
#include <map>

class HypermazeGame final : public CE::Game::AbstractGame {
    static constexpr float tile_size = 32;
    CE::Runtime::Context& engine;
    std::filesystem::path asset_root;
    std::uint32_t seed;
    HypermazeModel::HyperMaze maze;
    PlayerController controller;
    HypermazeUI ui;
    CE::Rendering::Camera2D camera;
    std::shared_ptr<const CE::Assets::Tilesets::Asset> tiles;
    std::optional<CE::Assets::Graphics::Asset> player;
    std::map<HypermazeModel::Position, HypermazeModel::Tile> map_tiles;

    void refresh_visibility();

public:
    HypermazeGame(CE::Runtime::Context& engine, std::filesystem::path assets, std::uint32_t seed);
    void init_platform() override;
    void init_simulation() override;
    void update(const CE::Game::TickContext& tick) override;
    void prepare_render_frame(CE::Rendering::FrameWriter& frame) const override;
    void deinit_platform() override;
};
