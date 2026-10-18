#include <space_invaders/systems/invasion_mgr.h>
#include <vector>

// Makes all the changes needed when changing the invasion direction
void Invasion_Formation::Flip_Direction() {
    m_InvaderLock.lock();
    xincrements = -1 * (xincrements + 30);
    xincrements = xincrements > 0 ? (xincrements < 300 ? xincrements : 300) : (xincrements > -300 ? xincrements : -300);
    for (auto col_iter = GetV().begin(); col_iter != GetV().end(); ++col_iter) {
        for (auto invader_iter = col_iter->begin(); invader_iter != col_iter->end(); ++invader_iter) {
            if ((*invader_iter)->deleteme) {
                continue;
            } else {
                (*invader_iter)->y = (*invader_iter)->y - 10;
            }
        }
    }
    m_InvaderLock.unlock();
}

// Moves invaders across the screen
void Invasion_Formation::Move_Invaders(double& seconds) {
    m_InvaderLock.lock();
    auto& Invaders = GetV();
    for (auto col_it = Invaders.begin(); col_it != Invaders.end(); ++col_it) {
        for (auto InvaderPtr : *col_it) {
            if (InvaderPtr->deleteme) {
                continue;
            } else {
                InvaderPtr->x = InvaderPtr->x + (xincrements * seconds);
            }
        }
    }
    m_InvaderLock.unlock();
}

// Performs screen collisions for Invaders and updates enemy positions
void Invasion_Formation::Update(double& seconds) {
    //We will need to know whether to zig or to zag
    static bool zagging = true;
    float xpos;

    m_InvaderLock.lock();
    auto& Invaders = GetV();

    //zagging = Moving rightwards
    if (zagging) {
        auto col_rit = Invaders.rbegin();
        for (; col_rit != Invaders.rend(); ++col_rit) {
            for (auto InvaderPtr : *col_rit) {
                if (InvaderPtr->deleteme) {
                    continue;
                } else {
                    xpos = InvaderPtr->x;
                    goto EXITZAGLOOP;
                }
            }
        }
        m_InvaderLock.unlock();
        return;
    EXITZAGLOOP:
        m_InvaderLock.unlock();
        if ((xpos + 50) >= 500/*Screen::Width()*/) {
            Flip_Direction();
            zagging = !zagging;
        }
        Move_Invaders(seconds);
    }
    //zigging = Moving leftwards
    else {
        auto col_it = Invaders.begin();
        for (; col_it != Invaders.end(); ++col_it) {
            for (auto InvaderPtr : *col_it) {
                if (InvaderPtr->deleteme) {
                    continue;
                } else {
                    xpos = InvaderPtr->x;
                    goto EXITZIGLOOP;
                }
            }
        }
        m_InvaderLock.unlock();
        return;
    EXITZIGLOOP:
        m_InvaderLock.unlock();
        if ((xpos - 50) <= 0) {
            Flip_Direction();
            zagging = !zagging;
        }
        Move_Invaders(seconds);
    }
}

// Safely cleans the Invasion's Invader List
void Invasion_Formation::Clean_Up(bool delete_all) {
    //Locking the singleton array
    m_InvaderLock.lock();
    if (!delete_all) {
        auto col_iter = GetV().begin();
        int col_index = 0;
        while (col_iter != GetV().end()) {
            auto Invader_iter = col_iter->begin();
            int row_index = 0;
            while (Invader_iter != col_iter->end()) {
                if ((*Invader_iter)->deleteme) {
                    col_iter->erase(Invader_iter);
                    Invader_iter = col_iter->begin() + row_index;
                } else {
                    Invader_iter++;
                    row_index++;
                }
            }
            if (col_iter->size() == 0) {
                GetV().erase(col_iter);
                col_iter = GetV().begin() + col_index;
            } else {
                col_iter++;
                col_index++;
            }
        }
    } else {
        //We want to remove everything
        GetV().clear();
    }
    m_InvaderLock.unlock();
}

// Spawns a hard-coded number of columns with 8 Enemies per column
void Invasion_Formation::SpawnNewWave() {
    static int round = 1;
    if (xincrements > 0) {
        xincrements = 40 + (10 * round++) + (xincrements / 100);
    } else if (xincrements < 0) {
        xincrements = (xincrements / 100) - 40 - (10 * round++);
    } else {
        xincrements = 40 + (10 * round++) + (xincrements / 100);
    }

    for (int i = 0; i < 10; ++i) {
        std::vector<phys_box*> column;

        Enemy* myEnemy = new Woodie();
        screen.Add((ScreenObject*)myEnemy);
        column.push_back((phys_box*)myEnemy);

        myEnemy = new Kriege();
        screen.Add((ScreenObject*)myEnemy);
        column.push_back((phys_box*)myEnemy);

        myEnemy = new Ray();
        screen.Add((ScreenObject*)myEnemy);
        column.push_back((phys_box*)myEnemy);

        myEnemy = new Cyril();
        screen.Add((ScreenObject*)myEnemy);
        column.push_back((phys_box*)myEnemy);

        myEnemy = new Cheryl();
        screen.Add((ScreenObject*)myEnemy);
        column.push_back((phys_box*)myEnemy);

        myEnemy = new Lana();
        screen.Add((ScreenObject*)myEnemy);
        column.push_back((phys_box*)myEnemy);

        myEnemy = new Archie();
        screen.Add((ScreenObject*)myEnemy);
        column.push_back((phys_box*)myEnemy);

        myEnemy = new Mal();
        screen.Add((ScreenObject*)myEnemy);
        column.push_back((phys_box*)myEnemy);

        Add(column);
    }
}

void Invasion_Formation::Add(invasion_columns col) {
    m_InvaderLock.lock();
    GetV().push_back(col);
    m_InvaderLock.unlock();
}
