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

	main-server.cpp
	Main source for console server application.
*/

#include "gpro-net/gpro-net-server/gpro-net-RakNet-Server.hpp"
#include "gpro-net/gpro-net/gpro-net-util/Timer.h"


int main(int const argc, char const* const argv[])
{
	const double FRAME_TIME = 2.0875;// 0.0020875;	// 480 fps in seconds
	gproNet::cRakNetServer server;
	Timer timer;

	timer.Start();
	while (1)
	{
		server.MessageLoop();
		if (timer.Lap() >= FRAME_TIME)
		{
			server.PhysicsUpdate(0.001 * timer.Stop());
			timer.Start();
		}
	}

	printf("\n\n");
	system("pause");
}
