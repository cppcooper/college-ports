#ifndef _PHYS_OBJ_H_
#define _PHYS_OBJ_H_

#include <atomic>

struct point
{
	float x;
	float y;
	float z;
};

struct atomic_point
{
	std::atomic<float> x;
	std::atomic<float> y;
	std::atomic<float> z;
};

struct phys_box : public atomic_point
{
	std::atomic<bool> deleteme = false;
	float width;
	float height;
	//~phys_box(){}
};

#endif
