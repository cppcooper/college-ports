#pragma once

#include <stdexcept>
#include <cheryl/core/resources/asset-management/sprite-mgr.h>

enum class EnemyType {
    MALOLRIE,
    ARCHER,
    LANA,
    CHERYL,
    CYRIL,
    RAY,
    KRIEGER,
    WOODIE
};

[[nodiscard]] constexpr int points_for(EnemyType type) {
    switch (type) {
        case EnemyType::MALOLRIE: return 10;
        case EnemyType::ARCHER: return 20;
        case EnemyType::LANA: return 30;
        case EnemyType::CHERYL: return 40;
        case EnemyType::CYRIL: return 50;
        case EnemyType::RAY: return 60;
        case EnemyType::KRIEGER: return 70;
        case EnemyType::WOODIE: return 500;
    }
    throw std::invalid_argument("Unknown enemy type");
}

inline std::shared_ptr<const CE::Assets::Sprites::Asset> GetEnemySprite(const EnemyType type) {
    auto& assets = CE::Assets::Sprites::Manager::get();
    switch (type) {
        case EnemyType::MALOLRIE: return assets.get_asset("invaders", "enemy_1");
        case EnemyType::ARCHER: return assets.get_asset("invaders", "enemy_2");
        case EnemyType::LANA: return assets.get_asset("invaders", "enemy_3");
        case EnemyType::CHERYL: return assets.get_asset("invaders", "enemy_4");
        case EnemyType::CYRIL: return assets.get_asset("invaders", "enemy_5");
        case EnemyType::RAY: return assets.get_asset("invaders", "enemy_6");
        case EnemyType::KRIEGER: return assets.get_asset("invaders", "enemy_7");
        case EnemyType::WOODIE: return assets.get_asset("invaders", "enemy_8");
    }
    throw std::invalid_argument("Unknown enemy type");
}
