#include <GFX\gl_engine.h>
#include <space_invaders/objects/Play.h>
#include <space_invaders/systems/BulletManager.h>

bullet::bullet(float x, float y, bool isplayer) {
    this->x = x;
    this->y = y;
    m_frameIndex = game_sprites::PEW;
    SpriteFrame& frame = (*s_sprite)[m_frameIndex];
    width = frame.Width() * frame.m_Scale;
    height = frame.Height() * frame.m_Scale;
    //  isplayer? UP Value : DOWN Value
    yincrements = isplayer ? -600 : 200;
}

//Position Update
void bullet::Update(double& seconds) {
    LOGFILE_1(_DEBUG3) << "Bullet Object: " << seconds;

    //Update Position
    y = y - (yincrements * seconds);

    //Check if I've hit the Screen Edge
    if (y < 0)
        deleteme = true;
}

bullet_manager::bullet_manager() {
    //We need a way to generate a number to represent how many bullets to be shot
    //The ratio of outputs cannot be linear
    /**/
    //INTERVALS: Represents how many shots can be made
    //WEIGHTS: Represents how often the corresponding number above will occur relative to the others
    std::vector<double> intervals{1, 2, 3, 4, 5, 7};
    std::vector<double> weights{12, 25, 16, 24, 21, 2};

    //We have our statistical data but now we need..
    //a random number distribution which follows our statistical model
    shots_distrib = std::piecewise_linear_distribution<>(intervals.begin(), intervals.end(), weights.begin());


    //Unimportant, initial values
    yesno_distrib = std::bernoulli_distribution(0.005);
    last_time_shotsmade = std::chrono::system_clock::now();
}

bool bullet_manager::x_overlap_check(bullet* B, phys_box* obj) {
    // Difference between A & B positions on X
    float x = obj->x - B->x;
    if (x < 0)
        x *= -1;

    // Combined distance between centres on X & Y separately
    float rx = (obj->width / 2) + (B->width / 2);

    if (x <= rx) {
        return true;
    }
    return false;
}

bool bullet_manager::y_overlap_check(bullet* B, phys_box* obj) {
    // Difference between A & B positions on Y
    float y = obj->y - B->y;
    if (y < 0)
        y *= -1;

    // Combined distance between centres on X & Y separately
    float ry = (obj->height / 2) + (B->height / 2);

    if (y <= ry) {
        return true;
    }
    return false;
}

//Performs all the functionality involving bullets (except: bullet position updates)
void bullet_manager::Update(double& seconds) {
    LOGFILE_1(_INFO);

    m_InvaderLock.lock();
    size_t size = GetV().size();
    m_InvaderLock.unlock();
    if (size == 0) {
        SpawnNewWave();
    }

    // If the player pointer isn't == nullptr we're in business
    if (theplayer) {
        std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - last_time_shotsmade;

        //TODO: Revise RNG code - You know what to do

        //Generate random boolean state deciding whether the alien(s) will shoot
        double P = (((double)elapsed.count()) / 500);

        m_BulletsLock.lock();
        int size = m_Bullets.size() + 1;
        P = P + ((1 - ((m_Bullets.size() + 1) / 30)) / 1000) + 0.02;
        m_BulletsLock.unlock();

        if (P > 0 && P <= 1) {
            yesno_distrib = std::bernoulli_distribution(P);
        } else if (P > 1) {
            yesno_distrib = std::bernoulli_distribution(0.5);
        } else {
            yesno_distrib = std::bernoulli_distribution(0.00001);
        }

        //Shall we generate bullets or not? (if true) {make bullets fly}, else don't
        if (yesno_distrib(RNG)) {
            Enemies_Shoot();
        }

        //We now must perform a collision check for every ENEMY Bullet AGAINST the PLAYER
        PlayerCollisions();

        //We now check the PLAYER Bullet AGAINST any and all ENEMIES
        EnemyCollisions();
    }
}

//Encapsulate collision loop for Enemies vs the Player Bullet
void bullet_manager::EnemyCollisions() {
    if (theplayer->playerbullet) {
        if (theplayer->playerbullet->y > Screen::Height()) {
            delete theplayer->playerbullet;
            theplayer->playerbullet = nullptr;
            return;
        }

        m_InvaderLock.lock();
        auto& Invaders = GetV();
        //The bullet could collide, so we check if it intersects anything
        for (auto column : Invaders) {
            if (x_overlap_check(theplayer->playerbullet, (*column.begin()))) {
                for (auto rit = column.rbegin(); rit != column.rend(); ++rit) {
                    phys_box* InvaderPtr = *rit;
                    if (!InvaderPtr->deleteme && y_overlap_check(theplayer->playerbullet, InvaderPtr)) {
                        //X and Y overlap checked out, we've got a casualty
                        ((Enemy*)InvaderPtr)->Death();
                        delete theplayer->playerbullet;
                        theplayer->playerbullet = nullptr;
                        m_InvaderLock.unlock();
                        return;
                    }
                }
            }
        }
        m_InvaderLock.unlock();
    }
}

//Encapsulate collision loop for the Player vs Enemy Bullet(s)
void bullet_manager::PlayerCollisions() {
    m_BulletsLock.lock();
    for (auto B : m_Bullets) {
        if (!B->isHidden() && x_overlap_check(B, theplayer)) {
            if (y_overlap_check(B, theplayer)) {
                theplayer->Death();
                m_BulletsLock.unlock();
                Clear_Chamber();

                //Let's not see what happens if we keep iterating through the now empty vector
                return;
            }
        }
    }
    m_BulletsLock.unlock();
}

//Big complicated mess of logic determining how many bullets to shoot and from where
void bullet_manager::Enemies_Shoot() {
    using ushort = unsigned short;
    last_time_shotsmade = std::chrono::system_clock::now();
    auto& Invaders = GetV();
    ushort bullets_to_make = -1;

    m_InvaderLock.lock();
    do {
        bullets_to_make = shots_distrib(RNG);
    } while (bullets_to_make > Invaders.size());
    column_distrib = std::uniform_int_distribution<int>(0, Invaders.size() - 1);
    m_InvaderLock.unlock();

    ushort col_index;
    ushort* index_array = new unsigned short[bullets_to_make];
    for (int b = 0; b < bullets_to_make; ++b) {
    REDECIDE:
        col_index = column_distrib(RNG);
        for (int i = 0; i < b; ++i) {
            if (col_index == index_array[i]) {
                goto REDECIDE;
            }
        }
        index_array[b] = col_index;
    }

    for (int i = 0; i < bullets_to_make; ++i) {
        m_InvaderLock.lock();
        auto col_it = Invaders.begin() + index_array[i];
        for (auto rit = col_it->rbegin(); rit != col_it->rend(); ++rit) {
            auto InvaderPtr = *rit;
            if (!InvaderPtr->deleteme) {
                ushort x = InvaderPtr->x;
                ushort y = InvaderPtr->y;
                bullet* bullet_ptr = new bullet(x, y - 50);

                m_BulletsLock.lock();
                m_Bullets.push_back(bullet_ptr);
                m_BulletsLock.unlock();

                screen.Add((ScreenObject*)bullet_ptr);
                break;
            }
        }
        m_InvaderLock.unlock();
    }
    delete index_array;
}

//Shoots a bullet if there isn't already one
void bullet_manager::Player_Shoots() {
    if (!theplayer->playerbullet)
        theplayer->playerbullet = new bullet(theplayer->GetX(), theplayer->GetY(), true);
}

//Removes dead bullets
void bullet_manager::Clean_Up() {
    m_BulletsLock.lock();
    auto Bullet_itor = m_Bullets.begin();
    int index = 0;
    while (Bullet_itor != m_Bullets.end()) {
        LOGFILE_1(_DEBUG4) << "Enemy Bullets: \t" << ((__int64)(*Bullet_itor));
        //If we find a bullet wishing to be deleted we remove it from our container
        if ((*Bullet_itor)->deleteme) {
            m_Bullets.erase(Bullet_itor);
            //iterator may have ^ been invalidated
            Bullet_itor = m_Bullets.begin() + index;
        } else {
            //This is the only place we *increment* our index and iterator
            Bullet_itor++;
            index++;
        }
    }
    m_BulletsLock.unlock();
}

//Kills all the bullets
void bullet_manager::Clear_Chamber() {
    m_BulletsLock.lock();
    for (auto B : m_Bullets) {
        B->deleteme = true;
    }
    m_Bullets.clear();
    m_BulletsLock.unlock();
}
