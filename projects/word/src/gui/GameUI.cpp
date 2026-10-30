#include <word/gui/GameUI.h>

#include <algorithm>
#include <array>
#include <format>
#include <set>

namespace {
    tgui::Label::Ptr panel_label(tgui::ScrollablePanel& panel, const std::string& text, unsigned int size) {
        auto label = tgui::Label::create(text);
        label->setTextSize(size);
        label->setAutoSize(true);
        label->setIgnoreMouseEvents(true);
        label->getRenderer()->setTextColor(tgui::Color::White);
        panel.add(label);
        return label;
    }
}
void WordUI::create_widgets() {
    title = label("", 24);
    ui().set_layout(title, {.offset = {.fixed = {16, 12}}});
    help = label("Arrows / D-pad: select   Space / A button: reveal   A key: reveal all\nR: rescan   Mouse: scroll puzzle or word list   Esc: quit", 16);
    ui().set_layout(help, {.anchor = {0, 1}, .offset = {.fixed = {16, -12}}});
    words = tgui::ScrollablePanel::create();
    grid = tgui::ScrollablePanel::create();
    for (const auto& panel : {words, grid}) {
        panel->getRenderer()->setBackgroundColor({24, 30, 42});
        ui().gui().add(panel);
    }
    for (int y = 0; y < box.height(); ++y)
        for (int x = 0; x < box.width(); ++x) {
            auto widget = panel_label(*grid, std::string(1, box.letter({x,y})), 24);
            widget->setAutoSize(false);
            widget->setHorizontalAlignment(tgui::HorizontalAlignment::Center);
            widget->setVerticalAlignment(tgui::VerticalAlignment::Center);
            widget->getScrollbar()->setPolicy(tgui::Scrollbar::Policy::Never);
            letters.push_back(widget);
        }
    std::set<std::string> matched;
    for (const auto& match : search.found()) {
        auto widget = panel_label(*words, match.word, 18);
        widget->setPosition(8, 8 + static_cast<float>(results.size()) * 28);
        results.push_back(widget);
        matched.insert(match.word);
    }
    int y = 8 + static_cast<int>(results.size()) * 28;
    if (results.empty()) {
        auto empty = panel_label(*words, "No matches found", 18);
        empty->setPosition(8, y); y += 36;
    }
    for (const auto& answer : box.words()) {
        if (matched.contains(answer)) continue;
        auto missing = panel_label(*words, answer + " (not found)", 16);
        missing->getRenderer()->setTextColor({175, 185, 200});
        missing->setPosition(8, y); y += 28;
    }
    resize(1100, 760);
    show_state();
}
void WordUI::resize(int logical_width, int logical_height) {
    if (width == logical_width && height == logical_height) return;
    width = logical_width; height = logical_height;
    const float sidebar = std::clamp(width * 0.22f, 100.f, 240.f);
    const float panel_height = std::max(48.f, height - 124.f);
    const float grid_width = std::max(48.f, width - sidebar - 48.f);
    words->setPosition(16, 56); words->setSize(sidebar, panel_height);
    grid->setPosition(sidebar + 32, 56); grid->setSize(grid_width, panel_height);
    const float cell = std::clamp(std::min((grid_width-20)/box.width(), (panel_height-20)/box.height()), 24.f, 48.f);
    grid->setContentSize({box.width() * cell + 8, box.height() * cell + 8});
    for (int y = 0; y < box.height(); ++y)
        for (int x = 0; x < box.width(); ++x) {
            auto& widget = letters[y * box.width() + x];
            widget->setPosition(4 + x * cell, 4 + y * cell);
            widget->setSize(cell, cell);
            widget->setTextSize(static_cast<unsigned int>(cell * 0.65f));
        }
    help->setMaximumTextWidth(std::max(48.f, width - 32.f));
}
void WordUI::show_state() {
    std::set<std::string> matched;
    std::vector<int> highlights(letters.size());
    for (std::size_t i = 0; i < search.found().size(); ++i) {
        const auto& match = search.found()[i];
        matched.insert(match.word);
        const bool selected = search.selected() == i;
        results[i]->setText(selected ? "-> " + match.word + " <-" : match.word);
        results[i]->getRenderer()->setTextColor(selected ? tgui::Color{255, 215, 115} : tgui::Color::White);
        if (!match.revealed) continue;
        for (auto p : match.cells()) {
            auto& highlight = highlights[p.y * box.width() + p.x];
            highlight = std::max(highlight, selected ? 2 : 1);
        }
    }
    for (std::size_t i = 0; i < letters.size(); ++i) {
        const std::array colors{tgui::Color::Transparent, tgui::Color{35, 110, 80}, tgui::Color{145, 100, 20}};
        letters[i]->getRenderer()->setBackgroundColor(colors[highlights[i]]);
    }
    title->setText(std::format("Word Search | {} / {} words | {} matches", matched.size(), box.words().size(), search.found().size()));
    if (const auto selected = search.selected()) {
        const auto scrollbar = words->getVerticalScrollbar();
        const float top = static_cast<float>(scrollbar->getValue());
        const float bottom = top + words->getInnerSize().y - 28;
        const float y = 8 + static_cast<float>(*selected) * 28;
        if (y < top) scrollbar->setValue(static_cast<unsigned int>(y));
        else if (y + 28 > bottom) scrollbar->setValue(static_cast<unsigned int>(std::max(0.f, y + 28 - (bottom-top))));
    }
}
void WordUI::release_widgets() {
    letters.clear(); results.clear();
    title.reset(); help.reset(); words.reset(); grid.reset();
}
