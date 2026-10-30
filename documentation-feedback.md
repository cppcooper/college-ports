# Documentation feedback

Reviewed October 29, 2026.

The main opportunity is to make the first useful implementation easier to reach.
Much of the information requested in the chats is now documented. The remaining
friction is often connecting the types, lifecycle hooks, and ownership rules into
one small example. Adding more reference text or long comments would not, by
itself, solve that problem.

The three changes I would start with are:

1. Extend the UI guides with a complete pause-menu example: TGUI HUD, RmlUi menu,
   input routing, deferred menu actions, continued UI updates while paused, and
   cleanup.
2. Add the missing variable-rate audio procedure, using the existing SDK API.
3. Extend the animation procedure with default-clip conventions, playback speed,
   and a clear distinction between an asset, a playback cursor, and a selected cell.

## Scope and evidence

This review covers the current conversation and six saved `college-ports` session
logs. Five contain substantive user discussion; the sixth contains no user
messages. The substantive sessions cover these topics:

| Session | Main subjects |
| --- | --- |
| `01a157e3` | Controller construction, input contexts, lifecycle owners, mixed UI, rendering, memory, animation, and time units. |
| `01a1610f` | Formation movement, controller decisions, events, collision queries, and boundaries between application systems. |
| `01a169d5` | Lifecycle/render interfaces, actor/controller ownership, UI separation, and pooled formation construction. |
| `01a1839b` | Collisions, death/respawn, scoring, game speed, pooled enemies, and UFO behavior. |
| `01a1858b` | HUD sizing, shutdown diagnostics, menus, high scores, audio, quit, and new-game flow. |

The implementation reviewed is `college-ports` commit `28c973fb55ac`. The engine
submodule and `/mnt/projects/cheryl-engine` are both at `7685abfc2b98`; the asset
repositories are at `768f741b48bc`. The working trees were clean when inspected.
Representative installed SDK declarations were compared with the matching engine
source. Documentation observations concern the Markdown source, not the published
website or an IDE's cached documentation.

This is a source and documentation review. No game builds, native execution,
gameplay tests, or documentation builds were run. Earlier sessions contain test
reports and user observations; they are historical evidence, not checks repeated
by this review. The user's preference to handle game testing remains applicable.

## Documentation improvements

### 1. Make the input-context types explicit at the construction step

The first conversation repeatedly mixed up a binding definition, an attachment
endpoint, and the attached context. The deleted-copy-constructor error followed
directly from that confusion.

The current [PlayerController tutorial](extern/cheryl-engine/docs/site/getting-started/create-a/controller.md)
already shows endpoint acquisition and controller construction. Its
[maintained controller](extern/cheryl-engine/projects/apps/tutorial-game/src/player-controller.h)
also moves the context into its member. Treat this as a first-use explanation and
troubleshooting improvement, not a missing constructor example.

Add a short type table beside that example:

| Expression or value | Type | Meaning |
| --- | --- | --- |
| `engine.input()` | `CE::Input::System&` | Borrow the initialized input system. |
| `input.action_contexts()` | `CE::Input::Actions::ContextSubmission` | Acquire a request endpoint on platform; its `attach()` operation can be used by other producers. |
| `keyboard(...)` / `defined_controls(...)` | `CE::Input::Actions::ContextDefinition` | Application-owned mapping data. These helper names belong to the example. |
| `endpoint.attach(definition)` | `CE::Input::Actions::Context` | A move-only lease for one action namespace. |
| `tick.input.actions(context)` | `CE::Input::Actions::View` | Read that namespace from the tick; the view borrows the tick. |

Show both immediate construction and named-variable transfer with `std::move`.
Explain that an accepted attachment becomes visible after a subsequent poll.
Briefly identify `CE::Runtime::Context` and rendering's `Submission::Context2D`
as unrelated types despite their similar names.

Public inheritance and borrowing through `unique_ptr::get()` also caused trouble.
A small C++ troubleshooting note is sufficient; neither calls for an engine cast
or ownership API.

### 2. Turn the UI lifecycle contracts into an interactive composition recipe

The current [UIController tutorial](extern/cheryl-engine/docs/site/getting-started/create-a/ui-controller.md),
[GUI overview](extern/cheryl-engine/docs/site/gui/index.md), and toolkit references
already explain the owner split. The port now demonstrates the next step:
[UIController](projects/space-invaders/include/space_invaders/controllers/ui.h)
handles application input, [GameUI](projects/space-invaders/src/gui/GameUI.cpp)
owns the HUD presentation, and [GameMenu](projects/space-invaders/src/gui/game-menu.cpp)
owns an independently published RmlUi menu.

Extend the existing guides with a compact composition example covering:

- Platform initialization of bindings, uploaders, and submission endpoints.
- Simulation initialization of sessions, widgets, documents, and listeners.
- One nonzero routing target per toolkit session, with separate native widget focus.
- Passive HUD input, menu pointer delivery, and the choice between `PassThrough`
  and `Exclusive` keyboard routing. Keyboard focus does not disable all gameplay
  input, including gamepad input.
- Recording and nonblocking adoption with one outstanding upload per toolkit.
- Simulation shutdown, followed by platform cleanup after accepted work settles;
  release external widget/document/listener references before closing sessions.

Keep a short failure explanation beside each sensitive step. For example,
[uploader construction captures its creating thread](extern/cheryl-engine/projects/modules/ui/tgui/src/scene.cpp).
Constructing it on simulation can make a later platform upload throw
`failed_operation`; the error reaches simulation through the upload future.
That is more useful than simply saying the uploader is platform-owned.

Use equally precise language for input: acquiring `action_contexts()` lazily
changes live mapper state in [input-bindings.cpp](extern/cheryl-engine/projects/engine/src/core/controls/input-bindings.cpp).
Wrong-owner acquisition is not guaranteed to throw; overlapping it with polling
can race. That differs from `ContextSubmission::attach()`, which is designed for
producer-side submission. Startup ordering must not be described as a guaranteed
crash merely because a call was placed on the wrong owner.

Also state that retained UI scenes lag live widgets. Hiding, reopening, or changing
a menu page can leave an older complete scene available until the replacement
finishes. Explain when retaining that scene is acceptable and when an application
should clear it or reject obsolete completions. Current adoption behavior is
intentional; this is a presentation-policy explanation, not a confirmed engine bug.

### 3. Explain the rendering vocabulary together, then show the call chain

The conversation needed several rounds to distinguish frame, pass, batch, layer,
and submission context. [First Game](extern/cheryl-engine/docs/site/getting-started/first-game.md)
now explicitly explains `FrameWriter`, `PassWriter`, and `Context2D`. Preserve that
explanation and link to it at the actor and UI submission steps.

A small diagram should show `frame -> ordered passes -> ordered draw packets`.
A pass shares camera and rendering settings; it does not prevent overlap or imply
one GPU draw call. Visibility still depends on draw order, depth, and blending.

The port's naming provides a useful example:

- [WorldObject::append_draws(PassWriter&)](projects/space-invaders/include/space_invaders/game-objects/world-object.h)
  contributes an actor's selected appearance to the world pass.
- [GameUI::append_passes(FrameWriter&)](projects/space-invaders/src/gui/GameUI.cpp)
  appends HUD and menu passes in order.
- [SpaceInvaders::prepare_render_frame()](projects/space-invaders/src/SpaceInvaders.cpp)
  chooses the overall order.

Keep the complete `resolve_sprite(sprite, cell, style, context)` call visible in
the actor example. Identify `DrawStyle2D` as per-object inputs and `Context2D` as
the pass inputs. The extra argument was a concrete stumbling block. No new shared
render base class is needed to explain this pattern.

### 4. Separate sprite selection, animation selection, and playback time

The existing [animation procedure](extern/cheryl-engine/docs/site/getting-started/how-to/animate-sprite.md)
already adds a named two-cell clip, retains a cursor, and advances it. Extend its
opening and add a short speed example rather than another animation tutorial.

Explain that selecting `invaders:enemy_1` selects an asset and its grid. It does not
select an animation timeline. A clip supplies cell order, durations, and looping;
each actor owns a separate cursor over shared clip data. A sprite with two cells
does not automatically have an animation.

The current [Invaders manifest](assets/graphics/definitions/misc/invaders.json)
uses the name `default` by application convention. The SDK does not automatically
select that name. Its two-cell enemy clips use 2,000 ms per frame, which means a
four-second complete cycle at normal speed; the UFO has a one-cell clip.

Show the units and rate convention explicitly:

```cpp
animation.advance(std::chrono::duration<double>{tick.delta_seconds * speed});
```

`duration<double>` uses seconds. With a two-second frame duration, speed `1` gives
two seconds per frame and speed `2` gives one second. Scaling elapsed time is
different from scaling the interval itself. Keep finite, nonnegative elapsed time
and positive manifest frame durations. This already works through `advance()`;
it does not require another engine speed-control operation.

[EnemyActor](projects/space-invaders/include/space_invaders/game-objects/actors/enemy.h)
already uses this approach. Also discuss shared formation timing when enemies
must march in phase. Do not imply that all timers must use one game-speed value:
the port intentionally keeps explosion and respawn timers separate from scaled
movement and animation.

### 5. Connect events to pause, restart, and safe mutation boundaries

The original request could not find which engine events existed. There is now an
[engine-event catalogue](extern/cheryl-engine/docs/site/engine-systems/events.md),
and the [event tutorial](extern/cheryl-engine/docs/site/getting-started/create-a/event.md)
explains publishing and cleanup. Improve routes to those pages instead of
maintaining another event list. Point out the existing name-only event path for
signals and typed channels for payloads such as shot position and velocity.

The missing application example is the policy around those events:

- Pause is application state. Keep UI input, scene publication, and frame
  preparation running while skipping the selected world updates.
- Process UI before gameplay and define what happens to input on the resume tick.
- Immediate listeners run within publication. A shot can create a bullet before
  `fire()` returns; a hit can notify several game systems before collision
  iteration finishes.
- Defer destructive transitions until a suitable boundary. The port drains
  [menu requests](projects/space-invaders/src/gui/game-menu.cpp) after toolkit input,
  records a [new-game intent](projects/space-invaders/include/space_invaders/SpaceInvaders.h),
  and defers [bullet clearing](projects/space-invaders/src/systems/bullet-mgr.cpp)
  during collision iteration.
- Menu quit requests become `tick.request_stop()` on the game update path. A
  menu callback does not need to own runtime shutdown.

Put the non-RAII registration rule beside the first lasting subscription:
dropping `EventBus::Registration` does not unsubscribe. Explain `disconnect()`
versus waiting for entered callbacks; retain explicit quiescence requirements
for concurrent/queued listeners. The current game explicitly disconnects its
listeners. An event does not, by itself, move work to the required thread.

### 6. Show ownership and update order without prescribing a game-object hierarchy

The later sessions repeatedly revisited who owns actors, whether controllers
create them, whether references require shared ownership, and who forwards updates.
The [abstract-controller discussion](extern/cheryl-engine/docs/site/getting-started/create-a/abstract-controller.md)
already recommends concrete calls and narrow interfaces. Add a small ownership
example linking that discussion to lifecycle setup.

The port now has a useful concrete arrangement: the game owns its systems and
player/UFO, controllers borrow their targets, the invasion owns its enemy block,
and the UI owns its toolkit objects. The game explicitly schedules actor,
controller, system, and UI updates. A controller can choose an action while its
target implements the basic operation. Update an actor once, rather than both
directly and through a forwarding controller.

Distinguish ordinary construction from initialization that needs loaded assets
or a particular owner. Explain that an application-defined `AbstractGameObject`
does not register itself with Cheryl. A bullet can reuse world appearance and
bounds without needing every actor behavior. These are composition choices, not
missing engine entity classes.

### 7. Explain what pooling covers, then give examples for two ownership needs

The early memory question was about manifest loading. Later code needed storage
for application objects. Connect those as separate tasks in the
[memory overview](extern/cheryl-engine/docs/site/engine-systems/memory.md) and
[allocation guide](extern/cheryl-engine/docs/site/resources/resource-lifetime.md).

Sprite/tileset loaders reserve asset-object slots, grid construction uses managed
CPU vertex blocks, decoded images use ordinary vectors, and the backend creates
GPU resources. Loading a manifest is not one allocation for every kind of data,
and it does not automatically allocate the game's actors.

The port supplies two examples worth explaining:

- [Invasion::Pool](projects/space-invaders/src/systems/invasion.cpp): one allocation
  for 70 enemies, varying constructor arguments, a live prefix during construction,
  borrowed spans/pointers, and cleanup if construction fails. Replacing a wave
  invalidates its borrowed pointers. The UFO is separately owned.
- [Bullet](projects/space-invaders/include/space_invaders/game-objects/bullets.h):
  independently destroyed objects with ordinary `unique_ptr` ownership, backed by
  an object-pool allocator and a warmed pool of 100 slots. That warm-up is not a
  hard maximum or a memory budget.

The guide already presents shared pool handles and reservations. Explain their
ownership tradeoffs before recommending them for every pooled object. Contiguous
storage, shared ownership, object construction, and cache residency solve
different problems. This review does not establish a need for a new allocator.

### 8. Document the variable-rate audio API and its clock

This is a current documentation gap. The installed
[audio declarations](extern/cheryl-engine/projects/engine/include/cheryl/audio/system.h)
provide `PlaybackOptions::variable_rate`, `Voice::set_playback_rate()`, and a
snapshot playback rate. The [audio contract](extern/cheryl-engine/docs/site/runtime/audio.md)
and [miniaudio guide](extern/cheryl-engine/docs/site/modules/miniaudio.md) do not
explain that control path.

The miniaudio guide also says that voices bypass pitch processing and that Cheryl
exposes no pitch control. Qualify those statements for normal playback and
distinguish independent pitch control from the pitch change caused by variable
playback rate. The [backend implementation](extern/cheryl-engine/projects/modules/audio/miniaudio/src/system.cpp)
already enables that processing when `variable_rate` is selected.

Add a short procedure: opt into variable-rate playback when opening the voice,
retain the voice, change its rate during updates, and handle unsupported backends.
Document the finite `[0.125, 8]` multiplier range, that pitch follows speed, that
restart preserves the rate, and that snapshot duration describes source duration.
Distinguish rate control from pitch-preserving time stretching.

[GameAudio](projects/space-invaders/src/audio.cpp) also demonstrates why pausing
world updates does not pause device playback. Explain explicit music/effect pause
policy, retained voice handles, regular `maintain()`, and notification pumping
when terminal events are used.

The same implementation pre-mixes enemy volleys with 22 ms spacing so timing does
not depend on the game's update frequency. Document the distinction between
simulation scheduling and audio-clock scheduling. The existing playback-rate API
needs documentation, not another speed-control API.

### 9. Link the existing tint recipe from sprite submission

The port builds a custom material for red enemies in
[SpaceInvaders.cpp](projects/space-invaders/src/SpaceInvaders.cpp) and inserts a
custom `tint` value in [WorldObject](projects/space-invaders/include/space_invaders/game-objects/world-object.h).
The rendering guide already has a complete
[per-object material example](extern/cheryl-engine/docs/site/rendering/pipelines-and-materials.md#give-one-object-a-custom-material).

Make that route easy to find from the first sprite draw. Explain why alpha and
scale are available directly while tint depends on the selected material's
parameter contract, and why sprite-cell triangle strips differ from UI triangles.
The current evidence supports better navigation; it does not establish a need to
change `DrawStyle2D` or require a new built-in shader.

### 10. Add symptom-based troubleshooting without treating every symptom as an engine bug

The later sessions exposed issues beyond the initial rendering/input questions:

- A label's arrows were its scrollbar: the chosen font did not fit a fixed-height
  label. Link to native widget sizing and show when autosizing is appropriate.
- A shutdown leak report was traced in an earlier session to GTK/Fontconfig/Mesa
  dependency paths. The current [Native GLFW diagnostics](extern/cheryl-engine/projects/modules/platform/native-glfw/src/core/display/glfw-diagnostics.cpp)
  implementation provides a weak default LeakSanitizer suppression hook on Linux
  with GCC/Clang.
  Add a development note explaining their scope, the application override, and
  that suppression does not free allocations or prove that application leaks are
  absent. This review did not reproduce the leak.
- SDK changes to lifecycle hooks caused stale examples and comments. Put source
  revision, installed SDK, CMake package selection, and documentation-version
  checks in one short troubleshooting route.

Keep a concise normal path and link to the exact contracts. Repeated requests for
shorter TODO comments and a three-item starting list are evidence about the
presentation: start with the next useful steps, then offer the detailed rules.

## Possible engine additions

These are proposals, not existing APIs or changes made by this review. Most of
the problems above can be addressed without changing the engine.

**Default animation selection is a reasonable additive convenience.** Every
`EnemyActor` currently requests `sprite->animation("default")`, and the name has
no special engine meaning. Consider an explicit `default_animation` metadata
field plus an operation that obtains a fresh default playback cursor. Validate
the referenced name/facing during loading, define missing/default-free behavior,
and leave existing named lookup unchanged. Do not silently choose the first
unordered manifest entry. Each caller still needs independent playback state.
This directly addresses the repeated confusion; it is not required for the port
to use its current convention.

**Delayed voice starts are a plausible optional audio capability.** The current
API has immediate/paused playback and explicit resume, but no scheduled-start
operation. A backend-supported output-clock delay or scheduled start could avoid
pre-mixing every volley size. Its contract would need units, late-start behavior,
pause/stop/restart rules, unsupported-backend behavior, and a clear distinction
between requested timing and guaranteed sample accuracy. The port's pre-mixed
clips already meet its fixed-spacing requirement, so this is a future capability
to evaluate, not an urgent fix.

**Keep lifecycle and world-policy additions deferred.** The local lifecycle
interface may be useful to illustrate, but an engine-owned component base would
not automatically solve scheduling, dependency order, or ownership. The current
port does not justify an ECS, mandatory actor/controller classes, global gameplay
pause events, automatic game-speed propagation, or a second event system.
Likewise, a pass-context helper could reduce repeated construction, but a complete
example is sufficient evidence-based work before expanding that API.

## Application observations from the review

Two implementation edges should remain separate from documentation/API requests:

- [InvasionController::update()](projects/space-invaders/include/space_invaders/controllers/invasion.h)
  handles an initial edge hit, then discards the result of its remaining-time move.
  A sufficiently large delta or game speed can reach another edge in that same
  update without processing that second result. The code makes game speed
  uncapped. This is an application movement-policy issue, not evidence that the
  engine needs a formation controller.
- The world camera and several gameplay bounds are captured during platform
  initialization, while UI sessions follow the current view automatically. If
  window resizing is supported, those coordinate spaces can diverge. The existing
  First Game recipe already demonstrates using copied tick dimensions; this is
  application integration to check, not a newly missing camera API.

Neither case was reproduced through execution, and no implementation was changed.
The report's recommended outcome is a few more complete, easy-to-find procedures
and two bounded API proposals, rather than a redesign of the game or engine.
