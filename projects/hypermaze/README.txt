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

The world model tests run through CTest. Interactive play remains a manual check.
