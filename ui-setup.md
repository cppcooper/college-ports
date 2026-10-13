# HUD setup

The HUD documents and styles live in the `assets` submodule. These files recreate the existing displays; the games still need the C++ hookup described below.

| Project | Files | Display |
| --- | --- | --- |
| Space Invaders | [RML](assets/graphics/ui/space-invaders/hud.rml), [RCSS](assets/graphics/ui/space-invaders/hud.rcss) | High score at left, score in the middle, lives at right. |
| Rover | [RML](assets/graphics/ui/rover/hud.rml), [RCSS](assets/graphics/ui/rover/hud.rcss) | Original translucent panel at bottom-left, with recursion and move counters. |
| Word | [RML](assets/graphics/ui/word/hud.rml), [RCSS](assets/graphics/ui/word/hud.rcss) | Found-word list, selection arrows, puzzle letters and solution highlights. |
| Hypermaze | [RML](assets/graphics/ui/hypermaze/hud.rml), [RCSS](assets/graphics/ui/hypermaze/hud.rcss) | Eight-line debug readout at top-left. |

All four displays are passive. They have `pointer-events: none` and need no click listeners or keyboard focus. Existing gameplay actions continue to control them.

## Module selection

Configure with `CHERYL_BUILD_UI_RMLUI=ON`, then link each consuming executable to the module. For example:

```cmake
target_link_libraries(space_invaders PRIVATE Cheryl::UI::RmlUi)
```

The module uses the bundled RmlUi source or a compatible configured dependency. Its [dependency guide](../cheryl-engine/projects/modules/ui/rmlui/README.md#selection-and-dependency) describes source/package selection. `Session` owns RmlUi initialization and shutdown; application code does not need a separate `Rml::Initialise()` call.

## Create the UI once

Keep one session, document and cached element pointers for the game lifetime. Initialization belongs to startup. Updates change data, dimensions and recorded render state; they do not reload the document or recreate the session.

With the current Cheryl lifecycle, `AbstractGame::init()` runs on the platform/graphics thread. In sequential mode, `update()` runs there too, so creating the session directly in `init()` works. In concurrent mode, `update()` runs on the simulation worker. `Session` remembers its creating thread and rejects ordinary calls from another thread, so a platform-created session cannot then be used by concurrent `update()`.

To initiate UI setup from `init()` in either mode, use the existing simulation dispatcher. Queue creation once from `init()`; the runtime executes that callback on the simulation owner before its first actual update. This keeps the initialization request in `init()` and preserves the session's thread ownership.

The snippets below illustrate this arrangement. `set_ui_dispatcher`, `start_hud` and `make_hud_materials` are application helpers to add to the game. The existing engine API does not need changing for this arrangement.

After constructing the runtime, pass its submission endpoint to the game before calling `run()`:

```cpp
Game game(*result.engine, asset_root);
auto runtime = result.make_runtime(game);
game.set_ui_dispatcher(runtime.simulation_dispatcher().submission());
runtime.run();
```

Store that endpoint in `std::optional<CE::Engine::SimulationDispatcher::Submission>`: the endpoint has no public default constructor. The game also retains a `std::future<void>` for startup completion, its `Session`, document/element pointers, `SceneUploader`, `Materials`, current `Scene`, pending scene future, and the platform submission endpoint. The game and engine remain alive through `runtime.run()` returning or throwing.

```cpp
void Game::set_ui_dispatcher(CE::Engine::SimulationDispatcher::Submission submission) {
    ui_dispatcher_ = std::move(submission);
}

void Game::init() {
    // Graphics resources are constructed on the platform owner.
    uploader_ = std::make_unique<CE::UI::RmlUi::SceneUploader>(engine_.resources());
    platform_ = engine_.platform_dispatcher().submission();
    materials_ = make_hud_materials(engine_.resources(), asset_root_);

    if (!ui_dispatcher_)
        throw std::runtime_error("HUD simulation dispatcher is missing");

    // Copy dimensions here; the callback must not query the live window.
    const auto logical = engine_.window().logical_size();
    const auto framebuffer = engine_.window().framebuffer_size();
    ui_start_ = ui_dispatcher_->submit([this, logical, framebuffer] {
        start_hud(logical, framebuffer);
    });
}
```

The simulation-owner helper creates the session and loads the document. For Space Invaders:

```cpp
void Game::start_hud(CE::ViewPort<int> logical, CE::FramebufferSize framebuffer) {
    ui_ = std::make_unique<CE::UI::RmlUi::Session>(engine_.input(), 100);
    ui_->set_view(logical, framebuffer);

    const auto path = (asset_root_ / "graphics/ui/space-invaders/hud.rml").u8string();
    document_ = ui_->context().LoadDocument(Rml::String(path.begin(), path.end()));
    if (!document_)
        throw std::runtime_error("Cannot load HUD document");

    score_label_ = document_->GetElementById("score");
    high_score_label_ = document_->GetElementById("high-score");
    lives_label_ = document_->GetElementById("lives");
    if (!score_label_ || !high_score_label_ || !lives_label_)
        throw std::runtime_error("HUD document is missing a required element");

    document_->Show(Rml::ModalFlag::None, Rml::FocusFlag::None);
}
```

Use a nonzero focus ID that is unused by other UI/input targets. Only one Cheryl RmlUi session may be active in the process; load additional documents into the same session. Include the session/scene headers, engine context/window definitions, RmlUi `ElementDocument.h`, and the standard headers used by the snippets.

In `update()`, observe the startup result once, then update the retained UI:

```cpp
void Game::update(const CE::GFramework::TickContext& tick) {
    // The init-submitted callback has already run: dispatcher drain precedes
    // this update in both runtime modes. get() reports errors without waiting.
    if (ui_start_.valid())
        ui_start_.get();

    ui_->set_view(tick.logical_size, tick.framebuffer_size);
    ui_->update_time(tick.delta_seconds);
    score_label_->SetInnerRML(std::to_string(score));
    high_score_label_->SetInnerRML(std::to_string(highscore));
    lives_label_->SetInnerRML(std::to_string(player_lives));

    try {
        CE::UI::RmlUi::adopt_scene(pending_, scene_);
        if (!pending_.valid())
            pending_ = uploader_->submit(*platform_, ui_->record(), materials_);
    } catch (const std::exception& error) {
        // Keep the last complete scene and retain/report the upload error.
        ui_error_ = error.what();
    }
}
```

The score variables in this example come from the game's state. Existing world updates remain part of `update()` too. `platform_` may be stored in an `std::optional<CE::Engine::PlatformDispatcher::Submission>` for the shown initialization/dereference pattern.

An alternative is a guarded `if (!ui_) start_hud(...)` in the first update, as used by Cheryl's demo. That also creates the UI exactly once. The earlier unguarded example omitted this detail and would recreate the session if copied into every update.

## Font files and element selection belong in RCSS

Each HUD stylesheet now declares its font source:

```css
@font-face {
    font-family: college-hud;
    src: "assets/fonts/DejaVuSans.ttf";
    font-style: normal;
    font-weight: normal;
}

body {
    font-family: college-hud;
}
```

`LoadDocument()` loads the linked RCSS, whose `@font-face` declarations load and register the fonts. These HUDs need no `Session::load_font()` call. The C++ method remains available for programmatic or memory-backed font loading.

An element can select a different family, size, style or weight using its RCSS selector. For example, after declaring another face as `score-font`:

```css
#score {
    font-family: score-font;
    font-size: 32px;
}
```

Add that face's file under another `@font-face` block in the stylesheet. There is no per-element C++ font hookup. Additional families and their font files are authored in the UI configuration. [RmlUi font documentation](https://mikke89.github.io/RmlUiDoc/pages/rcss/fonts.html)

The bundled parser currently passes font `src` filenames directly to the file interface. They resolve against the process working directory, even though current upstream documentation describes stylesheet-relative resolution. The declarations above therefore require launching with the repository root as the working directory, or deploying with the same `assets/` layout under the launch directory. An absolute document path does not change this font-path behavior. Use quoted file paths; this parser does not implement browser CSS `url(...)` or `format(...)` syntax for font sources.

The local implementation is in [StyleSheetParser.cpp](../cheryl-engine/extern/rmlui/Source/Core/StyleSheetParser.cpp), particularly `FontFacePropertyParser::Parse` and `ParseFontFaceBlock`. Supporting configuration-owned fonts requires no new Cheryl API. Making their paths consistently stylesheet-relative would be a separate engine/dependency improvement.

## Materials and frame publication

`make_hud_materials` above is an application helper, not a supplied Cheryl function. Adapt the [demo's material setup](../cheryl-engine/projects/apps/demo/src/rmlui-demo.cpp) to create solid and textured materials. Its shader files already exist here under `assets/graphics/shaders/`: `tgui.vert`, `tgui-solid.frag`, and `tgui-textured.frag`. They are usable for the RmlUi pipeline despite their names.

Use the colored triangle vertex layout, premultiplied alpha blending, disabled depth testing/writing and culling, a projection semantic, and the textured material's image sampler. Build these resources during `init()` on the graphics owner. The uploader submits owned CPU recordings through the platform dispatcher; adoption replaces only a complete successful scene.

Append the retained HUD scene after the world passes in `prepare_render_frame()`:

```cpp
void Game::prepare_render_frame(CE::RenderAPIs::RenderFrameWriter& frame) const {
    // Existing world-frame preparation goes here.
    scene_.write(frame);
}
```

## Game data hookup

| Project | Element IDs / contents | Values supplied by the game |
| --- | --- | --- |
| Space Invaders | `score`, `high-score`, `lives` | `score`, `highscore`, `player_lives`. |
| Rover | `rover-recursion-level`, `rover-total-moves` | `Recursion_Level`, `Total_Moves`. |
| Word | `found-words`, `puzzle-letters`, `solution-cells` | Found words and selection, `WordBox` letters, visible solution paths. |
| Hypermaze | `player-coordinates`, `traversal-coordinates`, `terrain-coordinates`, `travel-direction`, `traversal-direction`, `terrain-id`, `node-id`, `node-link-id` | Debugger coordinates/directions/IDs; terrain coordinates are player coordinates minus terrain origin. |

Expose an immutable HUD snapshot or getters for private game data, or author the element changes within the owning game class.

For Word, append `.word-entry` children to `found-words`. Selected entries display the literal text `->WORD<-`; use text nodes or escaped RML so `<` is preserved. Populate `puzzle-letters` with `.letter-cell` children and `solution-cells` with `.solution-cell` children. Set each grid child's `left` to `26*x` pixels and `top` to `26*y` pixels. Rebuild/change these elements when puzzle state changes; they do not need rebuilding every update. Keep arrow-key selection and Space solution toggling in gameplay input.

For Hypermaze, show/hide the document when `debugger.m_Enabled` changes. Format the three IDs as hexadecimal if preserving the old pointer display. The HUD has no separate debug-toggle button.

## Shutdown and current limits

After simulation has joined, release document/element/listener references and call `ui_->close_after_quiescence()` during `deinit()`, then reset the session pointer. Tolerate partial startup: the queued callback may never execute if the runtime stops first. Do not wait for the startup future in `init()`, `quiesce()` or `deinit()`; the runtime cancels pending simulation work before game teardown. Its lifetime contract keeps the game's captured `this` alive through callback execution/cancellation.

After dispatcher shutdown/join, a remaining valid startup future is already settled. Consume it without blocking during teardown: ignore `std::future_errc::broken_promise` from a canceled callback, and report other startup failures after releasing the UI resources. This covers a stop between callback execution and the first update. Release retained scene/material/uploader resources on the platform owner too.

The HUD files have static markup/reference checks, but no in-engine visual acceptance yet. DejaVu Sans approximates the unavailable original bitmap fonts. Word's containers are deliberately empty until actual puzzle data is provided; `word_data_1.wrd` is absent. Space Invaders is mid-port and currently has an empty new frame-preparation hook. The hookups in this guide have not been inserted into the games.

For the current module contracts, see the [RmlUi guide](../cheryl-engine/projects/modules/ui/rmlui/README.md), [AbstractGame hooks](../cheryl-engine/projects/engine/include/cheryl/core/game-framework/abstract-game.h), [Session ownership check](../cheryl-engine/projects/modules/ui/rmlui/src/session.cpp), and [simulation dispatcher](../cheryl-engine/projects/engine/include/cheryl/core/engine/simulation-dispatcher.h).
