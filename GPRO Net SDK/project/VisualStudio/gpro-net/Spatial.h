#ifndef SPATIAL_H
#define SPATIAL_H

#include "gpro-net/gpro-net/gpro-net-RakNet.hpp"

struct sSpatialPose
{
	float scale[3];			// Non-uniform scale
	float rotate[4];		// Orientation as a quaternion
	float translation[3];	// Translation

	RakNet::BitStream& Write(RakNet::BitStream& bitstream)
	{
		
		return bitstream;
	}

	RakNet::BitStream& Read(RakNet::BitStream& bitstream)
	{

		return bitstream;
	}
};

int GetLargest(float input[], int size)
{
	float max = MININT; // Replace with minimum float value
	for (int i = 0; i < size; i++)
	{
		if (input[i] > max)
			max = input[i];
	}

	return max;
}

#endif // SPATIAL_H