#include "gpro-net/gpro-net/gpro-net-util/Physics_Update.h"

// Function definitions for utility functions calculating rudimentary kinematics for RenderSceneData objects
// Written by Henry Chronowski and Ethan Heil

void updateVelocity(float vel[3], float accel[3], double dt)
{
	// Calculate the x, y, and z updated velocities using the formula of v1 = v0 + a*dt
	for (int i = 0; i < 3; ++i)
	{
		vel[i] += (float)(dt * accel[i]);
	}
}

void updatePosition(float pos[3], float vel[3], double dt)
{
	// Calculate the x, y, and z updated positions using the formula of p1 = p0 + v*dt
	for (int i = 0; i < 3; ++i)
	{
		pos[i] += (float)(dt * vel[i]);
	}
}