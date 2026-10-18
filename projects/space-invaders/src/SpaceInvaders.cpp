#include <space_invaders/SpaceInvaders.h>
#include <cheryl/core/logging.h>

#include "core/resources/asset-management/asset-loader.h"
#include "core/resources/asset-management/sprite-mgr.h"
#include "templates/asset-mgr.h"

namespace SIControls {
    constexpr CE::Input::Actions::Id MoveLeft{1};
    constexpr CE::Input::Actions::Id MoveRight{2};
    constexpr CE::Input::Actions::Id Fire{3};
    constexpr CE::Input::Actions::Id Menu{4};
} // namespace De


void SpaceInvaders::init() {
    auto& input = engine_context.input();
    auto& bindings = input.bindings();

    using CE::Input::KeyboardKey;
    const std::array<CE::Input::Control,6> controls{{
        {input.keyboard_id(), KeyboardKey::A},
        {input.keyboard_id(), KeyboardKey::Left},
        {input.keyboard_id(), KeyboardKey::D},
        {input.keyboard_id(), KeyboardKey::Right},
        {input.keyboard_id(), KeyboardKey::Space},
        {input.keyboard_id(), KeyboardKey::Escape}
    }};
    binding_ids.insert(binding_ids.end(), {
        bindings.bind_button(controls[0], SIControls::MoveLeft),
        bindings.bind_button(controls[1], SIControls::MoveLeft),
        bindings.bind_button(controls[2], SIControls::MoveRight),
        bindings.bind_button(controls[3], SIControls::MoveRight),
        bindings.bind_button(controls[4], SIControls::Fire),
        bindings.bind_button(controls[5], SIControls::Menu)
    });

    const std::filesystem::path assets_root = "/mnt/projects/assets";
    CE::Assets::Loading::Loader loader(assets_root);
    loader.load_assets(engine_context.resources());
    //todo: move to invasion/enemy code: auto invaders_spritesheet = CE::Assets::Sprites::Manager::get().get_assets("invaders");

    CE::Text::FontSelection font_selection;
    font_selection.automatic_system_fonts = true;
    font_selection.asset_directories = {assets_root};
    font_selection.preferred = {
        "ProggyVector",
        "Cantarell",
        "FreeSans",
        "Adwaita Mono",
        "FreeSerif",
        "DejaVu Sans"
    };
    auto fonts = CE::Text::FontCollection::load(font_selection);

}

void SpaceInvaders::init_simulation() {}

void OldSpaceInvaders::DeInit(void) {
    if (t1.joinable()) {
        t1.join();
    }
    screen.Clean_Up(true); //We need the quasi-overloaded version to synchronize each object with the main thread

    delete ThePlayer;
    //logger::FileLog_Mgr::Stop();
}

void OldSpaceInvaders::Update(double& seconds) {
    if (t1.joinable()) {
        t1.join();
    }
    Updating = true;
    LOGFILE_1(_DEBUG1) << seconds;
    static double seconds_remainder = 0;
    seconds += seconds_remainder;
    enum { loops = 10 };
    if (seconds > (loops * timeslice)) {
        seconds = (loops * timeslice);
    }
    while (seconds >= timeslice) {
        if (!Drawing) {
            LOGFILE_1(_INFO) << "Running Updates. Seconds: " << seconds;
            seconds -= timeslice;
            if (moveL) {
                ThePlayer->MoveLeft(timeslice);
            }
            if (moveR) {
                ThePlayer->MoveRight(timeslice);
            }
            if (shoots) {
                bullet_mgr.Player_Shoots();
            }
            //These three lines update our world
            screen.Update_Objects(timeslice); //Update individual entities
            if (ThePlayer->playerbullet) {
                ThePlayer->playerbullet->Update(timeslice);
            }

            invasion_mgr.Update(timeslice); //Update invaders as a whole
            bullet_mgr.Update(timeslice); //Update bullet positions and perform collisions
            bullet_mgr.Clean_Up();
            invasion_mgr.Clean_Up();

            needsDraw = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }
    }
    Updating = false;
    while (Drawing) {}
    shoots = false;
    seconds_remainder = seconds;
}

void OldSpaceInvaders::Draw(void) {
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    do {
        if (needsDraw) {
            needsDraw = false;
            Drawing = true;
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            screen.Draw();
            ThePlayer->Draw();
            if (ThePlayer->playerbullet) {
                ThePlayer->playerbullet->Draw();
            }

            glClear(GL_DEPTH_BUFFER_BIT);

            //Lives - Top Right
            Var_Mutex.lock();
            std::string text = std::to_string(player_lives) + "x                  ";
            Var_Mutex.unlock();

            float y = Screen::Height() - 50;
            float textWidth;
            textWidth = m_font->WidthText(text);
            m_font->Print(Screen::Width() - textWidth, y, text);

            //Score - Top Middle
            text = "      Score :     " + std::to_string(score);
            textWidth = m_font->WidthText(text);
            m_font->Print(Screen::Width() / 2, y, text);

            //High Score - Top Left
            text = "High Score :    " + std::to_string(highscore) + "    ";
            textWidth = m_font->WidthText(text);
            m_font->Print(50.f, y, text);
            Drawing = false;

            glfwSwapBuffers(m_engine.m_window);
        }
    } while (Updating);
    screen.Clean_Up();
}

//the key codes/actions/mods for DoInput are from GLFW: check its documentation for their values
void OldSpaceInvaders::DoInput(int& key, int& scancode, int& action, int& mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                Game_Over(); //start the shutdown sequence
                break;

            case GLFW_KEY_LEFT:
            case GLFW_KEY_A:
                moveL = true;
                break;

            case GLFW_KEY_RIGHT:
            case GLFW_KEY_D:
                moveR = true;
                break;

            case GLFW_KEY_LEFT_SHIFT:
            case GLFW_KEY_LEFT_CONTROL:
            case GLFW_KEY_SPACE:
                shoots = true;
                break;
        }
    }
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_LEFT:
            case GLFW_KEY_A:
                moveL = false;
                break;

            case GLFW_KEY_RIGHT:
            case GLFW_KEY_D:
                moveR = false;
                break;

            case GLFW_KEY_LEFT_SHIFT:
            case GLFW_KEY_LEFT_CONTROL:
            case GLFW_KEY_SPACE:
                //shoots = false;
                break;
        }
    }
}

void OldSpaceInvaders::DoMouse(int& button, int& action, int& mod) {}

void OldSpaceInvaders::DoCursor(double& x, double& y) {}

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
