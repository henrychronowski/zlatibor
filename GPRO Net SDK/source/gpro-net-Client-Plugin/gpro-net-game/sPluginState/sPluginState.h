/*
	Copyright 2011-2021 Daniel S. Buckstein

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
	GPRO Net Framework
	By Daniel S. Buckstein

	sPluginState.h
	Plugin interface.
*/

#ifndef _GPRO_NET_SPLUGINSTATE_H_
#define _GPRO_NET_SPLUGINSTATE_H_


#ifdef GPRO_USING_A3
#include "../sRenderer/sRenderer.h"
#endif	// GPRO_USING_A3


#ifdef __cplusplus
//extern "C" {
//#endif	// __cplusplus

#include "gpro-net/gpro-net-client/gpro-net-RakNet-Client.hpp"


typedef struct sPluginState
{
	// client interface
	gproNet::cRakNetClient client[1];

#ifdef GPRO_USING_A3
	// renderer
	sRenderer renderer[1];

	// renderer data
	union {
		sSceneObjectComponent obj[4];
		struct {
			sSceneObjectComponent obj_root[1];
			sSceneObjectComponent obj_camera_main[1];
			sSceneObjectComponent obj_light_main[1];
			sSceneObjectComponent obj_skybox[1];
		};
	};
	sProjectorComponent proj_camera_main[1];
	sPointLightComponent lt_point_main[1];

	sSceneObjectData objdata[4];
	sProjectorData projdata[1];
	sPointLightData ltdata[1];
	sModelMatrixStack modelstack[4];
	sProjectorMatrixStack projstack[1];

	// renderer representation of client data
	sSceneObjectComponent obj_client[128];
	sSceneObjectData objdata_client[128];
	sModelMatrixStack modelstack_client[128];

	// input
	a3_MouseInput mouse[1];
	a3_KeyboardInput keyboard[1];
	a3_XboxControllerInput xcontrol[4];

	// timers
	a3_Timer timer_display[1];

	// pointer to fast trig table
	a3f32 trigTable[4096 * 4];

	//gproNet::RenderSceneData rsd;

#endif	// GPRO_USING_A3
} sPluginState;


//#ifdef __cplusplus
//}
#endif	// __cplusplus


#endif	// !_GPRO_NET_SPLUGINSTATE_H_