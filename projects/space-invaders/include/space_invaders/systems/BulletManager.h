#pragma once
#include <space_invaders/objects/screen_objs.h>
#include <space_invaders/systems/invasion_mgr.h>
#include <random>
#include <chrono>

class player;

class bullet : public ScreenObject {
    friend class bullet_manager;

private:
    float yincrements;

protected:
public:
    bullet(float x, float y, bool isplayer = false);
    virtual ~bullet() {}
    void deleteSelf() final override { delete this; }
    void Update(double& seconds);
    void Sync() {}
};

//TODO: Bullet_Manager should have a separate header
//Design-wise Invasion_Formation should not be apart of this
/**
 * Coordinates firing, projectile collisions, and replacement waves in the legacy
 * game loop. Update() uses elapsed time and the enemy bullet count to decide when
 * to fire a random volley, then checks enemy shots against the player and the
 * player's shot against the invaders. Hits call player::Death() or Enemy::Death();
 * those objects notify the game about lives and scoring.
 *
 * Protected inheritance from Invasion_Formation gives access to the shared enemy
 * columns for choosing shooters and finding targets. Update() calls SpawnNewWave()
 * when that list is empty; it does not call Invasion_Formation::Update(). The game
 * separately drives its invasion manager to move the formation and prune enemies.
 *
 * Enemy shots are tracked in m_Bullets and registered with Screen_Mgr, which moves,
 * draws, and eventually deletes them. Clean_Up() only removes marked pointers from
 * m_Bullets; Clear_Chamber() marks every enemy shot and clears that tracking list.
 * These references must be removed before Screen_Mgr deletes the corresponding
 * objects. m_BulletsLock protects this manager's enemy bullet list.
 *
 * The game supplies the non-owning theplayer pointer before shooting or calling
 * collision methods directly. Player_Shoots() creates at most one playerbullet;
 * this shot stays outside Screen_Mgr and m_Bullets. The game updates and draws it
 * separately, this manager deletes it on impact or after it leaves the top edge,
 * and the player destructor deletes any remaining shot.
 */
class bullet_manager : protected Invasion_Formation {
    Screen_Mgr screen;

    std::default_random_engine RNG;
    std::bernoulli_distribution yesno_distrib;
    std::uniform_int_distribution<int> column_distrib;
    std::piecewise_linear_distribution<double> shots_distrib;

    std::chrono::time_point<std::chrono::system_clock> last_time_shotsmade;
    std::vector<bullet*> m_Bullets;

protected:
    bool x_overlap_check(bullet* B, phys_box* obj);
    bool y_overlap_check(bullet* B, phys_box* obj);
    void Clear_Chamber();

public:
    bullet_manager();
    player* theplayer = nullptr;
    void Update(double& seconds);
    void Clean_Up();
    void Player_Shoots();
    void Enemies_Shoot();
    void EnemyCollisions();
    void PlayerCollisions();
};
