#pragma once

#include <cheryl/assets/types/2d/sprite.h>
#include <cheryl/assets/types/2d/tileset.h>
#include <cheryl/core/game-framework/abstract-game.h>
#include <cheryl/core/rendering/camera.h>
#include <rover/actors/rover.h>
#include <rover/controllers/controls.h>
#include <rover/gui/GameUI.h>
#include <rover/levels/viewport.h>

struct RoverOptions {
    std::filesystem::path assets, map, save;
    std::optional<RoverModel::Position> start, goal;
    bool editor = false;
    double steps_per_second = 600;
};
class RoverGame final : public CE::Game::AbstractGame {
    CE::Runtime::Context& engine;
    RoverOptions options;
    RoverModel::TileMap map;
    RoverModel::Rover rover;
    RoverModel::MapEditor editor;
    RoverModel::Viewport viewport;
    RoverModel::Position start, goal;
    RoverController controller;
    RoverUI ui;
    CE::Rendering::Camera2D camera;
    std::shared_ptr<const CE::Assets::Tilesets::Asset> tiles;
    std::array<std::shared_ptr<const CE::Assets::Sprites::Asset>, 4> sprites;
    bool editing, paused = false;
    double credit = 0, pan_cooldown = 0;
    std::string notice;
    void start_search();
public:
    RoverGame(CE::Runtime::Context& engine, RoverOptions options);
    void init_platform() override;
    void init_simulation() override;
    void update(const CE::Game::TickContext& tick) override;
    void prepare_render_frame(CE::Rendering::FrameWriter& frame) const override;
    void deinit_platform() override;
};
