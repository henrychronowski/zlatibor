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
#include <signal.h>


#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/GetTime.h"





#include <vector>


#define MAX_CLIENTS 10
#define SERVER_PORT 7777

enum GameMessages
{
	ID_PUBLIC_CLIENT_SERVER = ID_USER_PACKET_ENUM + 1,
	ID_PUBLIC_SERVER_CLIENT,
	ID_CLIENT_INFO,
	ID_CLIENT_REQUEST_USERS,
	ID_PRIVATE_CLIENT_SERVER,
	ID_PRIVATE_SERVER_CLIENT
};

struct User
{
	RakNet::SystemAddress mAddress;
	char mUserName[11];

	User(RakNet::SystemAddress address, const char* username) : mAddress(address)
	{
		snprintf(mUserName, sizeof mUserName, "%s", username);
	}
};


// Signal handler function to handle ctrl+c for program exit
bool sig_caught = false;

void signal_handler(int sig)
{
	if (sig == SIGINT)
	{
		sig_caught = true;
	}
}

// Local input

// Remote input

// Update

// Render

// Log a message
int logMessage(const char* message, const char* type = "notice\t", const char* directory = "C:\\Users\\Public\\", const char* extension = ".log")
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

	
	fprintf(file, "%d-%02d-%02d %02d:%02d:%02d\t%s:\t", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, type);
	fprintf(file, message);

	fclose(file);

	return 0;
}

//RakNet::SystemAddress getAddress(User* users, char* name, int count = MAX_CLIENTS)
//{
//	RakNet::SystemAddress result = NULL;
//	for (int i = 0; i < count; i++)
//	{
//		if (users[i].mUserName == name)
//			result = users[i].mAddress;
//	}
//
//	return result;
//}

// returns -1 on not found
int getClientIndex(std::vector<User>& users, RakNet::SystemAddress address)
{
	int result = -1;
	for (int i = 0; i < users.size(); i++)
	{
		if (users.at(i).mAddress == address)
			result = i;
	}

	return result;
}

int getClientIndex(std::vector<User>& users, char name[11])
{
	int result = -1;
	for (int i = 0; i < users.size(); i++)
	{
		if (!strcmp(users.at(i).mUserName, name))
			result = i;
	}

	return result;
}

int main(int const argc, char const* const argv[])
{

	// Checking to see if signal function registration succeeded
	if (signal(SIGINT, signal_handler) == SIG_ERR)
	{
		fprintf(stderr, "Signal function registration failed, unable to proceed\n");
		return EXIT_FAILURE;
	}

	logMessage("Starting server\n");

	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* packet;

	peer->SetOccasionalPing(true);

	RakNet::SocketDescriptor sd(SERVER_PORT, 0);
	peer->Startup(MAX_CLIENTS, &sd, 1);

	printf("Server starting.\n");
	peer->SetMaximumIncomingConnections(MAX_CLIENTS);

	////user users[MAX_CLIENTS];
	//user* users;
	//users = new user[MAX_CLIENTS];
	//int currentUsers = 0;

	std::vector<User> users;


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
			}
			break;
			case ID_CLIENT_INFO:
			{
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				char bufName[11];
				snprintf(bufName, sizeof bufName, "%s", rs.C_String());
				users.push_back(User(packet->systemAddress, bufName));

				char buf[256];
				snprintf(buf, sizeof buf, "%s", bufName);
				snprintf(buf, sizeof buf, "%s%s", buf, " has connected\n");
				rs = buf;
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_PUBLIC_SERVER_CLIENT);
				bsOut.Write(rs);

				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true); // To all but sender

				//delete[] bufName;
			}
				break;
			case ID_NEW_INCOMING_CONNECTION:
			{
				char buf[256];
				snprintf(buf, sizeof buf, "%s%s\n", packet->systemAddress.ToString(), " is connecting");
				printf(buf);
				logMessage(buf);

				//RakNet::RakString rs = buf;
				//RakNet::BitStream bsOut;
				//bsOut.Write((RakNet::MessageID)ID_PUBLIC_SERVER_CLIENT);
				//bsOut.Write(rs);

				//peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true); // To all but sender
			}
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
			{
				printf("The server is full.\n");
			}
				break;
			case ID_DISCONNECTION_NOTIFICATION:
			{
				printf("A client has disconnected.\n");
				char buf[256];
				snprintf(buf, sizeof buf, "%s%s\n", packet->systemAddress.ToString(), " has disconnected");
				logMessage(buf);

				RakNet::RakString rs = buf;
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_PUBLIC_SERVER_CLIENT);
				bsOut.Write(rs);

				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true); // To all but sender
			}
				break;
			case ID_CONNECTION_LOST:
			{
				printf("%s has lost connection.\n", packet->systemAddress.ToString());

				char buf[256];
				snprintf(buf, sizeof buf, "%s%s\n", packet->systemAddress.ToString(), " has lost connection");
				logMessage(buf);

				RakNet::RakString rs = buf;
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_PUBLIC_SERVER_CLIENT);
				bsOut.Write(rs);
				
				int result = getClientIndex(users, packet->systemAddress);
				if(result != -1)
					users.erase(users.begin() + result);

				for (auto food : users)
				{
					printf("%s\n", food.mUserName);
				}
				printf("\n\n");

				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true); // To all but sender
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

				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true); // To all but sender

				char buf[256];
				snprintf(buf, sizeof buf, "Message type %i %u:\"%s\" from client %s\n", ID_PUBLIC_SERVER_CLIENT, (UINT)timeStamp, rs.C_String(), packet->systemAddress.ToString());
				logMessage(buf, "message");
			}
			break;
			case ID_CLIENT_REQUEST_USERS:
			{
				RakNet::RakString rs;
				RakNet::BitStream bsOut;
				char buf[102];
				bsOut.Write((RakNet::MessageID)ID_CLIENT_REQUEST_USERS);
				snprintf(buf, sizeof buf, "%s", users.at(0).mUserName);
				/*for (int i = 1; i < currentUsers; i++)
				{
					snprintf(buf, sizeof buf, "%s|%s", buf, users[i].mUserName);
				}*/
				//for (int i = 0; i < users.size(); i++)
				//{
				//	//rs = 
				//	bsOut.Write(users.at(i).mUserName);
				//}
				for (int i = 1; i < users.size(); i++)
				{
					snprintf(buf, sizeof buf, "%s|%s", buf, users.at(i).mUserName);
				}

				//bsOut.Write("yellow");
				bsOut.Write(buf);
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false); // To only sender
			}
				break;
			case ID_PRIVATE_CLIENT_SERVER:
			{
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				char name[11];
				RakNet::RakString rs;
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				snprintf(name, sizeof name, "%s", rs.C_String());
				printf("%s\n", name);
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());

				RakNet::BitStream bsOut;

				int result = getClientIndex(users, name);
				if (result == -1)
				{
					rs = "Error: Client not found\n";
					bsOut.Write((RakNet::MessageID)ID_PUBLIC_SERVER_CLIENT);
					bsOut.Write(rs);

					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false); // To only sender
				}
				else
				{
					RakNet::Time timeStamp = RakNet::GetTime();

					char buf[256];
					snprintf(buf, sizeof buf, "%s whispers to you: %s\n", users.at(getClientIndex(users, packet->systemAddress)).mUserName, rs.C_String());
					bsOut.Write((RakNet::MessageID)ID_PRIVATE_SERVER_CLIENT);
					rs = buf;
					bsOut.Write(rs);
					bsOut.Write(timeStamp);

					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, users.at(result).mAddress, false); // To only address
				}

			}
				break;

			default:
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				char buf[256];
				snprintf(buf, sizeof buf, "Message with identifier %i has arrived from %s\n", packet->data[0], packet->systemAddress.ToString());
				logMessage(buf);
				break;
			}
		}


		// Check if exit signal has been caught and if so shut down the server
		if (sig_caught)
		{
			printf("Shutting down server\n");
			break;
		}
	}


	RakNet::RakPeerInterface::DestroyInstance(peer);
	logMessage("Server shutting down\n");
	//delete[] users;
	users.clear();
	system("pause");
	return EXIT_SUCCESS;
}

