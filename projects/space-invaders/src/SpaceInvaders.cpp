#include <cheryl/assets/submission/draw2d.h>
#include <cheryl/backends/opengl/gl.h>
#include <cheryl/core/engine/engine-context.h>
#include <cheryl/core/game-framework/tick-context.h>
#include <cheryl/core/logging.h>
#include <cheryl/core/rendering/render-frame.h>
#include <cheryl/core/resources/asset-management/asset-loader.h>
#include <cheryl/text/font-collection.h>
#include <cheryl/text/font-service.h>

#include <cassert>
#include <chrono>
#include <fstream>
#include <ios>
#include <string>
#include <thread>

#include <GLFW/glfw3.h>

#include <space_invaders/SpaceInvaders.h>
#include <space_invaders/controllers/ui.h>

void SpaceInvaders::init_platform() {
    CE::Assets::Loading::Loader loader(asset_root);
    loader.load_assets(engine_context.resources());

    CE::Text::FontSelection font_selection;
    font_selection.automatic_system_fonts = true;
    font_selection.asset_directories = {asset_root};
    font_selection.preferred = {
        "ProggyVector",
        "Cantarell",
        "FreeSans",
        "Adwaita Mono",
        "FreeSerif",
        "DejaVu Sans"
    };
    engine_context.fonts().configure(font_selection);

    for (const auto& obj : controllers) {
        if (obj) obj->init_platform(engine_context);
    }
}

void SpaceInvaders::init_simulation() {
    for (const auto& obj: controllers) {
        if (obj) obj->init_simulation();
    }
    //todo: initialize actors
}

void SpaceInvaders::update(const CE::Game::TickContext& tick) {
    // UI currently runs after player; a pause request would arrive after player input.
    for (const auto& obj : controllers) {
        // The invasion slot is empty: this null dereference is undefined behavior.
        if (obj) obj->update(tick);
    }
    if (player) player->update(tick);
    invasion.update(tick);
}

void SpaceInvaders::prepare_render_frame(CE::Rendering::FrameWriter& frame) const {
    auto pass = frame.begin_pass(camera.projection_matrix(), camera.view_matrix());
    if (player) player->append(pass);
    invasion.append(pass);
    const CE::Rendering::Submission::Context2D context{pass.semantics(), pass.parameters(), pass.constraints()};
    if (auto* ui = dynamic_cast<const UIController*>(controllers[CtrID::ui].get())) {
        ui->append(frame);
    }
}

void OldSpaceInvaders::Load_Score() {
    std::ifstream File_Handle;
    //Open file
    File_Handle.open("highscore.txt", std::ios::in);

    //Check if it opened
    if (File_Handle.is_open()) {
        try {
            //Load data
            File_Handle >> highscore;

            //Check if loading data went bad
            if (File_Handle.fail())
                throw File_Handle.rdstate();

            File_Handle.close();
            //File has been loaded Okay
        } catch (...) {
            assert("Problem loading data from file");
        }
    } else {
        assert("Problem opening file- check file path");
    }
}

void OldSpaceInvaders::Save_Score() {
    std::ofstream File_Handle;
    //Open file
    File_Handle.open("highscore.txt", std::ios::out);

    //Check if it opened
    if (File_Handle.is_open()) {
        try {
            //Save data
            File_Handle << score;

            //Check if saving went bad
            if (File_Handle.fail())
                throw File_Handle.rdstate();

            File_Handle.close();
            //File has been saved Okay
        } catch (...) {
            assert("Problem loading data from file");
        }
    } else {
        assert("Problem opening file- check file path");
    }
}

void OldSpaceInvaders::Player_Death() {
    if (t1.joinable()) {
        t1.join();
    }
    t1 = std::thread //Makin' a Lambda Thread.. oya
    ([this] //Capturing 'THIS' allows our lambda to access this object's members/methods
        {
            //Decrement player_lives
            Var_Mutex.lock();
            if (--player_lives <= 0) {
                Var_Mutex.unlock();
                Game_Over();
            } else {
                Var_Mutex.unlock();
                //We want the player's sprite to stay dead for 1000ms
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                ThePlayer->unDeath();
            }
        }
    ); //End of constructor
}

void OldSpaceInvaders::Game_Over() {
    if (score > highscore) {
        Save_Score();
    }
    m_engine.Quit();
}

void OldSpaceInvaders::Increase_Score(int delta) {
    static unsigned int d = 0;
    d += delta;
    Var_Mutex.lock();
    score += (delta * player_lives);
    Var_Mutex.unlock();

#define Threshold 4700
    if ((d / Threshold) > 1) {
        Var_Mutex.lock();
        if (player_lives < 8000) {
            player_lives += int(d / Threshold);
        }
        Var_Mutex.unlock();
        d = d % Threshold;
    }
}
