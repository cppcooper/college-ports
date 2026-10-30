#pragma once

#include <college_ports/game-ui.h>
#include <word/gameplay/WordSearch.h>
#include <TGUI/Widgets/ScrollablePanel.hpp>

class WordUI final : public CollegePorts::GameUI {
    const WordModel::WordBox& box;
    const WordModel::WordSearch& search;
    tgui::Label::Ptr title, help;
    tgui::ScrollablePanel::Ptr words, grid;
    std::vector<tgui::Label::Ptr> letters, results;
    int width = 0, height = 0;
    void create_widgets() override;
    void release_widgets() override;
public:
    WordUI(CE::Runtime::Context& engine, const WordModel::WordBox& box, const WordModel::WordSearch& search)
    : CollegePorts::GameUI(engine), box(box), search(search) {}
    void resize(int logical_width, int logical_height);
    void show_state();
};
