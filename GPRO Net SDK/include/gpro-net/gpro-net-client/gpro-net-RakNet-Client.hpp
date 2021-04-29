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

	gpro-net-RakNet-Client.hpp
	Header for RakNet client management.
*/

#ifndef _GPRO_NET_RAKNET_CLIENT_HPP_
#define _GPRO_NET_RAKNET_CLIENT_HPP_
#ifdef __cplusplus


#include "gpro-net/gpro-net/gpro-net-RakNet.hpp"


namespace gproNet
{
	// eMessageClient
	//	Enumeration of custom client message identifiers.
	enum eMessageClient
	{
		ID_GPRO_MESSAGE_CLIENT_BEGIN = ID_GPRO_MESSAGE_COMMON_END,

		ID_GPRO_RENDER_RESULT,
		ID_GPRO_MESSAGE_CLIENT_END
	};

	// cRakNetClient
	//	RakNet peer management for server.
	class cRakNetClient : public cRakNetManager
	{
		// protected data
	protected:
		// server
		//	System address of server to whom we are connected.
		RakNet::SystemAddress server;

		// index
		//	Index in system list of server to whom we are connected.
		int index;

		//clientID
		// The ID of this client provided by the server
		short clientID = -1;

		gproNet::RenderSceneData rsd;

		// public methods
	public:
		// cRakNetClient
		//	Default constructor.
		cRakNetClient();

		// ~cRakNetClient
		//	Destructor.
		virtual ~cRakNetClient();

		void SendRSDPosition(RenderSceneData& rsd);

		RenderSceneData& getRSD();

		// protected methods
	protected:
		// ProcessMessage
		//	Unpack and process packet message.
		//		param bitstream: packet data in bitstream
		//		param dtSendToReceive: locally-adjusted time difference from sender to receiver
		//		param msgID: message identifier
		//		return: was message processed
		virtual bool ProcessMessage(RakNet::BitStream& bitstream, RakNet::SystemAddress const sender, RakNet::Time const dtSendToReceive, RakNet::MessageID const msgID);
	};

}


#endif	// __cplusplus
#endif	// !_GPRO_NET_RAKNET_CLIENT_HPP_