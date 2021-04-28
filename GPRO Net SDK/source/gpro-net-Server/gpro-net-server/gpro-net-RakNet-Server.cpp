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
#include "cereal/cereal.hpp"
#include "cereal/archives/portable_binary.hpp"

typedef cereal::PortableBinaryOutputArchive OutArchive;
typedef cereal::PortableBinaryInputArchive InArchive;

namespace gproNet
{
	cRakNetServer::cRakNetServer()
	{
		RakNet::SocketDescriptor sd(SET_GPRO_SERVER_PORT, 0);
		unsigned short MAX_CLIENTS = 10;

		peer->Startup(MAX_CLIENTS, &sd, 1);
		peer->SetMaximumIncomingConnections(MAX_CLIENTS);
	}

	cRakNetServer::~cRakNetServer()
	{
		peer->Shutdown(0);
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
			WriteTest(bitstream_w, "Hello client from server");
			peer->Send(&bitstream_w, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, false);

			RakNet::BitStream trigger;
			WriteTimestamp(trigger);
			trigger.Write((RakNet::MessageID)ID_GPRO_SEND_UNIFORM);
			peer->Send(&trigger, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, false);
		}	return true;

		case ID_GPRO_COMMON_SEND_POSITION:
		{
			RakNet::RakString rs;
			std::stringstream ss;
			RenderSceneData dat = RenderSceneData();

			bitstream.Read(rs);
			ss << rs;

			{
				InArchive iarchive(ss);
				iarchive(cereal::binary_data(dat, sizeof(float) * MAX_OBJECTS * MAX_COMPONENTS));
			}

			for (int i = 0; i < 128; ++i)
			{
				printf("%f %f %f\n", dat.objectPositions[i][0], dat.objectPositions[i][1], dat.objectPositions[i][2]);
			}


		}

		}
		return false;
	}
}