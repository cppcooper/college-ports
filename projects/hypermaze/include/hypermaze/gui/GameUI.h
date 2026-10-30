#pragma once

#include <college_ports/game-ui.h>
#include <hypermaze/world/HyperMaze.h>

class HypermazeUI final : public CollegePorts::GameUI {
    tgui::Label::Ptr readout;
    tgui::Label::Ptr help;
    bool debug_visible = true;
    void create_widgets() override;
    void release_widgets() override { readout.reset(); help.reset(); }

public:
    using CollegePorts::GameUI::GameUI;
    void show_world(const HypermazeModel::HyperMaze& maze, std::uint32_t seed);
    void toggle_debug();
};
