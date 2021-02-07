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

	main-server.c/.cpp
	Main source for console server application.
*/

#include "gpro-net/gpro-net.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "RakNet/RakPeerInterface.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/GetTime.h"

#include "gpro-net/gpro-net.h"

#define MAX_CLIENTS 10
#define SERVER_PORT 7777




// Handle remote input->recieve/send
void handleRemoteInput(RakNet::RakPeerInterface* peer)
{
	RakNet::Packet* packet;

	for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
	{
		switch (packet->data[0])
		{
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			printf("Another client has disconnected.\n");
			break;
		case ID_REMOTE_CONNECTION_LOST:
			printf("Another client has lost connection.\n");
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
			printf("Another client has connected.\n");
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
		{
			printf("Our connection request has been accepted.\n");

			RakNet::BitStream bsOut;
			bsOut.Write((RakNet::MessageID)ID_GAME_MESSAGE_1);
			bsOut.Write("Hello world");
			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
		}
		break;
		case ID_NEW_INCOMING_CONNECTION:
			printf("A connection is incoming.\n");
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			printf("The server is full.\n");
		case ID_DISCONNECTION_NOTIFICATION:
			printf("A client has disconnected.\n");
			break;
		case ID_CONNECTION_LOST:
			printf("A client has lost connection.\n");
			break;
		case ID_GAME_MESSAGE_1:
		{
			RakNet::RakString rs;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			bsIn.Read(rs);
			printf("%s\n", rs.C_String());



			RakNet::Time timeStamp = RakNet::GetTime();
			RakNet::BitStream bsOut;

			bsOut.Write((RakNet::MessageID)ID_GAME_MESSAGE_2);
			bsOut.Write("Yay.");

			bsOut.Write(timeStamp);

			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			printf("Sending message from client at time %" PRINTF_64_BIT_MODIFIER "u\n", timeStamp);
		}
		break;
		case ID_PUBLIC_CLIENT_SERVER:
		{

		}
		break;

		default:
			printf("Message with identifier %i has arrived.\n", packet->data[0]);
			break;
		}
	}
}

// Handle local input->send
void handleLocalInput(RakNet::RakPeerInterface*)
{

}

// Update

// Render


int main(int const argc, char const* const argv[])
{


	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();

	peer->SetOccasionalPing(true);

	RakNet::SocketDescriptor sd(SERVER_PORT, 0);
	peer->Startup(MAX_CLIENTS, &sd, 1);

	printf("Server starting.\n");
	peer->SetMaximumIncomingConnections(MAX_CLIENTS);


	while (true)
	{
		handleRemoteInput(peer);
		handleLocalInput(peer);
	}


	RakNet::RakPeerInterface::DestroyInstance(peer);

	return EXIT_SUCCESS;
}

