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

	gpro-net.h
	Main include for framework.
*/

#ifndef _GPRO_NET_H_
#define _GPRO_NET_H_

//#ifdef __cplusplus
//extern "C" {
//#endif // _cplusplus


#include "RakNet/MessageIdentifiers.h"
#include "gpro-net-common/gpro-net-console.h"
#include "gpro-net-common/gpro-net-gamestate.h"

	//Enum for all common client-server messages
	enum GameMessages
	{
		ID_PUBLIC_CLIENT_SERVER = ID_USER_PACKET_ENUM + 1,
		ID_PUBLIC_SERVER_CLIENT,
		ID_CLIENT_INFO,
		ID_CLIENT_REQUEST_USERS,
		ID_PRIVATE_CLIENT_SERVER,
		ID_PRIVATE_SERVER_CLIENT
	};

	const int NUM_DECK_CARDS = 52;
	const int NUM_SUITE_CARDS = 13;

	typedef unsigned short CardData;


	typedef struct _Card
	{
		CardData suite;
		CardData value;
	}Card;

	Card drawCard();


#endif	// !_GPRO_NET_H_