#pragma once

struct iShoot {
    virtual ~iShoot() = default;
    virtual void fire() = 0;
};
