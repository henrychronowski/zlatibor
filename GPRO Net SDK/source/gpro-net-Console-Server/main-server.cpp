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
#include <time.h>


#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/GetTime.h"

#define MAX_CLIENTS 10
#define SERVER_PORT 7777

enum GameMessages
{
	ID_PUBLIC_CLIENT_SERVER = ID_USER_PACKET_ENUM + 1,
	ID_PUBLIC_SERVER_CLIENT
};

// Local input

// Remote input

// Update

// Render

// Log a message
int logMessage(const char* message, const char* directory = "C:\\Users\\Public\\", const char* extension = ".log")
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	char fileName[128];
	snprintf(fileName, sizeof(fileName), "%s%d-%02d-%02d%s", directory, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, extension);

	FILE* file = fopen(fileName, "a+");
	if (file == NULL)
	{
		fprintf(stderr, "Unable to open log file %s in create and append mode\n", fileName);
		return 1;
	}

	
	fprintf(file, "%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	fprintf(file, message);

	fclose(file);

	return 0;
}


int main(int const argc, char const* const argv[])
{
	logMessage("Starting server\n");

	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* packet;

	peer->SetOccasionalPing(true);

	RakNet::SocketDescriptor sd(SERVER_PORT, 0);
	peer->Startup(MAX_CLIENTS, &sd, 1);

	printf("Server starting.\n");
	peer->SetMaximumIncomingConnections(MAX_CLIENTS);


	while (true)
	{
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
				bsOut.Write((RakNet::MessageID)ID_PUBLIC_CLIENT_SERVER);
				bsOut.Write("Hello world");
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			}
			break;
			case ID_NEW_INCOMING_CONNECTION:
			{
				char buf[256];
				printf("A connection is incoming.\n");
				snprintf(buf, sizeof buf, "%s%s", packet->systemAddress.ToString(), " is connecting");
				logMessage(buf);
			}
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				printf("The server is full.\n");
			case ID_DISCONNECTION_NOTIFICATION:
			{
				printf("A client has disconnected.\n");
				char buf[256];
				snprintf(buf, sizeof buf, "%s%s", packet->systemAddress.ToString(), " has disconnected");
				logMessage(buf);
			}
				break;
			case ID_CONNECTION_LOST:
			{
				printf("A client has lost connection.\n");
				char buf[256];
				snprintf(buf, sizeof buf, "%s%s", packet->systemAddress.ToString(), " has lost connection");
				logMessage(buf);
			}
				break;
			case ID_PUBLIC_CLIENT_SERVER:
			{
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());

				RakNet::Time timeStamp = RakNet::GetTime();
				RakNet::BitStream bsOut;

				bsOut.Write((RakNet::MessageID)ID_PUBLIC_SERVER_CLIENT);
				bsOut.Write(rs);

				bsOut.Write(timeStamp);

				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);
			}
			break;

			default:
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}
		}
	}


	RakNet::RakPeerInterface::DestroyInstance(peer);
	logMessage("Server shutting down\n");
	return EXIT_SUCCESS;
}

