#include "gpro-net/gpro-net-server/Physics_Update.h"

void updateVelocity(float vel[3], float accel[3], double dt)
{
	for (int i = 0; i < 3; ++i)
	{
		vel[i] += (float)(dt * accel[i]);
	}
}

void updatePosition(float pos[3], float vel[3], double dt)
{
	for (int i = 0; i < 3; ++i)
	{
		pos[i] += (float)(dt * vel[i]);
	}
}