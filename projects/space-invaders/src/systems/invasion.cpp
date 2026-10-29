#include <space_invaders/systems/invasion.h>
#include <space_invaders/constants.h>

Invasion::Pool::~Pool() {
    auto* storage = enemies.data();
    for (auto i = enemies.size(); i > 0; --i)
        Traits::destroy(allocator, storage + i - 1);
    if (storage) Traits::deallocate(allocator, storage, formation_size);
}

EnemyActor& Invasion::Pool::construct(
    const EnemyType type, const glm::vec2 position, const CE::Events::EventBus::Publisher& publisher,
    const std::shared_ptr<const CE::Rendering::Resources::Material>& material
) {
    auto* storage = enemies.data();
    auto* enemy = storage + enemies.size();
    Traits::construct(allocator, enemy, type, position, static_cast<int>(enemies.size()), publisher, material);
    enemies = {storage, enemies.size() + 1};
    return *enemy;
}

std::unique_ptr<Invasion::Pool> Invasion::Pool::create(
    const CE::Events::EventBus::Publisher& publisher, const glm::vec2 world_size,
    const std::shared_ptr<const CE::Rendering::Resources::Material>& material
) {
    auto result = std::make_unique<Pool>();
    result->front_line.reserve(columns);
    result->enemies = {Traits::allocate(result->allocator, formation_size), 0};
    constexpr std::array types{
        EnemyType::MALOLRIE, EnemyType::ARCHER, EnemyType::LANA, EnemyType::CHERYL,
        EnemyType::CYRIL, EnemyType::RAY, EnemyType::KRIEGER
    };

    glm::vec2 largest{};
    for (const auto type : types) {
        const auto sprite = GetEnemySprite(type);
        if (!sprite) {
            throw std::logic_error("Load invader sprites before spawning the invasion");
        }
        const auto& frame = sprite->definition().grid.frame;
        largest.x = std::max(largest.x, frame.width * 0.45f);
        largest.y = std::max(largest.y, frame.height * 0.45f);
    }
    const float column_spacing = std::min(
        largest.x + 12.0f, std::max(0.0f, world_size.x - largest.x - 48.0f) / (columns - 1)
    );
    const float row_spacing = std::min(
        largest.y + 8.0f,
        std::max(
            0.0f, world_size.y - PlayerActor::spawn_height - SpaceInvadersConfig::formation_spawn_top_offset - 50.0f
        ) / (rows - 1)
    );
    const float first_x = (world_size.x - column_spacing * (columns - 1)) * 0.5f;
    const float first_y = world_size.y - SpaceInvadersConfig::formation_spawn_top_offset - row_spacing * (rows - 1);

    for (std::size_t column = 0; column < columns; ++column) {
        for (std::size_t row = 0; row < rows; ++row) {
            auto& enemy = result->construct(
                types[row],
                glm::vec2{first_x + column * column_spacing, first_y + row * row_spacing},
                publisher, material
            );
            if (row == 0) {
                result->front_line.push_back(&enemy);
            }
        }
    }
    return result;
}

void Invasion::Pool::refresh_front_line() {
    front_line.clear();
    for (std::size_t column = 0; column < columns; ++column) {
        const auto first = column * rows;
        for (std::size_t row = 0; row < rows; ++row) {
            auto& enemy = enemies[first + row];
            if (enemy.is_alive()) {
                front_line.push_back(&enemy);
                break;
            }
        }
    }
}

std::optional<Bounds> Invasion::Pool::formation_bounds() const {
    std::optional<Bounds> result;
    for (std::size_t i = 0; i < formation_size; ++i) {
        if (!enemies[i].is_alive()) continue;
        const auto shape = enemies[i].bounds();
        if (!result) {
            result = shape;
        } else {
            result->minimum = glm::min(result->minimum, shape.minimum);
            result->maximum = glm::max(result->maximum, shape.maximum);
        }
    }
    return result;
}

void Invasion::fire(int shots, std::default_random_engine& random) const {
    if (!pool || shots <= 0) return;
    pool->refresh_front_line();
    auto shooters = pool->front_line;
    std::shuffle(shooters.begin(), shooters.end(), random);
    const auto count = std::min(static_cast<std::size_t>(shots), shooters.size());
    for (std::size_t i = 0; i < count; ++i)
        shooters[i]->fire();
}

std::tuple<bool, double> Invasion::move(double delta_seconds, double horizontal) const {
    if (!pool || delta_seconds <= 0.0 || horizontal == 0.0) return {false, 0.0};
    const auto shape = pool->formation_bounds();
    if (!shape) return {false, 0.0};
    const bool right = horizontal > 0.0;
    const double gap = right ? world_size.x - shape->maximum.x : shape->minimum.x;
    if (gap <= 0.0) return {true, delta_seconds};

    const double time_to_edge = gap / std::abs(horizontal);
    const double movement_seconds = std::min(delta_seconds, time_to_edge);
    for (std::size_t i = 0; i < formation_size; ++i) {
        auto& enemy = pool->enemies[i];
        enemy.move(movement_seconds, horizontal);
    }
    const bool edge_reached = delta_seconds >= time_to_edge;
    return {edge_reached, edge_reached ? delta_seconds - movement_seconds : 0.0};
}

void Invasion::descend() const {
    if (!pool) return;
    for (std::size_t i = 0; i < formation_size; ++i) {
        auto& enemy = pool->enemies[i];
        enemy.translate({0.0f, -10.0f});
    }
    const auto shape = pool->formation_bounds();
    if (shape && shape->minimum.y <= PlayerActor::spawn_height) {
        event_publisher.notify(SpaceInvadersEvents::game_over_event, SpaceInvadersEvents::GameOver{});
    }
}

void Invasion::update(const CE::Game::TickContext& tick) {
    if (!pool) return;
    for (auto& enemy : pool->enemies) {
        enemy.update(tick);
    }
    if (std::ranges::none_of(pool->enemies, [](const auto& enemy) {
        return enemy.is_alive() || enemy.is_exploding();
    })) {
        spawn();
    }
    pool->refresh_front_line();
}

void Invasion::append_draws(CE::Rendering::PassWriter& pass) const {
    if (!pool) return;
    for (const auto& enemy : pool->enemies) {
        enemy.append_draws(pass);
    }
}
