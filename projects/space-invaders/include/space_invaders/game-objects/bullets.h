#pragma once

#include <cheryl/core/game-framework/tick-context.h>
#include <cheryl/core/resources/asset-management/sprite-mgr.h>
#include <cheryl/core/resources/memory/allocators/object-pool-allocator.hpp>

#include <cstddef>
#include <new>

#include <space_invaders/game-objects/world-object.h>

class Bullet final : public WorldObject {
    using Allocator = CE::Memory::Allocators::ObjectPoolAllocator<Bullet>;
    double speed;

    static Allocator& allocator() {
        static Allocator instance = [] {
            Allocator resource;
            constexpr std::size_t initial_pool_capacity = 100;
            auto* storage = resource.allocate(initial_pool_capacity);
            resource.deallocate(storage, initial_pool_capacity);
            return resource;
        }();
        return instance;
    }

public:
    Bullet(const glm::vec2 position, const double speed)
    : WorldObject(CE::Assets::Sprites::Manager::get().get_asset("invaders", "bullet"), position), speed(speed) {}

    static void preallocate() { static_cast<void>(allocator()); }
    static void* operator new(std::size_t size) {
        if (size != sizeof(Bullet)) throw std::bad_alloc{};
        return allocator().allocate(1);
    }
    static void operator delete(void* pointer) noexcept {
        if (pointer) allocator().deallocate(static_cast<Bullet*>(pointer), 1);
    }

    void update(const CE::Game::TickContext& tick) override {
        translate({0.0f, static_cast<float>(speed * tick.delta_seconds)});
    }

    [[nodiscard]] bool is_players() const {
        return speed > 0;
    }
};
