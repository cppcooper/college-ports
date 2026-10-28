#include <space_invaders/systems/bullet-mgr.h>

void BulletManager::init_simulation() {
    if (!events) {
        throw std::logic_error("Initialize BulletManager platform before simulation");
    }
    unregister_listener();
    Bullet::preallocate();
    shot_listener = events->register_listener(
        SpaceInvadersEvents::shot_requested_event,
        [this](const SpaceInvadersEvents::ShotRequested& shot) {
            bullets.push_back(std::make_unique<Bullet>(shot.position, shot.speed));
            report_count();
        }
    );
}

void BulletManager::update(const CE::Game::TickContext& tick) {
    for (const auto& bullet : bullets) {
        bullet->update(tick);
    }
    const auto removed = std::erase_if(bullets, [this](const auto& bullet) {
        const auto shape = bullet->bounds();
        return shape.maximum.y < 0.0f || shape.minimum.y > world_size.y;
    });
    if (removed) report_count();
    // Actor collision responses remain unfinished.
}

void BulletManager::append_draws(CE::Rendering::PassWriter& pass) const {
    for (const auto& bullet : bullets)
        bullet->append_draws(pass);
}
