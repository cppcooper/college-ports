#pragma once

#include <college_ports/game-ui.h>
#include <rover/actors/rover.h>
#include <rover/levels/map-editor.h>

class RoverUI final : public CollegePorts::GameUI {
    tgui::Label::Ptr counters, status, help;
    void create_widgets() override;
    void release_widgets() override { counters.reset(); status.reset(); help.reset(); }
public:
    using CollegePorts::GameUI::GameUI;
    void show_state(const RoverModel::Rover& rover, const RoverModel::MapEditor& editor,
        bool editing, bool paused, double speed, const std::string& notice);
};
