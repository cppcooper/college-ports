#pragma once

#include <cheryl/assets/submission/draw2d.h>
#include <cheryl/core/engine/engine-context.h>
#include <cheryl/core/rendering/render-frame.h>
#include <cheryl/core/resources/asset-management/asset-loader.h>
#include <cheryl/core/resources/asset-management/sprite-mgr.h>
#include <cheryl/core/resources/asset-management/tileset-mgr.h>
#include <cheryl/text/font-service.h>

#include <glm/gtc/matrix_transform.hpp>

#include <filesystem>
#include <stdexcept>

namespace CollegePorts {
    inline void configure_fonts(CE::Runtime::Context& engine, const std::filesystem::path& root) {
        CE::Text::FontSelection fonts;
        fonts.automatic_system_fonts = true;
        fonts.asset_directories = {root};
        fonts.preferred = {"DejaVu Sans", "Cantarell", "FreeSans"};
        engine.fonts().configure(fonts);
    }
    inline void load_assets(CE::Runtime::Context& engine, const std::filesystem::path& root) {
        CE::Assets::Loading::Loader loader(root);
        loader.load_assets(engine.resources());
        configure_fonts(engine, root);
    }
    inline auto tileset(const std::string& space, const std::string& name) {
        auto asset = CE::Assets::Tilesets::Manager::get().get_asset(space + ":" + name);
        if (!asset) throw std::runtime_error("Missing tileset " + space + ":" + name);
        return asset;
    }
    inline auto sprite(const std::string& space, const std::string& name) {
        auto asset = CE::Assets::Sprites::Manager::get().get_asset(space, name);
        if (!asset) throw std::runtime_error("Missing sprite " + space + ":" + name);
        return asset;
    }
    inline CE::Rendering::DrawStyle2D draw_style(glm::vec2 position, glm::vec2 scale, float alpha = 1) {
        CE::Rendering::DrawStyle2D style;
        style.model_matrix = glm::scale(
            glm::translate(glm::mat4{1}, glm::vec3{position, 0}), glm::vec3{scale, 1});
        style.alpha = alpha;
        return style;
    }
    inline void append_tile(
        CE::Rendering::PassWriter& pass, const CE::Assets::Tilesets::Asset& asset,
        CE::Assets::Grids::CellIndex cell, glm::vec2 position, glm::vec2 size
    ) {
        const auto& frame = asset.definition().grid.frame;
        pass.add(CE::Rendering::Submission::resolve_tile(asset, cell,
            draw_style(position, size / glm::vec2{frame.width, frame.height}),
            {pass.semantics(), pass.parameters(), pass.constraints()}));
    }
    inline void append_sprite(
        CE::Rendering::PassWriter& pass, const CE::Assets::Sprites::Asset& asset,
        glm::vec2 position, glm::vec2 size
    ) {
        const auto& frame = asset.definition().grid.frame;
        pass.add(CE::Rendering::Submission::resolve_sprite(asset, 0,
            draw_style(position, size / glm::vec2{frame.width, frame.height}),
            {pass.semantics(), pass.parameters(), pass.constraints()}));
    }
}
