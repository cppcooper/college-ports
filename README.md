# college-ports

Four college projects ported to Cheryl Engine 4: Space Invaders, Hypermaze, Rover,
and Word Search. Each uses Cheryl startup, action contexts, simulation lifecycle
hooks, retained render frames, and a TGUI HUD.

Use CMake 3.28+, Ninja, and a C++23 compiler. CMake first looks for an installed
Cheryl SDK using normal package discovery. For an installation outside the system
prefixes, set `Cheryl_DIR` or `CMAKE_PREFIX_PATH` in your CLion CMake profile.

With an installed SDK, configure and build directly:

```sh
cmake --preset shared-debug
cmake --build --preset shared-debug
```

If no SDK is found, CMake exposes the SDK setup targets. Build **cheryl-sdk** in
CLion, then use **Reload CMake Project** to enable the game targets and headers.
The command-line equivalent is:

```sh
cmake --preset shared-debug
cmake --build --preset shared-debug --target cheryl-sdk
cmake --preset shared-debug
cmake --build --preset shared-debug
```

The default project is `space-invaders`. Set `COLLEGE_PORTS_PROJECTS` to `all`,
`hypermaze`, `rover`, `space-invaders`, `word`, or a quoted semicolon-separated list.
Executables go in the build directory's `bin/` folder. For Release, use
`shared-release` in the same commands.

Build all four games and run the model tests:

```sh
cmake --preset shared-debug -DCOLLEGE_PORTS_PROJECTS=all
cmake --build --preset shared-debug
ctest --test-dir builds/shared-debug --output-on-failure
```

The new ports share small asset, input, and UI helpers under `projects/common/`.
Each game's `src/` and `include/<game>/` directories separate its application,
gameplay model, controllers, and GUI. Model tests live under `tests/`; map and
puzzle files live under `data/`. The [porting record](PORTING.md) describes the
layout and the three implementation stages.

The three newer ports run from any directory using CMake's assets and data
defaults. Override assets with `--assets PATH`; every executable accepts `--help`. Add `--concurrent`
to run simulation on a worker, or `--fixed` to use fixed simulation steps.

| Executable | Controls and options |
| --- | --- |
| `hypermaze` | WASD, arrows, or gamepad D-pad move; R generates a new maze; F3 toggles the debug HUD; Esc quits. `--seed 42` selects a repeatable maze. |
| `rover` | Space pauses search; N advances one operation; R restarts; +/- changes speed; E switches between search and editor; Esc quits. `--map PATH`, `--start X Y`, `--goal X Y`, and `--steps-per-second N` configure the search. |
| `word_search` | Arrows or D-pad select a match; Space or gamepad A toggles its solution; A key toggles all solutions; R rescans; Esc quits. Mouse wheel/scrollbars navigate the puzzle and match list. `--puzzle PATH` loads another puzzle. |

For example:

```sh
builds/shared-debug/bin/hypermaze --seed 42
builds/shared-debug/bin/rover --editor
builds/shared-debug/bin/word_search --puzzle projects/word/data/default.word
```

Rover's editor uses WASD to scroll, 1/2/3 to select rock/hole/dirt, and the left
mouse button to paint. Shift toggles spray painting; Up/Down changes brush radius.
F5 saves to the build directory's `projects/rover/data/edited.map`; use
`--save-map PATH` to choose another destination. The original maps are in
`projects/rover/data/`. Search uses goal-directed depth-first traversal, advances
within a bounded per-tick budget, and reports both found and unreachable goals.

Word Search accepts the original binary `.wrd` format: two unsigned dimension
bytes, row-major ASCII letters, and a brace-enclosed comma-separated answer list.
It also accepts text files beginning with `WORDSEARCH width height`, followed by
one letter row per line and `{ANSWER,WORDS}`. Letters and answers are normalized
to uppercase. Dimensions are limited to 256 per axis and files to 2 MiB. The
included sample has eight words, one in each search direction. Repeated and
overlapping occurrences are retained; reverse matches over the same cells of a
palindrome count once. Rescanning resets the selection and solution highlights.

The model tests cover maze connectivity/traversal, original packed-map loading,
search reachability, editor bounds, and puzzle parsing/scanning. Interactive
graphics, input, resizing, and sequential/concurrent runtime play remain manual
checks.

Set **CHERYL_ENGINE_SOURCE_DIR** in your CLion CMake profile to use an existing
buildable Cheryl checkout:

```text
-DCHERYL_ENGINE_SOURCE_DIR=/absolute/path/to/cheryl-engine
```

The default is the `extern/cheryl-engine` submodule. SDK setup initializes that
submodule and its dependencies on demand; configuring against an installed SDK
does not download or build Cheryl. An existing custom checkout must already have
its dependencies initialized.

The submodule's Git entry is the source of the required commit. **cheryl-sdk**
checks the selected checkout's `HEAD` against that entry. If it differs, build
**cheryl-source-checkout** to check out the recorded commit, then build
**cheryl-sdk**. The checkout target refuses to switch a checkout with tracked
changes. System-installed SDKs are discovered independently of this source check.

To change the required source revision, select the desired commit in the submodule
and stage `extern/cheryl-engine` in Git. Then align your custom checkout if needed,
build **cheryl-sdk**, and use
[Reload CMake Project](https://www.jetbrains.com/help/clion/reloading-project.html).
There is no separate version or commit setting in CMake.

The CMake files have these responsibilities:

| File | Purpose |
| --- | --- |
| [CMakeLists.txt](CMakeLists.txt) | Select games and load the installed SDK. |
| [cmake/Cheryl.cmake](cmake/Cheryl.cmake) | Discover SDKs, define the source path, and expose the SDK setup targets. |
| [cmake/InitializeCheryl.cmake](cmake/InitializeCheryl.cmake) | Initialize the default submodule and check or align the configured source commit. |
| [cmake/GameRuntime.cmake](cmake/GameRuntime.cmake) | Copy CMake-known runtime DLLs on Windows. |
| `projects/common/` | Share asset submission, action-binding construction, and the TGUI scene lifecycle between the three newer ports. |
| `projects/<game>/CMakeLists.txt` | Declare that game's sources, include directory, linked libraries, and executable location. |

Cheryl owns its build-option defaults; this project does not repeat or override
`CHERYL_*` options. The SDK setup supplies only the configuration, local install
prefix, compiler/toolchain, build tool, and CMake 4 compatibility for older
dependencies. Building the SDK requires
[Cheryl's native dependencies](https://github.com/cppcooper/cheryl-engine/blob/fixes-develop/docs/site/development/building.md#system-dependencies);
Linux installation also requires `patchelf`.

Build files live under `builds/cheryl/<configuration>/`, with a separate build
directory for each configured source path. The local installation is in
`builds/cheryl/<configuration>/sdk/`, including headers in `sdk/include/`.
Repeated builds reuse the compiled libraries. Normal SDK discovery takes priority
over this local fallback. CMake caches its selected package; set `Cheryl_DIR` or
reset the CMake cache when switching SDK installations.

Each game declares its dependencies in `target_link_libraries()`. All four use
`Cheryl::OpenGL::Startup` and `Cheryl::UI::TGUI`, which supplies both Cheryl's TGUI
integration and native `<TGUI/...>` widget headers. Hypermaze, Rover and Word
Search receive these through `college_ports_common`. Space Invaders also links
`Cheryl::Audio::Miniaudio` and `Cheryl::UI::RmlUi` for audio and its menu.

To use another library available in the SDK, add its target to the game's
`target_link_libraries()` list, for example `Cheryl::Audio::Miniaudio`.

CMake supplies build-time runtime paths on Linux/macOS. Windows builds copy
CMake-known runtime DLLs beside each executable; native dependencies without
runtime-location metadata still need their DLL directories on `PATH`.

Existing `CHERYL_*` cache settings in a game build no longer configure Cheryl;
the separate SDK build owns its configuration.
