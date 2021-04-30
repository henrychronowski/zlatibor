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

	gpro-net-RakNet.hpp
	Header for RakNet common management.
*/

#ifndef _GPRO_NET_RAKNET_HPP_
#define _GPRO_NET_RAKNET_HPP_
#ifdef __cplusplus


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/GetTime.h"

#define MAX_OBJECTS = 128
#define MAX_COMPONENTS = 3


namespace gproNet
{
	// eSettings
	//	Enumeration of common settings.
	enum eSettings
	{
		SET_GPRO_SERVER_PORT = 7777,
	};


	// eMessageCommon
	//	Enumeration of custom common message identifiers.
	enum eMessageCommon
	{
		ID_GPRO_MESSAGE_COMMON_BEGIN = ID_USER_PACKET_ENUM,

		ID_GPRO_COMMON_CLIENT_ID,
		ID_GPRO_COMMON_SEND_POSITION,
		ID_GPRO_COMMON_SEND_OBJECT_UPDATES,
		ID_GPRO_COMMON_OTHER_CLIENT_ID,

		ID_GPRO_MESSAGE_COMMON_END
	};


	struct RenderSceneData
	{
		short ownerID = 0;
		float position[3];
		float velocity[3];
		float acceleration[3];
		float force[3];

		static RakNet::BitStream& Write(RakNet::BitStream& bitstream, RenderSceneData const& obj)
		{
			bitstream.Write(obj.ownerID);
			
			bitstream.Write(obj.position[0]);
			bitstream.Write(obj.position[1]);
			bitstream.Write(obj.position[2]);

			bitstream.Write(obj.velocity[0]);
			bitstream.Write(obj.velocity[1]);
			bitstream.Write(obj.velocity[2]);

			bitstream.Write(obj.acceleration[0]);
			bitstream.Write(obj.acceleration[1]);
			bitstream.Write(obj.acceleration[2]);

			bitstream.Write(obj.force[0]);
			bitstream.Write(obj.force[1]);
			bitstream.Write(obj.force[2]);

			return bitstream;
		}

		static RakNet::BitStream& Read(RakNet::BitStream& bitstream, RenderSceneData const& obj)
		{
			bitstream.Read(obj.ownerID);
			bitstream.Read(obj.position[0]);
			bitstream.Read(obj.position[1]);
			bitstream.Read(obj.position[2]);

			bitstream.Read(obj.velocity[0]);
			bitstream.Read(obj.velocity[1]);
			bitstream.Read(obj.velocity[2]);

			bitstream.Read(obj.acceleration[0]);
			bitstream.Read(obj.acceleration[1]);
			bitstream.Read(obj.acceleration[2]);

			bitstream.Read(obj.force[0]);
			bitstream.Read(obj.force[1]);
			bitstream.Read(obj.force[2]);

			return bitstream;
		}
		
		static RenderSceneData& Copy(RenderSceneData&lhs, RenderSceneData const&rhs)
		{
			lhs.ownerID = rhs.ownerID;

			lhs.position[0] = rhs.position[0];
			lhs.position[1] = rhs.position[1];
			lhs.position[2] = rhs.position[2];

			lhs.velocity[0] = rhs.velocity[0];
			lhs.velocity[1] = rhs.velocity[1];
			lhs.velocity[2] = rhs.velocity[2];

			lhs.acceleration[0] = rhs.acceleration[0];
			lhs.acceleration[1] = rhs.acceleration[1];
			lhs.acceleration[2] = rhs.acceleration[2];

			lhs.force[0] = rhs.force[0];
			lhs.force[1] = rhs.force[1];
			lhs.force[2] = rhs.force[2];

			return lhs;
		}
	};


	// cRakNetManager
	//	Base class for RakNet peer management.
	class cRakNetManager abstract
	{
		// protected data
	protected:
		// peer
		//	Pointer to RakNet peer instance.
		RakNet::RakPeerInterface* peer;

		// protected methods
	protected:
		// cRakNetManager
		//	Default constructor.
		cRakNetManager();

		// ~cRakNetManager
		//	Destructor.
		virtual ~cRakNetManager();

		// ProcessMessage
		//	Unpack and process packet message.
		//		param bitstream: packet data in bitstream
		//		param dtSendToReceive: locally-adjusted time difference from sender to receiver
		//		param msgID: message identifier
		//		return: was message processed
		virtual bool ProcessMessage(RakNet::BitStream& bitstream, RakNet::SystemAddress const sender, RakNet::Time const dtSendToReceive, RakNet::MessageID const msgID);

		// WriteTimestamp
		//	Write timestamp ID and current time.
		//		param bitstream: packet data in bitstream
		//		return: bitstream
		RakNet::BitStream& WriteTimestamp(RakNet::BitStream& bitstream);

		// ReadTimestamp
		//	Read timestamp ID and current time.
		//		param bitstream: packet data in bitstream
		//		return: bitstream
		RakNet::BitStream& ReadTimestamp(RakNet::BitStream& bitstream, RakNet::Time& dtSendToReceive_out, RakNet::MessageID& msgID_out);

		// WriteTest
		//	Write test greeting message.
		//		param bitstream: packet data in bitstream
		//		param message: message string
		//		return: bitstream
		RakNet::BitStream& WriteTest(RakNet::BitStream& bitstream, char const message[]);

		// ReadTest
		//	Read test greeting message.
		//		param bitstream: packet data in bitstream
		//		return: bitstream
		RakNet::BitStream& ReadTest(RakNet::BitStream& bitstream);

		// public methods
	public:
		// MessageLoop
		//	Unpack and process packets.
		//		return: number of messages processed
		int MessageLoop();
	};

}


#endif	// __cplusplus
#endif	// !_GPRO_NET_RAKNET_HPP_