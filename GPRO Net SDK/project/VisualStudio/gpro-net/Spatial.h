#ifndef SPATIAL_H
#define SPATIAL_H

#include "gpro-net/gpro-net/gpro-net-RakNet.hpp"

// NB: I could not think of the proper way to write the compressed data to the bitstream 
// so the write statements where I put them in for the compressed data are writing integers

// For a slight efficiency boost if sqrt(2) wasn't needed in isolation could cache sqrt(2)*511 instead
double const SQRT_2 = 1.414213562373095;

struct sSpatialPose
{
	float scale[3];			// Non-uniform scale
	float rotate[4];		// Orientation as a quaternion
	float translation[3];	// Translation

	// Package information into a 40 byte bitstream for sending 
	RakNet::BitStream& Write(RakNet::BitStream& bitstream)
	{
		int largest, i;
		float tmp;

		// Translation ********************************************//

		// Get the largest value
		largest = GetLargest(translation, 3);

		// Write the largest value as a float
		bitstream.Write(translation[largest]);

		// Write the index of the largest value as two bits
		bitstream.Write(largest);

		// Compress and write other two values to 30 bits
		for (i = 0; i < 3; i++)
		{
			// Don't act on the largest value
			if (i = largest)
				continue;

			// If not the largest value, scale to [-1,1]
			tmp = translation[i];
			tmp /= translation[largest];

			// Multiply by 767 to compress into the 15 bits
			tmp *= 767;

			// Write the data to the bitstream as 15 bits
			//bitstream.Write(<data>);
		}

		// Scale **************************************************//

		// Get the largest value
		largest = GetLargest(scale, 3);

		// Write the largest value as a float
		bitstream.Write(scale[largest]);

		// Write the index of the largest value as two bits
		bitstream.Write(largest);

		// Compress and write other two values to 30 bits
		for (i = 0; i < 3; i++)
		{
			// Don't act on the largest value
			if (i = largest)
				continue;

			// If not the largest value, scale to [-1,1]
			tmp = scale[i];
			tmp /= scale[largest];

			// Multiply by 767 to compress into the 15 bits
			tmp *= 767;

			// Write the data to the bitstream as 15 bits
			//bitstream.Write(<data>);
		}

		// Rotation ***********************************************//

		// Get largest component
		largest = GetLargest(rotate, 4);

		// Write the index of the largest component to the bitstream in the first 2 bits
		bitstream.Write(largest);

		// Compress and write other three values to 30 bits
		for (i = 0; i < 4; i++)
		{
			// Don't act on the largest value
			if (i = largest)
				continue;

			// If not the largest value, multiply by sqrt(2)*511 to compress to 10 bits
			tmp = rotate[i];
			tmp *= SQRT_2 * 511;

			// Write the data to the bitstream as 10 bits
			//bitstream.Write(<data>);
		}

		return bitstream;
	}

	// Unpack information from a bitstream for recieving
	RakNet::BitStream& Read(RakNet::BitStream& bitstream)
	{
		int largest, i;
		float tmp;

		// Translation ********************************************//
		
		// Read in largest element
		bitstream.Read(tmp);

		// Read in largest element index
		bitstream.Read(largest);
		translation[largest] = tmp;

		// Multiply remaining factos by <Largest Element>/767
		for (i = 0; i < 3; i++)
		{
			// Don't act on the largest value
			if (i = largest)
				continue;

			// Read in next value
			bitstream.Read(tmp);

			// If not the largest value, multiply by <Largest Element>/767
			tmp *= translation[largest] / 767;

			translation[i] = tmp;
		}

		// Scale **************************************************//
		
		// Read in largest element
		bitstream.Read(tmp);

		// Read in largest element index
		bitstream.Read(largest);
		scale[largest] = tmp;

		// Multiply remaining factos by <Largest Element>/767
		for (i = 0; i < 3; i++)
		{
			// Don't act on the largest value
			if (i = largest)
				continue;

			// Read in next value
			bitstream.Read(tmp);

			// If not the largest value, multiply by <Largest Element>/767
			tmp *= scale[largest] / 767;

			scale[i] = tmp;
		}
		 
		// Rotation ***********************************************//

		// Read in the index of largest element
		bitstream.Read(largest);

		// Divide componenets by 511 * sqrt(2)
		for (i = 0; i < 4; i++)
		{
			// Don't act on the largest value
			if (i = largest)
				continue;

			// Read in next value
			bitstream.Read(tmp);

			// If not the largest value, divide by 511 * sqrt(2)
			tmp /= SQRT_2 * 511;

			rotate[i] = tmp;
		}
		
		// Use sqrt(a^2 + b^2 + c^2 - 1) = w to find largest component value and store
		/*rotate[largest] = sqrt(see above equation)*/

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