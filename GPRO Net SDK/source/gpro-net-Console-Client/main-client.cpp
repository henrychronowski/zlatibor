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
	Extended by Ethan Heil & Henry Chronowski
	Based on RakNet tutorial http://www.jenkinssoftware.com/raknet/manual/tutorial.html

	main-client.c/.cpp
	Main source for console client application.
*/

#ifdef __cplusplus
extern "C" {
#include "gpro-net/gpro-net.h"
}
#endif // __cplusplus


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <vector>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/GetTime.h"

#define SERVER_PORT 7777
#define SERVER_IP "172.16.2.59"

const char REQUEST_USER_LIST[8] = "!users";

std::vector<Card> clientHand;


//Removes newline character from name (can't think of abetter solution at the moment)
void removeNewline(char* str) 
{
	for (int i = 0; i < strlen(str); i++)
	{
		if (str[i] == '\n')
			str[i] = '\0';
	}
}

void printCurrentHand(std::vector<Card> hand) 
{
	for (int i = 0; i < hand.size(); i++)
	{
		printCard(clientHand.at(i));
	}
}

void checkScore()
{
	CardData total = 0;
	bool clear = false;
	bool aceHigh = false;

	for (int i = 0; i < clientHand.size(); i++)
	{
		if (clientHand.at(i).value > 10 && clientHand.at(i).value < 14) 
		{
			total += 10;
		}
		else if (clientHand.at(i).value == 14)
		{
			aceHigh = true;
			total += 11;
		}
		else 
		{
			total += clientHand.at(i).value;
		}
	}

	if (total > BLACKJACK)
	{
		//Bust
		if (aceHigh)
		{
			for (int i = 0; i < clientHand.size(); i++)
			{
				if (clientHand.at(i).value == 14)
					clientHand.at(i).value = 1;
			}
			checkScore();
			return;
		}

		clear = true;
		gpro_consoleSetColor(gpro_consoleColor_red, gpro_consoleColor_black);
		printCurrentHand(clientHand);
		gpro_consoleResetColor();
		printf("\nBust! :(\n");
	}
	else if (total == BLACKJACK)
	{
		//Black Jack
		clear = true;
		gpro_consoleSetColor(gpro_consoleColor_green, gpro_consoleColor_black);
		printCurrentHand(clientHand);
		gpro_consoleResetColor();
		printf("\nBlackjack! :)\n");
	}
	else
	{
		//end turn
	}

	if(clear)
		clientHand.clear();
}

void updateLocalGameState(const char* username)
{
	gpro_consoleSetColor(gpro_consoleColor_cyan, gpro_consoleColor_black);
	//print current hand
	printf("%s's Hand: ", username);
	gpro_consoleResetColor();
	gpro_consoleSetColor(gpro_consoleColor_magenta, gpro_consoleColor_black);
	printCurrentHand(clientHand);
	gpro_consoleResetColor();
	printf("\n");

	//Ask draw or hold
	char c;
	printf("Draw (D) or Hold (H)?: ");
	c = std::getchar();

	if (c == 'D' || c == 'd')
	{
		//Draw
		clientHand.push_back(drawCard());
	}
	else if (c == 'H' || c == 'h')
	{
		//hold
	}

	//check score
	checkScore();
}

//CLIENT
int main(int const argc, char const* const argv[])
{
	char str[512];
	char userName[11];

	//Initialize RakNet peer
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* packet;

	peer->SetOccasionalPing(true);

	RakNet::SocketDescriptor sd;
	peer->Startup(1, &sd, 1);

	printf("Client starting.\n");
	peer->Connect(SERVER_IP, SERVER_PORT, 0, 0);


	//Get client username
	char c = 0;

	gpro_consoleSetColor(gpro_consoleColor_cyan, gpro_consoleColor_black);
	printf("Enter Username (10 Characters): ");
	gpro_consoleResetColor();
	gpro_consoleSetColor(gpro_consoleColor_yellow, gpro_consoleColor_black);
	fgets(userName, 10, stdin);
	fflush(stdin);

	removeNewline(userName);

	//Initial game hand
	clientHand.push_back(drawCard());
	clientHand.push_back(drawCard());
	
	//Main loop, runs until client is shut down
	while (true)
	{
		//Networking Loop
		for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			switch (packet->data[0])
			{
			case ID_CONNECTION_REQUEST_ACCEPTED: //Client successfully connects to the server
			{
				printf("Our connection request has been accepted.\n");

				//Create message to send username to the server
				RakNet::RakString rs = userName;
				RakNet::Time time = RakNet::GetTime();
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_CLIENT_INFO);
				bsOut.Write(rs);
				bsOut.Write(time);

				//Send usernam to server to be stored
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			}
			break;
			case ID_DISCONNECTION_NOTIFICATION:
				printf("We have been disconnected.\n");
				break;
			case ID_CONNECTION_LOST:
				printf("We have lost connection.\n");
				break;
			case ID_PUBLIC_CLIENT_SERVER: //Public message from a client to all clients
			{
				//Read in message data from client
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());
			}
			break;
			case ID_PUBLIC_SERVER_CLIENT: //Public message from server
			{
				//Read in message data from server
				RakNet::RakString rs;
				RakNet::Time time;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				bsIn.Read(time);
				printf("%s\n", rs.C_String());
			}
			break;
			case ID_CLIENT_REQUEST_USERS: //Client requesting a list of users from the server
			{
				//Read in list of users from the server
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());
			}
			break;
			case ID_PRIVATE_SERVER_CLIENT: //Recieve private message from server
			{
				// Read in intended recipient and message
				RakNet::RakString rs;
				RakNet::Time time;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				bsIn.Read(time);
				printf("%s\n", rs.C_String());
			}
			break;
			default: //Unknown message ID
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}
		}

		updateLocalGameState(userName);

		//Get message from user
		//printf("%s: ", userName);
		fgets(str, 512, stdin);
		removeNewline(str);
		
		//Check for client commands
		if (strstr(str, "@")) //Send private message to specific client. Assumes that user will only enter '@' to do this
		{
			//Get username from the message string
			char user[11];
			printf("User: ");
			fgets(user, 10, stdin);
			removeNewline(user);

			//Get message for chosen user
			char msg[512];
			printf("Message: ");
			fgets(msg, 512, stdin);
			removeNewline(msg);

			//Send message to server
			RakNet::RakString userRs = user;
			RakNet::RakString msgRs = msg;
			RakNet::Time time = RakNet::GetTime();
			RakNet::BitStream bsOut;
			bsOut.Write((RakNet::MessageID)ID_PRIVATE_CLIENT_SERVER);
			bsOut.Write(userRs);
			bsOut.Write(msgRs);
			bsOut.Write(time);

			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(SERVER_IP, SERVER_PORT), false); //Send to specific user
		}
		else if (strstr(str, REQUEST_USER_LIST)) //Request user list from server
		{
			//Send message to server
			RakNet::Time time = RakNet::GetTime();
			RakNet::BitStream bsOut;
			bsOut.Write((RakNet::MessageID)ID_CLIENT_REQUEST_USERS);
			bsOut.Write(time);

			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(SERVER_IP, SERVER_PORT), false); //Send to server
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

			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(SERVER_IP, SERVER_PORT), false); //Send to server
		}
	}


	RakNet::RakPeerInterface::DestroyInstance(peer);

	return EXIT_SUCCESS;
}
