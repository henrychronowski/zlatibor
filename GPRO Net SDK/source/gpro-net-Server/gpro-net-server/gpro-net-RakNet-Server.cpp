/*
   Copyright 2021 Daniel S. Buckstein

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

/*
	GPRO Net SDK: Networking framework.
	By Daniel S. Buckstein
	Extended by Henry Chronowski

	gpro-net-RakNet-Server.cpp
	Source for RakNet server management.
*/

#include <sstream>

#include "gpro-net/gpro-net-server/gpro-net-RakNet-Server.hpp"
#include "gpro-net/gpro-net/gpro-net-util/Physics_Update.h"


namespace gproNet
{
	cRakNetServer::cRakNetServer()
	{
		RakNet::SocketDescriptor sd(SET_GPRO_SERVER_PORT, 0);
		unsigned short MAX_CLIENTS = 10;

		peer->Startup(MAX_CLIENTS, &sd, 1);
		peer->SetMaximumIncomingConnections(MAX_CLIENTS);

		// Set the starter physics data for all objects
		InitializePhysicsObjects();
	}

	cRakNetServer::~cRakNetServer()
	{
		peer->Shutdown(0);
	}

	void cRakNetServer::PhysicsUpdate(double dt)
	{
		// Iterate through all physics objects
		for (size_t i = 0; i < MAX_PHYSICS_OBJECTS; ++i)
		{
			// If the object is managed by the server calculate kinematics
			if (physicsObjects->ownerID == 0)
			{
				updateVelocity(physicsObjects[i].velocity, physicsObjects[i].acceleration, dt);
				updatePosition(physicsObjects[i].position, physicsObjects[i].velocity, dt);

				// If the object has passed below a threshold, reposition it to the top of the area (in order to keep them visible for longer)
				if (physicsObjects[i].position[2] <= -100)
				{
					physicsObjects[i].velocity[2] = 0.0f;
					physicsObjects[i].position[2] = 100.0f;
				}
			}
		}
	}

	bool cRakNetServer::ProcessMessage(RakNet::BitStream& bitstream, RakNet::SystemAddress const sender, RakNet::Time const dtSendToReceive, RakNet::MessageID const msgID)
	{
		if (cRakNetManager::ProcessMessage(bitstream, sender, dtSendToReceive, msgID))
			return true;

		// server-specific messages
		switch (msgID)
		{
		case ID_NEW_INCOMING_CONNECTION:
		{
			// send new connection their index on server
			RakNet::BitStream bitstream_w;
			bitstream_w.Write((RakNet::MessageID)ID_GPRO_MESSAGE_COMMON_BEGIN);
			bitstream_w.Write(peer->GetIndexFromSystemAddress(sender));
			peer->Send(&bitstream_w, IMMEDIATE_PRIORITY, RELIABLE_ORDERED, 0, sender, false);
		}	return true;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			//printf("The server is full.\n");
			return true;
		case ID_DISCONNECTION_NOTIFICATION:
			//printf("A client has disconnected.\n");
			return true;
		case ID_CONNECTION_LOST:
			//printf("A client lost the connection.\n");
			return true;

		// Recieved when a new client has successfully connected
		case ID_GPRO_COMMON_NEW_PLAYER:
		{
			// Notify the client of their ownerID in the server's records
			RakNet::BitStream trigger;
			WriteTimestamp(trigger);
			trigger.Write((RakNet::MessageID)ID_GPRO_COMMON_CLIENT_ID);
			trigger.Write(peer->NumberOfConnections());	// NumberOfConnections = OwnerID of the new client
			peer->Send(&trigger, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, false);

			// Send the current physics data for all objects in order to initialize the objects on the client
			trigger.Reset();
			trigger.Write((RakNet::MessageID)ID_GPRO_COMMON_INITIAL_PARAMETERS);
			WritePhysicsData(peer->NumberOfConnections(), trigger);
			peer->Send(&trigger, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, false);

			// Send the ownerID of the new client to all other connected clients
			trigger.Reset();
			WriteTimestamp(trigger);
			trigger.Write((RakNet::MessageID)ID_GPRO_COMMON_OTHER_CLIENT_ID);
			trigger.Write(peer->NumberOfConnections());
			peer->Send(&trigger, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, true);	// To all but sender
		}	return true;

		//	Recieved when a client updates the server with their position, responds by sending updated physics data to the client
		case ID_GPRO_COMMON_SEND_POSITION:
		{
			// Read in the new physics data for the client and store it
			RenderSceneData dat;
			RenderSceneData::Read(bitstream, dat);
			RenderSceneData::Copy(physicsObjects[dat.ownerID], dat);
			
			// Send the updated physics data for server owned objects and objects owned by other clients back to the sender client
			RakNet::BitStream bitstream_out;
			bitstream_out.Write((RakNet::MessageID)ID_GPRO_COMMON_SEND_OBJECT_UPDATES);
			WritePhysicsData(dat.ownerID, bitstream_out);
			peer->Send(&bitstream_out, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, false);
		}

		// Recieved during the process of initial client connection, contains the initial physics data of the client's owned object
		case ID_GPRO_COMMON_INITIAL_CLIENT_PARAMETERS:
		{
			// Read in and store the physics data for the newly client-owned object
			RenderSceneData dat;
			RenderSceneData::Read(bitstream, dat);
			RenderSceneData::Copy(physicsObjects[dat.ownerID], dat);

			// Send the updated physics data for the new client to all other connected clients
			RakNet::BitStream bitstream_out;
			bitstream_out.Write((RakNet::MessageID)ID_GPRO_COMMON_INITIAL_CLIENT_PARAMETERS);
			RenderSceneData::Write(bitstream_out, dat);
			peer->Send(&bitstream_out, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, true);	// To all but sender
		}

		}
		return false;
	}

	void cRakNetServer::InitializePhysicsObjects()
	{
		// Iterate through all physics objects and initialize them to defaults
		for (size_t i = 0; i < MAX_PHYSICS_OBJECTS; ++i)
		{
			// Server-managed by default
			physicsObjects[i].ownerID = 0;

			// Positions objects in a grid
			physicsObjects[i].position[0] = (float)((i % 8) * 3);
			physicsObjects[i].position[1] = (float)((i / 8) * 3);
			physicsObjects[i].position[2] = 0.0f;

			// Gives the objects a random initial velocity in the positive z direction
			physicsObjects[i].velocity[0] = (float)(rand() % 30 - 14);
			physicsObjects[i].velocity[1] = (float)(rand() % 30 - 14);
			physicsObjects[i].velocity[2] = (float)(rand() % 10 + 1);

			// Sets the objects acceleration to a constant gravity
			physicsObjects[i].acceleration[0] = physicsObjects[i].acceleration[1] = 0.0f;
			physicsObjects[i].acceleration[2] = PHYSICS_GRAVITY;
		}
	}

	void cRakNetServer::WritePhysicsData(short ownerID, RakNet::BitStream& out)
	{
		// Iterate through all physics objects
		for (size_t i = 0; i < MAX_PHYSICS_OBJECTS; ++i)
		{
			// Write physics data to the bitstream, ignoring any owned by the given ownerID
			if (physicsObjects[i].ownerID != ownerID)
				RenderSceneData::Write(out, physicsObjects[i]);
		}
	}
}