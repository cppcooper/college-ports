#include <space_invaders/systems/bullet-mgr.h>
#include <space_invaders/game-objects/actors.h>
#include <space_invaders/systems/invasion.h>

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
}

void BulletManager::detect_collisions(PlayerActor* p, Invasion* i) {
    const auto removed = std::erase_if(bullets, [p, i](const auto& bullet) {
        const auto shape = bullet->bounds();
        if (bullet->is_players()) {
            if (!i) return false;
            for (auto& enemy : i->enemies()) {
                if (enemy.is_alive() && shape.overlaps(enemy.bounds())) {
                    enemy.hit();
                    return true;
                }
            }
        } else if (p && p->is_alive() && shape.overlaps(p->bounds())) {
            p->hit();
            return true;
        }
        return false;
    });
    if (removed) report_count();
}

void BulletManager::append_draws(CE::Rendering::PassWriter& pass) const {
    for (const auto& bullet : bullets)
        bullet->append_draws(pass);
}
