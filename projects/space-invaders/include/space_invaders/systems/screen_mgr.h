#pragma once
#include <space_invaders/objects/screen_objs.h>

/**
 * Owns the registered screen objects in the legacy game loop. Every Screen_Mgr
 * instance uses the same static s_Array and s_Lock, so enemies registered by
 * Invasion_Formation and enemy shots registered by bullet_manager are visible
 * through the game's Screen_Mgr instance too.
 *
 * Update_Objects() calls Update() on visible objects to advance enemy animation
 * and enemy-shot positions; Draw() dispatches their draw calls. The game calls
 * Invasion_Formation separately for collective enemy movement and bullet_manager
 * for firing and collision checks. The player and its single shot are updated
 * and drawn separately by the game and are not registered here.
 *
 * Clean_Up() calls Sync() and deletes objects whose deleteme flag makes isHidden()
 * true. Invasion_Formation::Clean_Up() and bullet_manager::Clean_Up() must first
 * discard their tracking references to those objects. Sync() joins an enemy's
 * death thread before destruction. Clean_Up(true), also called by the destructor,
 * deletes every registered object; other managers must clear or stop using their
 * references as part of shutdown. s_Lock serializes operations on the shared
 * registry; the game coordinates these operations with the other managers.
 */
class Screen_Mgr {
private:
    static std::mutex s_Lock;
    static std::vector<ScreenObject*> s_Array;

public:
    ~Screen_Mgr();
    void Add(ScreenObject* obj);
    void Clean_Up(bool delete_all = false);
    void Update_Objects(double& seconds);
    void Draw();
};
