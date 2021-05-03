#ifndef PHYSICS_UPDATE_H
#define PHYSICS_UPDATE_H

#include "gpro-net/gpro-net/RenderSceneData.h"

// Utility functions for calculating rudimentary kinematics for RenderSceneData objects
// Written by Henry Chronowski and Ethan Heil

// Constant gravitational acceleration value, arbitrary
const float PHYSICS_GRAVITY = -3.0f;

// updateVelocity
		//	Calculates an updated velocity given a current velocity, current acceleration, and delta time
		//		param vel[3]: the current velocity in xyz
		//		param accel[3]: the current acceleration in xyz
		//		param dt: the delta time in seconds to calculate kinematics over
		//		return: void
void updateVelocity(float vel[3], float accel[3], double dt);

// updatePosition
		//	Calculates an updated position given a current position, current velocity, and delta time
		//		param vel[3]: the current position in xyz
		//		param accel[3]: the current velocity in xyz
		//		param dt: the delta time in seconds to calculate kinematics over
		//		return: void
void updatePosition(float pos[3], float vel[3], double dt);

#endif // !PHYSICS_UPDATE_H