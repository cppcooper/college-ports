#pragma once

#include <cheryl.h>
#include <cheryl/core/resources/asset-management/sprite-mgr.h>
#include <TGUI/Widgets/Label.hpp>
#include <TGUI/Widgets/Picture.hpp>

#include <array>
#include <format>
#include <future>
#include <memory>
#include <stdexcept>

class HUD final {
    CE::Input::System& input;
    CE::UI::TGUI::Session& session;

    CE::Runtime::PlatformDispatcher::Submission platform;
    CE::UI::TGUI::SceneUploader uploader;
    CE::UI::TGUI::Scene scene;
    std::future<CE::UI::TGUI::Scene> pending;

    std::array<tgui::Picture::Ptr, 3> lives;
    tgui::Label::Ptr score;
    int extra_lives = 0;
    int score_value = 0;

public:
    explicit HUD(CE::Runtime::Context& engine, const std::unique_ptr<CE::UI::TGUI::Session>& session)
    : input(engine.input()),
      session(*session),
      platform(engine.platform_dispatcher().submission()),
      uploader(engine.resources()) {
        create_widgets();
    }

    void set_score(int value) { score_value = value; }

    // The game supplies its reserve-life count; the HUD only displays it.
    void update(const CE::Game::TickContext& tick) {
        score->setText(std::format("Score: {}", score_value));
        for (std::size_t i = 0; i < lives.size(); ++i)
            lives[i]->setVisible(i < extra_lives);

        // pending is never assigned, so this never publishes a HUD-owned scene.
        static_cast<void>(CE::UI::TGUI::adopt_scene(pending, scene));
    }

    // scene stays empty, so this append currently draws nothing.
    void append(CE::Rendering::FrameWriter& frame) const { scene.write(frame); }

    void close_after_quiescence() {
        score.reset();
        for (auto& life : lives)
            life.reset();
        // Also closes UIController's borrowed session; later UI calls then throw failed_operation.
        session.close_after_quiescence();
        scene = CE::UI::TGUI::Scene{};
    }

private:
    void create_widgets() {
        score = tgui::Label::create("Score: 0");
        score->setTextSize(24);
        score->setSize({220, 32});
        session.gui().add(score);
        session.set_layout(score, {
            .anchor = {0, 0},
            .offset = {.fixed = {24, 16}},
        });

        // The loaded manifest selects the normal player cell independently
        // of the player class. Cell 1 is the death frame.
        const auto player = CE::Assets::Sprites::Manager::get().get_asset("invaders", "player");
        if (!player)
            throw std::runtime_error("Load the invaders:player sprite before updating the HUD");
        const auto& definition = player->definition();
        const auto cell = definition.grid.cell_rect(0);
        const tgui::Texture texture{
            tgui::String{definition.texture.u8string()},
            {static_cast<unsigned int>(cell.x), static_cast<unsigned int>(cell.y), cell.width, cell.height},
            {}, false
        };
        const tgui::Vector2f icon_size{cell.width * 0.5f, cell.height * 0.5f};
        constexpr float gap = 8;
        for (std::size_t i = 0; i < lives.size(); ++i) {
            auto& life = lives[i];
            life = tgui::Picture::create(texture);
            life->setSize(icon_size);
            life->setIgnoreMouseEvents(true);
            life->setVisible(false);
            session.gui().add(life);
            session.set_layout(life, {
                .anchor = {1, 0},
                .offset = {.fixed = {-24 - static_cast<float>(i) * (icon_size.x + gap), 16}},
            });
        }
    }
};
