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
	Extended by Henry Chronowski & Ethan Heil
	Based on RakNet tutorial http://www.jenkinssoftware.com/raknet/manual/tutorial.html

	main-server.c/.cpp
	Main source for console server application.

	General consulted sources:
	http://www.jenkinssoftware.com/raknet/manual/index.html
	https://www.gnu.org/software/libc/manual/html_node/Basic-Signal-Handling.html
	https://www.gnu.org/software/libc/manual/html_node/Formatted-Output-Functions.html
*/

#ifdef __cplusplus
extern "C" {
#include "gpro-net/gpro-net.h"
}
#endif // __cplusplus


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <vector>


#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/GetTime.h"

// Ideally would take these values in as arguments and have these as defaults
#define MAX_CLIENTS 10
#define SERVER_PORT 7777


// Handles interrupt signal
// If the debugger is configured to handle SIGINT it will override this but without the debugger will function properly
// Referenced for signal handling code: https://www.gnu.org/software/libc/manual/html_node/Basic-Signal-Handling.html
bool sig_caught = false;
void signal_handler(int sig)
{
	if (sig == SIGINT)
	{
		sig_caught = true;
	}
}

// A struct to represent a user, holding a username and the address of their system
struct User
{
	RakNet::SystemAddress mAddress;
	char mUserName[11];

	User(RakNet::SystemAddress address, const char* username) : mAddress(address)
	{
		snprintf(mUserName, sizeof mUserName, "%s", username);
	}
};

// Logs a message to the given file for the purpose of a running server log
int logMessage(const char* message, const char* type = "notice", const char* directory = "C:\\Users\\Public\\", const char* extension = ".log")
{
	// Get time in human readable format
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	// Open file in the given directory with a name of the current date, appending if a log already exists for this date
	char fileName[128];
	snprintf(fileName, sizeof(fileName), "%s%d-%02d-%02d%s", directory, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, extension);
	FILE* file = fopen(fileName, "a+");
	if (file == NULL)
	{
		fprintf(stderr, "Unable to open log file %s in create and append mode\n", fileName);
		return 1;
	}

	// Print the human readable time and message type, then the message content
	fprintf(file, "%d-%02d-%02d %02d:%02d:%02d\t%s\t:\t%s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, type, message);

	fclose(file);

	return 0;
}

// Checks for the given address in the given vector, returns -1 on not found
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

// Checks for the given user name in the given vector, returns -1 on not found
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


// SERVER
int main(int const argc, char const* const argv[])
{
	// Checking to see if signal function registration succeeded
	if (signal(SIGINT, signal_handler) == SIG_ERR)
	{
		fprintf(stderr, "Signal function registration failed, unable to proceed\n");
		return EXIT_FAILURE;
	}

	logMessage("Starting server");
	printf("Server starting.\n");

	// Initialize RakNet peer and associated settings
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* packet;
	peer->SetOccasionalPing(true);
	RakNet::SocketDescriptor sd(SERVER_PORT, 0);
	peer->Startup(MAX_CLIENTS, &sd, 1);	
	peer->SetMaximumIncomingConnections(MAX_CLIENTS);

	// If this were entirely in c I would likely implement a binary search tree rather than using vector
	std::vector<User> users;

	drawCard();

	// Main loop, runs until server is shut down
	while (true)
	{
		// Process all incoming packets each loop
		for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			switch (packet->data[0])
			{
			case ID_CLIENT_INFO:	// Client response to successful connection, contains username
			{
				// Read in and store data from client
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				char bufName[11];
				snprintf(bufName, sizeof bufName, "%s", rs.C_String());
				users.push_back(User(packet->systemAddress, bufName));

				// Update other clients with the status and username of the new client
				char buf[256];
				snprintf(buf, sizeof buf, "%s", bufName);	// This makes it work
				snprintf(buf, sizeof buf, "%s%s", buf, " has connected\n");
				rs = buf;
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_PUBLIC_SERVER_CLIENT);
				bsOut.Write(rs);

				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true); // To all but sender
			}
				break;
			case ID_NEW_INCOMING_CONNECTION:
			{
				char buf[256];
				snprintf(buf, sizeof buf, "%s%s", packet->systemAddress.ToString(), " is connecting");
				printf("%s\n", buf);
				logMessage(buf);
			}
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
			{
				printf("The server is full.\n");
			}
				break;
			case ID_DISCONNECTION_NOTIFICATION:
			{
				// Log and print disconnection information
				char buf[256];
				snprintf(buf, sizeof buf, "%s%s", packet->systemAddress.ToString(), " has disconnected");
				printf("%s\n", buf);
				logMessage(buf);

				// Notify other clients of the disconnection
				RakNet::RakString rs = buf;
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_PUBLIC_SERVER_CLIENT);
				bsOut.Write(rs);

				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true); // To all but sender

				// Remove disconnected client from stored clients
				int result = getClientIndex(users, packet->systemAddress);
				if (result != -1)
					users.erase(users.begin() + result);
			}
				break;
			case ID_CONNECTION_LOST:
			{
				// Log and print connection lost information
				char buf[256];
				snprintf(buf, sizeof buf, "%s%s", packet->systemAddress.ToString(), " has lost connection");
				printf("%s\n", buf);
				logMessage(buf);

				// Notify other clients of connection lost
				RakNet::RakString rs = buf;
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_PUBLIC_SERVER_CLIENT);
				bsOut.Write(rs);
				
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true); // To all but sender

				// Remove disconnected client from stored clients
				int result = getClientIndex(users, packet->systemAddress);
				if (result != -1)
					users.erase(users.begin() + result);
			}
				break;
			case ID_PUBLIC_CLIENT_SERVER:	// Message from a client intended for all clients
			{
				// Read in message data from client
				RakNet::RakString rs;
				RakNet::Time timeStamp;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				bsIn.Read(timeStamp);
				printf("%s\n", rs.C_String());

				// Construct and send message to all other clients
				RakNet::BitStream bsOut;
				char buf[512];
				snprintf(buf, sizeof buf, "%s:%s", users.at(getClientIndex(users, packet->systemAddress)).mUserName, rs.C_String());
				rs = buf;
				bsOut.Write((RakNet::MessageID)ID_PUBLIC_SERVER_CLIENT);
				bsOut.Write(rs);
				bsOut.Write(timeStamp);

				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true); // To all but sender

				// Log message traffic
				//char buf[256];
				snprintf(buf, sizeof buf, "Message type %i %u:\"%s\" from client %s", ID_PUBLIC_SERVER_CLIENT, (UINT)timeStamp, rs.C_String(), packet->systemAddress.ToString());
				logMessage(buf, "message");
			}
			break;
			case ID_CLIENT_REQUEST_USERS:	// The client is requesting a list of connected clients from the server
			{
				// Construct list of currently connected usernames in a char buffer
				RakNet::RakString rs;
				RakNet::BitStream bsOut;
				char buf[MAX_CLIENTS * 12];	// 12 comes from max username length (10) + 2 buffer for delimiters and parity
				bsOut.Write((RakNet::MessageID)ID_CLIENT_REQUEST_USERS);

				// Constructs a buffer of all connected usernames delimited by a pipe |
				snprintf(buf, sizeof buf, "%s", users.at(0).mUserName);
				for (int i = 1; i < users.size(); i++)
				{
					snprintf(buf, sizeof buf, "%s|%s", buf, users.at(i).mUserName);
				}

				// Send constructed information to requester
				bsOut.Write(buf);
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false); // To only sender
			}
				break;
			case ID_PRIVATE_CLIENT_SERVER:	// A message from a client intended for a specific other client
			{
				// Read in intended recipient and message
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				char name[11];
				RakNet::RakString rs;
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);	// recipient username
				snprintf(name, sizeof name, "%s", rs.C_String());
				bsIn.Read(rs);	// message content

				RakNet::BitStream bsOut;

				// Check for user existence
				int result = getClientIndex(users, name);
				if (result == -1)
				{
					// If client not found return error to the source
					rs = "Error: Client not found\n";
					bsOut.Write((RakNet::MessageID)ID_PUBLIC_SERVER_CLIENT);
					bsOut.Write(rs);

					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false); // To only sender
				}
				else
				{
					// If client found package and send message to the recipient
					RakNet::Time timeStamp = RakNet::GetTime();
					char buf[512], msg[512];
					snprintf(msg, sizeof msg, rs.C_String());
					snprintf(buf, sizeof buf, "%s whispers to you: %s\n", users.at(getClientIndex(users, packet->systemAddress)).mUserName, msg);
					bsOut.Write((RakNet::MessageID)ID_PRIVATE_SERVER_CLIENT);
					rs = buf;
					bsOut.Write(rs);
					bsOut.Write(timeStamp);

					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, users.at(result).mAddress, false); // To only address

					// Log message traffic
					snprintf(buf, sizeof buf, "Message type %i %u:\"%s\" from client %s to client %s", ID_PRIVATE_SERVER_CLIENT, (UINT)timeStamp, msg, packet->systemAddress.ToString(), users.at(result).mAddress.ToString());
					logMessage(buf, "message");
				}
			}
				break;
			default:
				char buf[256];
				snprintf(buf, sizeof buf, "Message with identifier %i has arrived from %s", packet->data[0], packet->systemAddress.ToString());
				printf("%s\n", buf);
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


	// Clean up and shutdown
	RakNet::RakPeerInterface::DestroyInstance(peer);
	logMessage("Server shutting down");
	users.clear();
	return EXIT_SUCCESS;
}