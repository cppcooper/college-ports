# Remaining Cheryl 4 ports

The implementation follows Space Invaders: application startup creates an injected
runtime context; platform hooks load assets and attach input; simulation hooks own
gameplay and widgets; frame preparation copies draws; platform cleanup releases
resources after runtime quiescence. No game owns another engine or detached thread.

## Layout chosen before implementation

```text
projects/common/
  CMakeLists.txt
  include/college_ports/{assets,controls,game-ui}.h
projects/hypermaze/
  src/{main,Hypermaze}.cpp
  src/world/{Graph,Terrain,HyperMaze}.cpp
  src/controllers/player.cpp
  src/gui/GameUI.cpp
  include/hypermaze/Hypermaze.h
  include/hypermaze/world/{Graph,Terrain,HyperMaze}.h
  include/hypermaze/controllers/player.h
  include/hypermaze/gui/GameUI.h
  tests/world.cpp
projects/rover/
  src/{main,RoverGame}.cpp
  src/levels/{tile-map,map-editor,viewport}.cpp
  src/actors/rover.cpp
  src/controllers/controls.cpp
  src/gui/GameUI.cpp
  include/rover/ (matching headers)
  data/Map{1,2,3}.map
  tests/pathfinding.cpp
projects/word/
  src/{main,WordGame}.cpp
  src/gameplay/{WordBox,WordSearch}.cpp
  src/controllers/controls.cpp
  src/gui/GameUI.cpp
  include/word/ (matching headers)
  data/default.word
  tests/search.cpp
```

Common headers contain only asset submission, input-binding construction and the
retained TGUI scene lifecycle. Gameplay stays within each project. Existing assets
and engine submodules are consumed without modification. Legacy engine copies and
obsolete scene-graph adapters are removed when their replacement is integrated;
historical project documents remain. Default assets/data paths are supplied by
CMake, with command-line overrides.

## Three autonomous stages

1. **Hypermaze**
   1. Port graph generation, room/corridor terrain and traversal with owned storage
      and signed coordinates; keep loops and persistent discovered terrain.
   2. Integrate startup, player action context, camera, sprites and debug HUD.
   3. Build and verify graph connectivity, collision and traversal round trips.
2. **Rover**
   1. Port packed map loading/saving, viewport/editor and goal-directed depth-first
      search into bounded simulation steps.
   2. Integrate startup, search/editor controls, tile/rover rendering and counters.
   3. Build and verify packed-map round trips, reachability, dead ends and edits.
3. **Word Search**
   1. Port puzzle loading and scanning in all eight directions, including the
      original binary `.wrd` format; supply a text puzzle because none is present.
   2. Integrate startup, selection/reveal controls and a responsive puzzle HUD.
   3. Build all games and verify parsing, repeated/overlapping matches, direction
      coverage, empty results and selection boundaries; update usage documentation.

Validation uses deterministic model tests and build/CLI checks. Interactive game
testing remains a separate manual check.

## Progress

- Stage 1 complete: Hypermaze executable builds; model tests pass across all
  corridor directions, self-loops and 64 generated mazes; `--help` passes.
- Stage 2 complete: Rover builds and its CLI check passes. Tests verify packed
  maps, bounded search against independent reachability on all three original
  maps, restart/cancel, unreachable goals, brush clipping and viewport edges.
- Stage 3 complete: Word Search builds, its CLI check passes, and model tests
  cover all eight directions, overlap/palindromes, legacy and text parsing,
  malformed input, empty results and selection/reveal/reset behavior. Trie results
  also match an independent exhaustive scan on 32 generated puzzles.
- Integration validation: the Debug build succeeds with all four games selected;
  all three CTest suites pass. Rover's relocated maps match their original bytes.
  README and UI documentation describe the implemented layout and controls.
- Interactive graphics, input, resizing and concurrent/sequential play were not
  exercised; these remain manual checks.
