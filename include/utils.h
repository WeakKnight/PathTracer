#pragma once

#include <math.h>
#include "cyVector.h"

using namespace cy;

#define RANDOM_THRESHOLD 0.05f 

Vec3f RandomInUnitSphere();

Vec2f NonUniformRandomPointInCircle(float radius);

Vec2f RandomPointInCircle(float radius);