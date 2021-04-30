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

	gpro-net-RakNet-Server.cpp
	Source for RakNet server management.
*/

#include <sstream>

#include "gpro-net/gpro-net-server/gpro-net-RakNet-Server.hpp"
#include "gpro-net/gpro-net-server/Physics_Update.h"


namespace gproNet
{
	cRakNetServer::cRakNetServer()
	{
		RakNet::SocketDescriptor sd(SET_GPRO_SERVER_PORT, 0);
		unsigned short MAX_CLIENTS = 10;

		peer->Startup(MAX_CLIENTS, &sd, 1);
		peer->SetMaximumIncomingConnections(MAX_CLIENTS);

		InitializePhysicsObjects();
	}

	cRakNetServer::~cRakNetServer()
	{
		peer->Shutdown(0);
	}

	void cRakNetServer::PhysicsUpdate(double dt)
	{
		for (size_t i = 0; i < MAX_PHYSICS_OBJECTS; ++i)
		{
			if (physicsObjects->ownerID == 0)
			{
				updateVelocity(physicsObjects[i].velocity, physicsObjects[i].acceleration, dt);
				updatePosition(physicsObjects[i].position, physicsObjects[i].velocity, dt);
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
			// send new connection their index on our system
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

			// test message
		case ID_GPRO_MESSAGE_COMMON_END:
		{
			// server receives greeting, print it and send one back
			RakNet::BitStream bitstream_w;
			ReadTest(bitstream);
			//WriteTest(bitstream_w, "Hello client from server");
			//peer->Send(&bitstream_w, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, false);

			RakNet::BitStream trigger;
			WriteTimestamp(trigger);
			trigger.Write((RakNet::MessageID)ID_GPRO_COMMON_CLIENT_ID);
			trigger.Write(peer->NumberOfConnections());
			peer->Send(&trigger, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, false);

			trigger.Reset();
			WriteTimestamp(trigger);
			trigger.Write((RakNet::MessageID)ID_GPRO_COMMON_OTHER_CLIENT_ID);
			trigger.Write(peer->NumberOfConnections());
			peer->Send(&trigger, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, true);	// To all but sender
		}	return true;

		case ID_GPRO_COMMON_SEND_POSITION:
		{
			RenderSceneData dat;

			RenderSceneData::Read(bitstream, dat);
			
			RenderSceneData::Copy(physicsObjects[dat.ownerID], dat);
			
			RakNet::BitStream bitstream_out;
			bitstream_out.Write((RakNet::MessageID)ID_GPRO_COMMON_SEND_OBJECT_UPDATES);
			WritePhysicsData(dat.ownerID, bitstream_out);
			peer->Send(&bitstream_out, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, false);
		}

		}
		return false;
	}

	void cRakNetServer::InitializePhysicsObjects()
	{
		for (size_t i = 0; i < MAX_PHYSICS_OBJECTS; ++i)
		{
			physicsObjects[i].ownerID = 0;

			physicsObjects[i].position[0] = (float)((i % 8) * 3);
			physicsObjects[i].position[1] = (float)((i / 8) * 3);
			physicsObjects[i].position[2] = 0.0f;

			physicsObjects[i].velocity[0] = physicsObjects[i].velocity[1] = physicsObjects[i].velocity[2] = 0.0f;

			physicsObjects[i].acceleration[0] = physicsObjects[i].acceleration[1] = 0.0f;
			physicsObjects[i].acceleration[2] = PHYSICS_GRAVITY;
		}
	}

	void cRakNetServer::WritePhysicsData(short ownerID, RakNet::BitStream& out)
	{
		for (size_t i = 0; i < MAX_PHYSICS_OBJECTS; ++i)
		{
			if (physicsObjects[i].ownerID != ownerID)
				RenderSceneData::Write(out, physicsObjects[i]);
		}
	}
}