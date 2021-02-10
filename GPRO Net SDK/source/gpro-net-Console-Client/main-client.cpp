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

	main-client.c/.cpp
	Main source for console client application.
*/

#include "gpro-net/gpro-net.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/GetTime.h"

#define SERVER_PORT 7777
#define SERVER_IP "172.16.2.67"

const char REQUEST_USER_LIST[7] = "!users";

enum GameMessages
{
	ID_PUBLIC_CLIENT_SERVER = ID_USER_PACKET_ENUM + 1,
	ID_PUBLIC_SERVER_CLIENT,
	ID_CLIENT_INFO
};

int main(int const argc, char const* const argv[])
{
	char str[512];
	char userName[10];
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* packet;

	peer->SetOccasionalPing(true);

	RakNet::SocketDescriptor sd;
	peer->Startup(1, &sd, 1);

	printf("Client starting.\n");
	peer->Connect(SERVER_IP, SERVER_PORT, 0, 0);


	//Get client username
	char c = 0;

	printf("Enter Username (10 Characters): ");
	fgets(userName, 10, stdin);
	fflush(stdin);

	for (int i = 0; i < strlen(userName); i++) //Remove newline character from name (can't think of abetter solution at the moment)
	{
		if (userName[i] == '\n')
			userName[i] = '\0';
	}


	while (true)
	{
		for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			switch (packet->data[0])
			{
			case ID_CONNECTION_REQUEST_ACCEPTED:
			{
				printf("Our connection request has been accepted.\n");

				RakNet::RakString rs = userName;
				RakNet::Time time = RakNet::GetTime();
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_CLIENT_INFO);
				bsOut.Write(rs);
				bsOut.Write(time);

				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			}
			break;
			case ID_NEW_INCOMING_CONNECTION:
				printf("A connection is incoming.\n");
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				printf("The server is full.\n");
			case ID_DISCONNECTION_NOTIFICATION:
				printf("We have been disconnected.\n");
				break;
			case ID_CONNECTION_LOST:
				printf("We have lost connection.\n");
				break;
			case ID_PUBLIC_CLIENT_SERVER:
			{
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());
			}
			break;
			case ID_PUBLIC_SERVER_CLIENT:
			{
				RakNet::RakString rs;
				RakNet::Time time;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				bsIn.Read(time);
				printf("%s\n", rs.C_String());
			}
			break;

			default:
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}
		}

		//Get message from user
		printf("%s: ", userName);
		fgets(str, 512, stdin);

		//Remove newline
		for (int i = 0; i < strlen(str); i++) 
		{
			if (str[i] == '\n')
				str[i] = '\0';
		}

		//Check for client commands

		if (str[0] == '@') 
		{
			//Send message to specific user
		}
		else if (str == REQUEST_USER_LIST) 
		{
			//Request user list from server
		}
		else
		{
			//Send message to server
			RakNet::RakString rs = str;
			RakNet::Time time = RakNet::GetTime();
			RakNet::BitStream bsOut;
			bsOut.Write((RakNet::MessageID)ID_PUBLIC_CLIENT_SERVER);
			bsOut.Write(rs);
			bsOut.Write(time);

			packet = peer->AllocatePacket(sizeof(rs));
			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(SERVER_IP, SERVER_PORT), false);
			peer->DeallocatePacket(packet);
		}
	}


	RakNet::RakPeerInterface::DestroyInstance(peer);

	return EXIT_SUCCESS;
}
