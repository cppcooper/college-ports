#pragma once
#include <cheryl/core/resources/asset-management/sprite-mgr.h>
#include <cheryl/core/resources/memory/allocators/object-pool-allocator.hpp>
#include <cheryl/templates/observed-variables.h>
#include <cheryl/assets/types/2d/sprite.h>

#include <chrono>
#include <cstddef>
#include <new>
#include <random>
#include <vector>

#include <vec2.hpp>

#include "space_invaders/events.h"

namespace CEAsset {
    using Sprite = CE::Assets::Sprites::Asset;
}

class bullet {
    using Allocator = CE::Memory::Allocators::ObjectPoolAllocator<bullet>;
    static constexpr std::size_t initial_pool_capacity = 100;

    std::shared_ptr<CEAsset::Sprite> sprite;
    glm::vec2 position;
    float speed;

    static Allocator& allocator() {
        static Allocator instance = [] {
            Allocator resource;
            auto* storage = resource.allocate(initial_pool_capacity);
            resource.deallocate(storage, initial_pool_capacity);
            return resource;
        }();
        return instance;
    }

public:
    // Optional startup initialization; the first allocation also initializes the pool.
    static void preallocate() {
        static_cast<void>(allocator());
    }

    void* operator new(std::size_t size) {
        if (size != sizeof(bullet)) {
            throw std::bad_alloc{};
        }
        return allocator().allocate(1);
    }

    void operator delete(void* pointer) noexcept {
        if (pointer) {
            allocator().deallocate(static_cast<bullet*>(pointer), 1);
        }
    }

    bullet(const glm::vec2 position, const float speed)
    : sprite(CE::Assets::Sprites::Manager::get().get_asset("invaders", "bullet")), position(position), speed(speed) {}

    void update(double& seconds) {

    }
};

class bullet_manager {
    using BulletCount = CE::State::ObservedVariable<int>;
    std::vector<std::unique_ptr<bullet>> m_Bullets;
    BulletCount bullet_count;
    CE::Events::EventBus::Registration shot_listener;

protected:
    void register_listener(CE::Events::EventBus& events) {
        shot_listener = events.register_listener(
            SpaceInvadersEvents::shot_requested_event,
            [this](const SpaceInvadersEvents::ShotRequested& shot) {
                m_Bullets.push_back(std::make_unique<bullet>(shot.position, static_cast<float>(shot.speed)));
                bullet_count.set(static_cast<int>(m_Bullets.size()));
            }
        );
    }

public:
    explicit bullet_manager(BulletCount::Callback observer)
    : bullet_count(0, {std::move(observer)}) {}

    void Update(double& seconds);
    void Clean_Up();
    void Player_Shoots();
    void Enemies_Shoot();
    void EnemyCollisions();
    void PlayerCollisions();
};
