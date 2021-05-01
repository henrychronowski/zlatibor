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

	gpro-net-RakNet-Client.cpp
	Source for RakNet client management.
*/

#include "gpro-net/gpro-net-client/gpro-net-RakNet-Client.hpp"
#include "gpro-net/gpro-net/RenderSceneData.h"
#include "gpro-net/gpro-net/gpro-net-util/Physics_Update.h"
#include <sstream>
#include <cmath>

namespace gproNet
{
	cRakNetClient::cRakNetClient()
		: index(-1)
	{
		RakNet::SocketDescriptor sd;
		char SERVER_IP[16] = "172.16.6.67";
		peer->Startup(1, &sd, 1);
		peer->SetMaximumIncomingConnections(0);
		peer->Connect(SERVER_IP, SET_GPRO_SERVER_PORT, 0, 0);
	}

	cRakNetClient::~cRakNetClient()
	{
		peer->CloseConnection(server, true, 0, IMMEDIATE_PRIORITY);
		peer->Shutdown(0, 0, IMMEDIATE_PRIORITY);
	}

	void cRakNetClient::SendRSDPosition(RenderSceneData& rsd)
	{
		RakNet::BitStream bitstream_w;
		WriteTimestamp(bitstream_w);
		bitstream_w.Write((RakNet::MessageID)ID_GPRO_COMMON_SEND_POSITION);

		rsd.Write(bitstream_w, rsd);

		peer->Send(&bitstream_w, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, server, false);
	}

	RenderSceneData& cRakNetClient::GetRSD(int index)
	{
		return rsdObjects[index];
	}

	void cRakNetClient::PhysicsUpdate(double dt)
	{
		for (int i = 0; i < 128; ++i)
		{
			updateVelocity(rsdObjects[i].velocity, rsdObjects[i].acceleration, dt);
			updatePosition(rsdObjects[i].position, rsdObjects[i].velocity, dt);
		}
	}

	bool cRakNetClient::ProcessMessage(RakNet::BitStream& bitstream, RakNet::SystemAddress const sender, RakNet::Time const dtSendToReceive, RakNet::MessageID const msgID)
	{
		if (cRakNetManager::ProcessMessage(bitstream, sender, dtSendToReceive, msgID))
			return true;

		// client-specific messages
		switch (msgID)
		{
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			//printf("Another client has disconnected.\n");
			break;
		case ID_REMOTE_CONNECTION_LOST:
			//printf("Another client has lost the connection.\n");
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
			//printf("Another client has connected.\n");
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			//printf("The server is full.\n");
			return true;
		case ID_DISCONNECTION_NOTIFICATION:
			//printf("We have been disconnected.\n");
			return true;
		case ID_CONNECTION_LOST:
			//printf("Connection lost.\n");
			return true;

		case ID_CONNECTION_REQUEST_ACCEPTED:
		{
			// set server info
			server = sender;

			// client connects to server, send greeting
			RakNet::BitStream bitstream_w;
			//WriteTest(bitstream_w, "Hello server from client");
			WriteTimestamp(bitstream_w);
			bitstream_w.Write((RakNet::MessageID)ID_GPRO_COMMON_NEW_PLAYER);
			peer->Send(&bitstream_w, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, false);
		}	return true;

		case ID_GPRO_COMMON_CLIENT_ID:
		{
			//Receive id
			unsigned short id;
			bitstream.Read(id);

			clientID = id;

			for (int i = 1; i <= id; ++i)
			{
				rsdObjects[i].ownerID = i;
			}

		} return true;

		case ID_GPRO_COMMON_OTHER_CLIENT_ID:
		{
			//Receive id
			unsigned short id;
			bitstream.Read(id);

			rsdObjects[id].ownerID = id;
			
		} return true;

		case ID_GPRO_COMMON_INITIAL_PARAMETERS:
		{
			for (int i = 0; i < 128; ++i)
			{
				if (i != clientID)
				{
					RenderSceneData dat;
					RenderSceneData::Read(bitstream, dat);

					RenderSceneData::Copy(rsdObjects[i], dat);
				}
				else
				{
					rsdObjects[clientID].position[0] = 0.0f;
					rsdObjects[clientID].position[1] = 0.0f;
					rsdObjects[clientID].position[2] = 20.0f;

					rsdObjects[clientID].velocity[0] = 0.0f;
					rsdObjects[clientID].velocity[1] = 0.0f;
					rsdObjects[clientID].velocity[2] = 0.0f;

					rsdObjects[clientID].acceleration[0] = 0.0f;
					rsdObjects[clientID].acceleration[1] = 0.0f;
					rsdObjects[clientID].acceleration[2] = -9.81f;

					RakNet::BitStream bitstream_w;
					WriteTimestamp(bitstream_w);
					bitstream_w.Write((RakNet::MessageID)ID_GPRO_COMMON_INITIAL_CLIENT_PARAMETERS);
					RenderSceneData::Write(bitstream_w, rsdObjects[clientID]);
					peer->Send(&bitstream_w, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, false);
				}
			}

			connected = true;

		} return true;

		case ID_GPRO_COMMON_INITIAL_CLIENT_PARAMETERS:
		{
			RenderSceneData dat;
			RenderSceneData::Read(bitstream, dat);

			RenderSceneData::Copy(rsdObjects[dat.ownerID], dat);

		} return true;

		case ID_GPRO_COMMON_SEND_OBJECT_UPDATES:
		{
			float d = 0.0f;
			for (int i = 0; i < 128; ++i)
			{
				if (i != rsdObjects[clientID].ownerID)
				{
					RenderSceneData dat;
					RenderSceneData::Read(bitstream, dat);

					if (i == 5)
						d = dat.position[2];

					RenderSceneData temp = rsdObjects[i];

					//Dead reckoning
					if (abs(dat.position[2] - temp.position[2]) < 2)
					{
						temp.position[0] = temp.position[0] + (dat.position[0] - temp.position[0]) * dtSendToReceive;
						temp.position[1] = temp.position[1] + (dat.position[1] - temp.position[1]) * dtSendToReceive;
						temp.position[2] = temp.position[2] + (dat.position[2] - temp.position[2]) * dtSendToReceive;

						temp.velocity[0] = dat.velocity[0];
						temp.velocity[1] = dat.velocity[1];
						temp.velocity[2] = dat.velocity[2];

						temp.acceleration[0] = dat.acceleration[0];
						temp.acceleration[1] = dat.acceleration[1];
						temp.acceleration[2] = dat.acceleration[2];

						//Update our state
						RenderSceneData::Copy(rsdObjects[i], temp);
					}
					else
					{
						//Update our state
						RenderSceneData::Copy(rsdObjects[i], dat);
					}
				}
			}

			printf("%f %f \n", rsdObjects[5].position[2], d);
		} return true;

			// index receipt
		case ID_GPRO_MESSAGE_COMMON_BEGIN:
		{
			// read our index relative to server
			if (sender == server && index < 0)
				bitstream.Read(index);
		}	return true;

			// test message
		case ID_GPRO_MESSAGE_COMMON_END:
		{
			// client receives greeting, just print it
			ReadTest(bitstream);
		}

		}
		return false;
	}
}