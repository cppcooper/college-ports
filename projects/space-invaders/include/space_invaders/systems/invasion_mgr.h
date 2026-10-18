#pragma once
#include <space_invaders/objects/enemies.h>
#include <../phys_obj.h>
#include <space_invaders/systems/screen_mgr.h>
#include <vector>

/**
 * Maintains the enemy formation used by the legacy game loop. SpawnNewWave()
 * creates ten columns of eight enemies and adjusts the movement speed for the
 * next wave. Update() moves the formation horizontally; reaching a side boundary
 * reverses its direction, adjusts its speed, and steps the enemies downward.
 * Individual enemy animation updates are dispatched by Screen_Mgr.
 *
 * GetV() stores the enemy columns in a static list shared by every instance,
 * including bullet_manager's Invasion_Formation base. The game updates a separate
 * invasion manager for formation movement, while bullet_manager reads the same
 * columns to select shooters, resolve player-shot hits, and spawn a replacement
 * wave when the list becomes empty. Movement speed and m_InvaderLock belong to
 * each instance, so that mutex alone does not coordinate access across instances.
 *
 * The spawning implementation registers each new enemy with Screen_Mgr for
 * per-object updates, drawing, and eventual deletion. The columns retain tracking
 * pointers: Clean_Up() removes enemies marked deleteme and discards empty columns;
 * Clean_Up(true) clears all formation references. Neither form deletes enemies.
 * The game must prune these references before Screen_Mgr synchronizes and deletes
 * the objects. Enemy::Death() handles scoring and the delay before marking an
 * enemy for removal.
 */
class Invasion_Formation {
    using invasion_columns = std::vector<phys_box*>;
    using invasion_forces = std::vector<invasion_columns>;
    using ushort = unsigned short;

protected:
    Screen_Mgr screen;
    void Add(invasion_columns col);
    inline void Flip_Direction();
    inline void Move_Invaders(double& seconds);

    static invasion_forces& GetV() {
        static invasion_forces Vector_of_Vectors_of_Aliens;
        return Vector_of_Vectors_of_Aliens;
    }

    float xincrements = 0;

public:
    virtual void Update(double& seconds);
    void Clean_Up(bool delete_all = false);
    void SpawnNewWave();
};
