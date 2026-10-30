#include <hypermaze/world/HyperMaze.h>
#include <hypermaze/world/Visibility.h>

#include <array>
#include <limits>
#include <random>
#include <stdexcept>

using namespace HypermazeModel;

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void connect(Graph& graph, std::size_t from, std::size_t to, Direction direction, int length = 5) {
    const auto link = graph.links.size();
    graph.links.push_back({{from, to}, direction, length});
    graph.nodes[from].doors[static_cast<int>(direction)].push_back({link, 0});
    graph.nodes[to].doors[static_cast<int>(opposite(direction))].push_back({link, 1});
}

int main() {
    VisibleTiles map;
    const TileLookup lookup = [&](Position p) {
        const auto tile = map.find(p);
        return tile == map.end() ? Tile::Blank : tile->second;
    };
    for (int y = -6; y <= 6; ++y)
        for (int x = -6; x <= 6; ++x) map[{x, y}] = Tile::Floor1;
    const auto clear = field_of_view({0, 0}, 6, 6, lookup);
    check(clear.size() == 169, "Open terrain should be visible in every direction");
    check(field_of_view({0, 0}, 0, 0, lookup).size() == 1, "Player tile should be visible");
    check(!has_line_of_sight({0, 0}, {7, 0}, lookup), "Unknown terrain should stay hidden");
    check(!blocks_sight(Tile::Floor1) && !blocks_sight(Tile::Floor2) && !blocks_sight(Tile::Stairs)
        && !blocks_sight(Tile::Horizontal) && !blocks_sight(Tile::Vertical) && !blocks_sight(Tile::DoorOpen),
        "Traversable terrain should transmit sight");
    const std::array opaque{Tile::Blank, Tile::Wall1, Tile::Wall2, Tile::DoorClosed, Tile::GateClosed,
        Tile::TopLeft, Tile::TopRight, Tile::BottomLeft, Tile::BottomRight};
    for (auto tile : opaque) {
        map[{1, 0}] = tile;
        check(!has_line_of_sight({0, 0}, {2, 0}, lookup), "Opaque tile did not block sight");
        check(has_line_of_sight({0, 0}, {1, 0}, lookup) == (tile != Tile::Blank), "First blocking wall should be visible");
    }
    map[{1, 0}] = Tile::DoorOpen;
    check(has_line_of_sight({0, 0}, {4, 0}, lookup), "An open door should transmit sight");

    map[{1, 0}] = Tile::Wall1;
    map[{0, 1}] = Tile::Wall2;
    check(!has_line_of_sight({0, 0}, {1, 1}, lookup), "Sight leaked through a closed diagonal corner");
    check(!has_line_of_sight({0, 0}, {4, 4}, lookup), "Sight leaked beyond a closed diagonal corner");
    map[{1, 1}] = Tile::Wall1;
    check(has_line_of_sight({0, 0}, {1, 1}, lookup), "Blocking corner wall should remain visible");
    map[{1, 0}] = Tile::Blank;
    check(!has_line_of_sight({0, 0}, {1, 1}, lookup), "Corner wall leaked through unknown terrain");
    map[{1, 0}] = Tile::Wall1;
    map[{1, 1}] = Tile::Floor1;
    map[{0, 1}] = Tile::Floor1;
    check(has_line_of_sight({0, 0}, {1, 1}, lookup), "A single wall should not close an otherwise open corner");
    map[{1, 0}] = Tile::Floor1;

    for (auto direction : {Position{1,1}, Position{1,-1}, Position{-1,1}, Position{-1,-1}}) {
        const Position obstruction{2 * direction.x, 0};
        map[obstruction] = Tile::Wall1;
        const Position target{4 * direction.x, direction.y};
        check(!has_line_of_sight({0,0}, target, lookup), "Shallow ray skipped a blocking tile");
        check(!has_line_of_sight(target, {0,0}, lookup), "Reverse shallow ray skipped a blocking tile");
        map[obstruction] = Tile::Floor1;
        const Position vertical_obstruction{0, 2 * direction.y};
        map[vertical_obstruction] = Tile::Wall2;
        check(!has_line_of_sight({0,0}, {direction.x, 4 * direction.y}, lookup), "Steep ray skipped a blocking tile");
        map[vertical_obstruction] = Tile::Floor1;
    }

    // A solid partition hides the far side even if it was visible previously.
    for (int y = -6; y <= 6; ++y) map[{0, y}] = Tile::Wall1;
    const auto left = field_of_view({-2, 0}, 6, 6, lookup);
    const auto right = field_of_view({2, 0}, 6, 6, lookup);
    check(left.contains({-3, 0}) && !left.contains({3, 0}), "Partition did not hide the far side");
    check(right.contains({3, 0}) && !right.contains({-3, 0}), "Old visibility was retained after movement");
    check(left.contains({0,0}) && right.contains({0,0}), "Partition itself should be visible from either side");

    // LOS between floor tiles is symmetric, including corner ties and all octants.
    std::mt19937 random(42);
    for (int sample = 0; sample < 16; ++sample) {
        for (auto& [position, tile] : map) tile = random() % 5 == 0 ? Tile::Wall1 : Tile::Floor1;
        for (int i = 0; i < 128; ++i) {
            const Position a{static_cast<int>(random() % 13)-6, static_cast<int>(random() % 13)-6};
            const Position b{static_cast<int>(random() % 13)-6, static_cast<int>(random() % 13)-6};
            if (blocks_sight(lookup(a)) || blocks_sight(lookup(b))) continue;
            check(has_line_of_sight(a, b, lookup) == has_line_of_sight(b, a, lookup), "Visibility is not symmetric");
        }
    }
    map.clear();
    check(field_of_view({0,0}, 3, 3, lookup).empty(), "Unknown space should not become visible");
    map[{std::numeric_limits<int>::max(), std::numeric_limits<int>::min()}] = Tile::Floor1;
    check(field_of_view({std::numeric_limits<int>::max(), std::numeric_limits<int>::min()}, 1, 1, lookup).size() == 1,
        "Visibility bounds wrapped signed world coordinates");
    bool rejected = false;
    try { static_cast<void>(field_of_view({0,0}, -1, 1, lookup)); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "Negative field-of-view extents were accepted");

    Graph graph;
    graph.nodes.resize(2);
    connect(graph, 0, 1, Direction::Right);
    HyperMaze maze(graph);
    const auto initial_view = maze.visible_tiles(12, 8);
    check(initial_view.contains({0,0}) && initial_view.contains({2,2}), "Room's enclosing walls should be visible");
    check(initial_view.contains({9,1}), "Sight stopped before an unvisited room");
    check(initial_view.contains({10,1}) && !initial_view.contains({11,1}), "Sight did not stop at the far room's wall");
    check(maze.tile({9,1}) == Tile::Blank && maze.discovered_tiles() == 9, "Looking should not mark rooms as visited");
    check(maze.move(Direction::Right) && maze.move(Direction::Right), "Cannot enter test corridor");
    const auto corridor_view = maze.visible_tiles(8, 8);
    check(corridor_view.contains({7,1}), "Straight corridor should remain visible");
    check(maze.move(Direction::Up) == false, "Test corridor wall should block movement");
    check(corridor_view == maze.visible_tiles(8, 8), "Blocked movement changed visibility");
    check(maze.visible_tiles(1, 1).size() < corridor_view.size(), "Smaller viewport did not reduce visible terrain");

    // All immediate rooms and passages are revealed, including branches that
    // cannot be seen from the player. They must not cast sight into another room.
    Graph chamber;
    chamber.nodes.resize(6);
    connect(chamber, 0, 1, Direction::Left);
    connect(chamber, 0, 2, Direction::Left);
    connect(chamber, 0, 3, Direction::Up);
    connect(chamber, 0, 4, Direction::Up);
    connect(chamber, 1, 5, Direction::Up);
    HyperMaze full_room(chamber);
    check(full_room.move(Direction::Left), "Cannot stand in the test room's doorway");
    const auto room_view = full_room.visible_tiles(16, 16);
    for (int y = 0; y < 5; ++y)
        for (int x = 0; x < 5; ++x)
            check(room_view.contains({x,y}) && room_view.at({x,y}) == full_room.terrain().at({x,y}),
                "Current room must show every floor, wall and door");
    check(room_view.at({0,3}) == Tile::DoorOpen, "Another door along the same wall should remain visible");
    check(room_view.contains({-7,1}), "Full-room visibility should preserve sight into unvisited rooms");
    check(room_view.contains({-1,3}) && room_view.contains({-7,3}), "Every immediately connected corridor and room should be revealed");
    check(room_view.contains({-8,2}) && room_view.contains({-6,4}) && room_view.at({-7,4}) == Tile::DoorOpen,
        "Adjacent rooms must show their walls and doors even outside player sight");
    check(room_view.contains({1,11}) && room_view.contains({3,11}), "Other directions should reveal their adjacent rooms");
    check(!room_view.contains({-7,6}) && !room_view.contains({-7,11}), "Automatic reveal leaked beyond one room connection");
    check(full_room.discovered_tiles() == 25, "Revealing adjacent rooms should not mark them visited");
    check(full_room.visible_tiles(0,0).size() == 1, "Full-room visibility should respect viewport bounds");
    check(full_room.move(Direction::Left) && full_room.in_corridor(), "Cannot leave the fully visible room");
    const auto from_passage = full_room.visible_tiles(16,16);
    check(from_passage.contains({0,3}) && from_passage.contains({-8,0}), "Both corridor endpoint rooms should be fully drawn");
    check(!from_passage.contains({-7,3}), "A corridor should not reveal unrelated neighbors of its endpoint rooms");
    check(full_room.move(Direction::Right) && !full_room.in_corridor(), "Cannot reenter the test room");
    check(full_room.visible_tiles(16,16).contains({-7,3}), "Reentering a room should restore its adjacent rooms");

    // There is no range cap: rays cross multiple unseen rooms and corridors in
    // every direction, including negative coordinates, until a real wall.
    for (auto direction : {Direction::Left, Direction::Right, Direction::Up, Direction::Down}) {
        Graph chain;
        chain.nodes.resize(5);
        for (std::size_t i = 0; i < 4; ++i) connect(chain, i, i + 1, direction);
        HyperMaze straight(chain, 17);
        const auto step = offset(direction);
        const auto eye = straight.player();
        const auto view = straight.visible_tiles(40, 40);
        const Position perpendicular{-step.y, step.x};
        check(view.contains(eye + Position{9 * step.x, 9 * step.y} + perpendicular),
            "An adjacent room should be revealed in full in every direction");
        check(!view.contains(eye + Position{17 * step.x, 17 * step.y} + perpendicular),
            "Farther rooms should only reveal tiles in player sight");
        for (int distance = 0; distance <= 33; ++distance) {
            const auto p = eye + Position{distance * step.x, distance * step.y};
            check(view.contains(p), "Sight was truncated in a connected chain");
            check(blocks_sight(view.at(p)) == (distance == 33), "Incorrect terminal wall in connected chain");
        }
        check(!view.contains(eye + Position{34 * step.x, 34 * step.y}), "Sight passed through the terminal wall");
        check(straight.player() == eye && straight.origin() == Position{} && straight.current_node() == 0
            && !straight.in_corridor() && !straight.current_link() && straight.traversal() == Position{}
            && straight.travel_direction() == Direction::Up && straight.discovered_tiles() == 9,
            "Visibility changed player traversal or exploration state");
        for (int distance = 1; distance < 33; ++distance) {
            check(straight.move(direction), "Cannot walk along previewed terrain");
            check(view.at(straight.player()) == straight.terrain().at(straight.player() - straight.origin()),
                "Previewed terrain differs from terrain reached by movement");
        }
        const auto from_far_room = straight.visible_tiles(40, 40);
        check(from_far_room.contains(eye), "Sight cannot follow visited corridors in reverse");
    }

    // A turn remains an occluder; connectivity alone must not reveal everything.
    Graph bent;
    bent.nodes.resize(3);
    connect(bent, 0, 1, Direction::Right);
    connect(bent, 1, 2, Direction::Up);
    HyperMaze corner(bent);
    const auto before_turn = corner.visible_tiles(20, 20);
    check(before_turn.at({9,2}) == Tile::DoorOpen, "The adjacent room's far door should be revealed");
    check(before_turn.contains({9,1}) && !before_turn.contains({9,5}) && !before_turn.contains({9,9}),
        "Revealing an adjacent room must not cast sight around its corner");
    for (int i = 0; i < 8; ++i) check(corner.move(Direction::Right), "Cannot reach test corner");
    check(corner.visible_tiles(20,20).contains({9,9}), "New corridor stayed hidden after reaching the corner");

    // Two different graph paths can place incompatible terrain at the same
    // coordinates. Looking through one door must keep following that door's
    // connection, regardless of which overlapping rooms were visited last.
    Graph overlap;
    overlap.nodes.resize(4);
    connect(overlap, 0, 1, Direction::Right);
    connect(overlap, 0, 2, Direction::Right, 3);
    connect(overlap, 3, 2, Direction::Right, 4);
    HyperMaze overlapping(overlap);
    const auto before_exploring = overlapping.visible_tiles(20, 20);
    check(before_exploring.contains({9,1}), "Overlapping graph fixture should reveal the first room");
    for (int i = 0; i < 8; ++i) check(overlapping.move(Direction::Right), "Cannot visit the first overlapping room");
    for (int i = 0; i < 8; ++i) check(overlapping.move(Direction::Left), "Cannot leave the first overlapping room");
    for (int i = 0; i < 2; ++i) check(overlapping.move(Direction::Up), "Cannot approach the second door");
    for (int i = 0; i < 6; ++i) check(overlapping.move(Direction::Right), "Cannot visit the second overlapping room");
    for (int i = 0; i < 6; ++i) check(overlapping.move(Direction::Left), "Cannot leave the second overlapping room");
    for (int i = 0; i < 2; ++i) check(overlapping.move(Direction::Down), "Cannot return to the original viewpoint");
    check(blocks_sight(overlapping.tile({8,1})), "Overlapping fixture must overwrite the first room's doorway");
    check(overlapping.visible_tiles(20, 20) == before_exploring, "Overlapping visited terrain changed the visible graph path");

    // Repeated placements of a self-loop unfold along the ray, instead of being
    // cut off by a global visited-node set or recursing without a viewport bound.
    for (auto direction : {Direction::Left, Direction::Right, Direction::Up, Direction::Down}) {
        Graph loop;
        loop.nodes.resize(1);
        connect(loop, 0, 0, direction);
        HyperMaze looping(loop);
        const auto step = offset(direction);
        const auto view = looping.visible_tiles(70, 70);
        for (int distance = -70; distance <= 70; ++distance) {
            const auto p = looping.player() + Position{distance * step.x, distance * step.y};
            check(view.contains(p) && !blocks_sight(view.at(p)), "Sight stopped in a straight self-loop");
        }
        check(looping.discovered_tiles() == 9 && looping.player() == Position{1,1}, "Looking through a loop moved the player");
    }
}
