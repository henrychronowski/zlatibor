#include "gpro-net/gpro-net-server/Physics_Update.h"

void updateVelocity(float vel[3], float accel[3], float dt)
{
	for (int i = 0; i < 3; ++i)
	{
		vel[i] = dt * accel[i];
	}
}

void updatePosition(float pos[3], float vel[3], float dt)
{
	for (int i = 0; i < 3; ++i)
	{
		pos[i] = dt * vel[i];
	}
}