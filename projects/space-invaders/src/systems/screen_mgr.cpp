#include <space_invaders/objects/Play.h>
#include <space_invaders/objects/enemies.h>
#include <space_invaders/objects/screen_objs.h>
#include <space_invaders/systems/screen_mgr.h>

std::mutex Screen_Mgr::s_Lock;
std::vector<ScreenObject*> Screen_Mgr::s_Array;

Screen_Mgr::~Screen_Mgr() {
    Clean_Up(true);
}

void Screen_Mgr::Add(ScreenObject* obj) {
    LOGFILE_1(_INFO) << callstack;
    s_Lock.lock();
    s_Array.push_back(obj);
    s_Lock.unlock();
}

void Screen_Mgr::Clean_Up(bool delete_all) {
    LOGFILE_1(_INFO) << callstack;
    s_Lock.lock();
    if (delete_all) {
        while (s_Array.begin() != s_Array.end()) {
            auto P = *s_Array.begin();
            P->Sync();
            delete P;
            s_Array.erase(s_Array.begin());
        }
        assert(s_Array.empty());
    } else {
        size_t index = 1;
        auto it = s_Array.begin();
        while (it != s_Array.end()) {
            ScreenObject* obj = *it;
            if (obj->isHidden()) {
                obj->Sync();
                index -= 1;
                switch (obj->getTYPEID()) {
                    case 0:
                        delete (Mal*)obj;
                        break;

                    case 2:
                        delete (Archie*)obj;
                        break;

                    case 4:
                        delete (Lana*)obj;
                        break;

                    case 6:
                        delete (Cheryl*)obj;
                        break;

                    case 8:
                        delete (Cyril*)obj;
                        break;

                    case 10:
                        delete (Ray*)obj;
                        break;

                    case 12:
                        delete (Kriege*)obj;
                        break;

                    case 14:
                        delete (Woodie*)obj;
                        break;

                    case 15:
                        exit(-16);

                    case 16:
                        delete (bullet*)obj;
                        break;

                    case 17:
                        exit(-16);

                    default:
                        exit(-16);
                }
                obj = nullptr;
                s_Array.erase(it);
            }
            it = s_Array.begin() + index++;
        }
    }
    s_Lock.unlock();
}

void Screen_Mgr::Update_Objects(double& seconds) {
    LOGFILE_1(_INFO) << callstack;
    s_Lock.lock();
    LOGFILE_1(_DEBUG1) << "Screen Manager Update"
        << newl << "Screen Manager: " << seconds
        << newl << "Vector Size: " << s_Array.size();
    for (auto obj : s_Array) {
        if (!obj->isHidden()) {
            //TODO: DEBUG Iterator Dereferencing Assertion
            //This code is sometimes run after an object is added or deleted
            obj->Update(seconds);
        }
    }
    s_Lock.unlock();
}

void Screen_Mgr::Draw() {
    LOGFILE_1(_INFO) << callstack;
    s_Lock.lock();
    for (auto obj : s_Array) {
        if (!obj->isHidden()) {
            obj->Draw();
        }
    }
    s_Lock.unlock();
}
