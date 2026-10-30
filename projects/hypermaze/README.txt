Hypermaze uses Cheryl Engine 4. See the repository README for SDK setup.

Build:
  cmake --preset shared-debug -DCOLLEGE_PORTS_PROJECTS=hypermaze
  cmake --build --preset shared-debug

Run:
  builds/shared-debug/bin/hypermaze --seed 42

WASD, arrow keys or the gamepad D-pad move one tile. Hold to repeat.
R generates another maze. F3 toggles the debug readout. Esc quits.

Rooms and corridors form a generated graph with loops. Crossing a doorway
places the next terrain relative to the player, retaining previously discovered
tiles. Signed coordinates support travel in every direction. Graph, terrain and
tile storage are owned by the gameplay model; rendering copies retained asset
draws into Cheryl frames on the simulation owner.

The current room, every room one connection away and their connecting corridors
are drawn in full, including walls and doors. While in a corridor, both endpoint
rooms are fully drawn. Farther terrain requires line of sight from the player's
actual position. Revealed rooms do not cast additional rays. Sight can cross any
number of connections within the viewport until a wall, closed door or closed
gate blocks it. Blocking walls remain visible. Each ray follows the maze graph so
overlapping placements and loops retain their connections. Visibility updates
after movement, regeneration and resizing.

The world model tests run through CTest. Interactive play remains a manual check.
