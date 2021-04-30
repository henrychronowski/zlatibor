#ifndef PHYSICS_UPDATE_H
#define PHYSICS_UPDATE_H

#include "gpro-net/gpro-net/RenderSceneData.h"

const float PHYSICS_GRAVITY = -9.81f;

void updateVelocity(float vel[3], float accel[3], double dt);
void updatePosition(float pos[3], float vel[3], double dt);

#endif // !PHYSICS_UPDATE_H