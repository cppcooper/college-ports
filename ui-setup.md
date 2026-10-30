# Game UI

All four games use TGUI for their HUDs. Space Invaders also uses RmlUi for its
menu. The RML/RCSS HUD templates in the assets submodule remain available, but the
current games construct their HUD widgets in C++.

| Game | UI implementation | Display |
| --- | --- | --- |
| Space Invaders | [GameUI](projects/space-invaders/src/gui/GameUI.cpp), [HUD](projects/space-invaders/include/space_invaders/gui/HUD.h), [menu](projects/space-invaders/src/gui/game-menu.cpp) | Score, lives, game over, and menu/high scores. |
| Hypermaze | [GameUI](projects/hypermaze/src/gui/GameUI.cpp) | Player/traversal coordinates, current space, seed, and graph IDs. |
| Rover | [GameUI](projects/rover/src/gui/GameUI.cpp) | Recursion level, move count, search/editor state, and controls. |
| Word Search | [GameUI](projects/word/src/gui/GameUI.cpp) | Scrollable match list and puzzle, selection arrows, and solution highlights. |

## Ownership and lifecycle

The three newer ports share [the TGUI lifecycle helper](projects/common/include/college_ports/game-ui.h),
following Space Invaders' split between platform resources and simulation widgets.

1. `init_platform()` retains the platform submission endpoint and constructs the
   `SceneUploader`. Games configure font selection on this owner first.
2. `init_simulation()` creates the session with `automatic_view = true` and creates
   the game's widgets once. Widget changes happen on the simulation owner.
3. Each update advances UI time, adopts a completed scene, and submits a recording
   when no upload is pending. The last complete scene stays available while the
   next recording uploads. Upload failures propagate through the runtime's normal
   exception and cleanup path.
4. `prepare_render_frame()` appends the retained scene after world passes. It
   performs no native graphics work or widget mutation.
5. Sessions, widgets and upload dependencies remain alive through runtime
   quiescence. `deinit_platform()` releases scenes and widgets, calls
   `close_after_quiescence()`, and releases the session/uploader and endpoint.
   These are application hooks; destructors do not replace runtime cleanup.

This arrangement supports both sequential and concurrent simulation. Input action
contexts attach and detach on platform; controllers query only copied tick input.
The current tick's view dimensions drive world cameras and custom layout. No game
queries the native window from the simulation worker.

## Widgets and input

Hypermaze and Rover have passive labels and backgrounds. Their controllers process
gameplay actions independently of the HUD. Hypermaze can hide its debug panel;
Rover keeps counters and editor/save feedback in retained labels.

Word Search passes the ordered tick records into its TGUI session to handle native
scrollbars and mouse-wheel input. Its controller handles match selection and reveal
actions. The puzzle and match panels scroll independently. Grid cells remain
readable when the puzzle is larger than the available space; resizing updates cell
sizes and panel dimensions without rebuilding the session. Revealed cells are
green, and the selected revealed match is amber.

Space Invaders retains its own mixed-toolkit composition, including menu input
routing and deferred new-game/quit requests. Its menu keeps updating while the
world is paused.

## Fonts and dependencies

The new ports configure Cheryl's font service with the selected `--assets`
directory and system-font fallback. Their preferred family is DejaVu Sans; the
assets submodule contains the font and its license. Word Search requires fonts
without loading unrelated world graphics.

`college_ports_common` links `Cheryl::UI::TGUI`, supplying Cheryl's adapter and
native `<TGUI/...>` widget headers. Space Invaders declares its TGUI and RmlUi
links directly. See [the repository README](README.md) for SDK setup, controls,
data files, and validation commands.
