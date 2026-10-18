# college-ports

College projects being ported to Cheryl Engine 4. The CMake configuration supports
all four projects; their legacy application code is still being ported.

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

Each game explicitly lists its dependencies in `target_link_libraries()`. All four
use `Cheryl::OpenGL::Startup`; Space Invaders also uses `Cheryl::UI::TGUI`, which
supplies both Cheryl's TGUI integration and the native `<TGUI/...>` widget headers.

To use another library available in the SDK, add its target to the game's
`target_link_libraries()` list, for example `Cheryl::Audio::Miniaudio`.

CMake supplies build-time runtime paths on Linux/macOS. Windows builds copy
CMake-known runtime DLLs beside each executable; native dependencies without
runtime-location metadata still need their DLL directories on `PATH`.

Existing `CHERYL_*` cache settings in a game build no longer configure Cheryl;
the separate SDK build owns its configuration.
