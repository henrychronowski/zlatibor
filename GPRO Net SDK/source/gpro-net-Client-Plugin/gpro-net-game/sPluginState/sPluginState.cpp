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

	sPluginState.cpp
	Plugin implementation.
*/

#define GPRO_USING_A3
#include "sPluginState.h"


//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

void renderer_input_controlProjector_xcontrol(
	a3_XboxControllerInput const* xcontrol,
	sProjectorComponent const* projector, a3f64 const dt);
void renderer_input_controlProjector_trad(
	a3_MouseInput const* mouse, a3_KeyboardInput const* keyboard,
	sProjectorComponent const* projector, a3f64 const dt);
void renderer_update_defaultAnimation(a3f64 const dt, sSceneObjectComponent const* sceneObjectArray,
	a3ui32 const count, a3ui32 const axis, a3boolean const updateAnimation);
void renderer_update_bindSkybox(sSceneObjectComponent const* sceneObject_skybox,
	sProjectorComponent const* projector_active);

#ifdef __cplusplus
}
#endif	// __cplusplus

void updateRSDPosition(sPluginState* state, int index);

//-----------------------------------------------------------------------------

void plugin_load_init(sPluginState* pluginState)
{
	sSceneObjectComponent const* sceneObject = 0;
	sSceneObjectData* sceneObjectData = 0;

	sProjectorComponent const* projector = 0;
	sProjectorData* projectorData = 0;

	sPointLightComponent const* pointLight = 0;
	sPointLightData* pointLightData = 0;

	// camera's starting orientation depends on "vertical" axis
	// we want the exact same view in either case
	a3real const sceneCameraAxisPos = 15.0f;
	a3vec3 const sceneCameraStartPos = {
		+sceneCameraAxisPos,
		-sceneCameraAxisPos,
		+sceneCameraAxisPos + 5.0f,
	};
	a3vec3 const sceneCameraStartEuler = {
		+55.0f,
		+0.0f,
		+45.0f,
	};
	a3f32 const sceneObjectDistance = 8.0f;
	a3f32 const sceneObjectHeight = 2.0f;

	a3mat4 const bias = {
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f
	}, biasInv = {
		2.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 2.0f, 0.0f,
		-1.0f, -1.0f, -1.0f, 1.0f
	};

	a3ui32 i, j;

	// initialize scene object pointers
	renderer_initSceneObject(pluginState->obj_root, 0,
		pluginState->objdata, pluginState->modelstack);
	renderer_initSceneObject(pluginState->obj_camera_main, 1,
		pluginState->objdata, pluginState->modelstack);
	renderer_initSceneObject(pluginState->obj_light_main, 2,
		pluginState->objdata, pluginState->modelstack);
	renderer_initSceneObject(pluginState->obj_skybox, 3,
		pluginState->objdata, pluginState->modelstack);

	// initialize projector pointers
	renderer_initProjector(pluginState->proj_camera_main,
		pluginState->obj_camera_main->sceneHierarchyIndex, 0,
		pluginState->projdata, pluginState->projstack, pluginState->obj);

	// initialize point light pointers
	renderer_initPointLight(pluginState->lt_point_main,
		pluginState->obj_light_main->sceneHierarchyIndex, 0,
		pluginState->ltdata, pluginState->obj);

	// initialize cameras dependent on viewport
	pluginState->proj_camera_main->dataPtr->aspect = pluginState->renderer->frameAspect;
	renderer_updateProjector(pluginState->proj_camera_main);
	renderer_updateProjectorViewMats(pluginState->proj_camera_main);
	renderer_updateProjectorBiasMats(pluginState->proj_camera_main, bias, biasInv);


	// initialize object data
	sceneObject = pluginState->obj_root;
	sceneObjectData = sceneObject->dataPtr;
	renderer_resetSceneObjectData(sceneObjectData);
	renderer_resetModelMatrixStack(sceneObject->modelMatrixStackPtr);

	sceneObject = pluginState->obj_camera_main;
	sceneObjectData = sceneObject->dataPtr;
	renderer_resetSceneObjectData(sceneObjectData);
	renderer_resetModelMatrixStack(sceneObject->modelMatrixStackPtr);
	sceneObjectData->position.xyz = sceneCameraStartPos;
	sceneObjectData->euler.xyz = sceneCameraStartEuler;

	sceneObject = pluginState->obj_light_main;
	sceneObjectData = sceneObject->dataPtr;
	renderer_resetSceneObjectData(sceneObjectData);
	renderer_resetModelMatrixStack(sceneObject->modelMatrixStackPtr);
	sceneObjectData->position.xyz = sceneCameraStartPos;
	sceneObjectData->euler.xyz = sceneCameraStartEuler;

	sceneObject = pluginState->obj_skybox;
	sceneObjectData = sceneObject->dataPtr;
	renderer_resetSceneObjectData(sceneObjectData);
	renderer_resetModelMatrixStack(sceneObject->modelMatrixStackPtr);
	sceneObjectData->scale.x = a3real_oneeighty;
	sceneObjectData->scaleMode = scale_uniform;


	// set up projectors
	projector = pluginState->proj_camera_main;
	projectorData = projector->dataPtr;
	projectorData->fovy = a3real_fortyfive;
	projectorData->znear = a3real_one;
	projectorData->zfar = a3real_threesixty;
	projectorData->ctrlMoveSpeed = a3real_half;
	projectorData->ctrlRotateSpeed = a3real_two;
	projectorData->ctrlZoomSpeed = a3real_fortyfive;
	projectorData->perspective = a3true;
	renderer_updateProjector(projector);


	// set up lights
	pointLight = pluginState->lt_point_main;
	pointLightData = pointLight->dataPtr;
	renderer_resetPointLightData(pointLightData);
	pointLightData->worldPos = pointLight->sceneObjectPtr->dataPtr->position;
	renderer_setPointLightRadius(pointLightData, a3real_onehundred);


	// set up client visualization data
	for (i = 0, j = rendererArrayLen(pluginState->obj_client),
		sceneObject = pluginState->obj_client, sceneObjectData = pluginState->objdata_client;
		i < j; ++i, ++sceneObject, ++sceneObjectData)
	{
		renderer_initSceneObject(pluginState->obj_client + i, i,
			pluginState->objdata_client, pluginState->modelstack_client);
		renderer_resetSceneObjectData(sceneObjectData);
		renderer_resetModelMatrixStack(sceneObject->modelMatrixStackPtr);

		// test posing them in a line
		sceneObjectData->position.x = (a3real)((i % 8) * 3);
		sceneObjectData->position.y = (a3real)((i / 8) * 3);
	}
}


//-----------------------------------------------------------------------------

void plugin_unload_term(sPluginState* pluginState)
{

}


//-----------------------------------------------------------------------------

void plugin_input_control(sPluginState* pluginState, double const dt)
{
	if (a3XboxControlIsConnected(pluginState->xcontrol))
		renderer_input_controlProjector_xcontrol(pluginState->xcontrol, pluginState->proj_camera_main, dt);
	else
		renderer_input_controlProjector_trad(pluginState->mouse, pluginState->keyboard, pluginState->proj_camera_main, dt);
}


//-----------------------------------------------------------------------------

void plugin_update_simulate(sPluginState* pluginState, double const dt)
{
	const a3mat4 bias = {
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f
	}, biasInv = {
		2.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 2.0f, 0.0f,
		-1.0f, -1.0f, -1.0f, 1.0f
	};

	sProjectorComponent* projector = pluginState->proj_camera_main;
	sPointLightData* pointLightData;
	a3ui32 i, j;

	// update camera
	renderer_updateSceneObject(pluginState->obj_camera_main, 1);
	renderer_updateSceneObjectStack(pluginState->obj_camera_main, projector);
	renderer_updateProjector(projector);
	renderer_updateProjectorViewMats(projector);
	renderer_updateProjectorBiasMats(projector, bias, biasInv);

	// update light
	renderer_updateSceneObject(pluginState->obj_light_main, 1);
	renderer_updateSceneObjectStack(pluginState->obj_light_main, projector);

	// update root
	renderer_updateSceneObject(pluginState->obj_root, 0);
	renderer_updateSceneObjectStack(pluginState->obj_root, projector);

	// update skybox
	renderer_updateSceneObject(pluginState->obj_skybox, 0);
	renderer_update_bindSkybox(pluginState->obj_skybox, projector);
	renderer_updateSceneObjectStack(pluginState->obj_skybox, projector);

	// update light positions and transforms
	for (i = 0, j = rendererArrayLen(pluginState->ltdata), pointLightData = pluginState->ltdata;
		i < j;
		++i, ++pointLightData)
	{
		a3real4Real4x4Product(pointLightData->position.v,
			projector->sceneObjectPtr->modelMatrixStackPtr->modelMatInverse.m,
			pointLightData->worldPos.v);
	}

	// update simulation data
	for (i = 0, j = rendererArrayLen(pluginState->obj_client);
		i < j; ++i)
	{
		renderer_updateSceneObject(pluginState->obj_client + i, 0);
		updateRSDPosition(pluginState, i);
		renderer_updateSceneObjectStack(pluginState->obj_client + i, projector);
	}

	// refill buffers
	a3bufferRefillOffset(pluginState->renderer->ubo_transform + 0, 0, 0, sizeof(pluginState->modelstack), pluginState->modelstack);
	a3bufferRefillOffset(pluginState->renderer->ubo_light + 0, 0, 0, sizeof(pluginState->ltdata), pluginState->ltdata);

	// client data buffer
	a3bufferRefillOffset(pluginState->renderer->ubo_transform + 1, 0, 0, sizeof(pluginState->modelstack_client), pluginState->modelstack_client);
}

void updateRSDPosition(sPluginState* state, int index)
{
	sSceneObjectComponent* objClient = state->obj_client + index;
	gproNet::cRakNetClient* client = state->client;

	client->getRSD().position[0] = objClient->dataPtr->position.x;
	client->getRSD().position[1] = objClient->dataPtr->position.y;
	client->getRSD().position[2] = objClient->dataPtr->position.z;

	state->client->SendRSDPosition(client->getRSD());
}


//-----------------------------------------------------------------------------

// OpenGL
#ifdef _WIN32
#include <gl/glew.h>
#include <Windows.h>
#include <GL/GL.h>
#else	// !_WIN32
#include <OpenGL/gl3.h>
#endif	// _WIN32


void plugin_render_display(sPluginState const* pluginState, double const dt)
{
	// pointers
	const sRendererProgram* currentDemoProgram;

	// framebuffers
	//const a3_Framebuffer* currentWriteFBO, * currentDisplayFBO;

	// indices
	a3ui32 i = 0, j = 0;

	// RGB
	const a3vec4 rgba4[] = {
		{ 1.00f, 0.00f, 0.00f, 1.00f },	// red
		{ 1.00f, 0.25f, 0.00f, 1.00f },
		{ 1.00f, 0.50f, 0.00f, 1.00f },	// orange
		{ 1.00f, 0.75f, 0.00f, 1.00f },
		{ 1.00f, 1.00f, 0.00f, 1.00f },	// yellow
		{ 0.75f, 1.00f, 0.00f, 1.00f },
		{ 0.50f, 1.00f, 0.00f, 1.00f },	// lime
		{ 0.25f, 1.00f, 0.00f, 1.00f },
		{ 0.00f, 1.00f, 0.00f, 1.00f },	// green
		{ 0.00f, 1.00f, 0.25f, 1.00f },
		{ 0.00f, 1.00f, 0.50f, 1.00f },	// aqua
		{ 0.00f, 1.00f, 0.75f, 1.00f },
		{ 0.00f, 1.00f, 1.00f, 1.00f },	// cyan
		{ 0.00f, 0.75f, 1.00f, 1.00f },
		{ 0.00f, 0.50f, 1.00f, 1.00f },	// sky
		{ 0.00f, 0.25f, 1.00f, 1.00f },
		{ 0.00f, 0.00f, 1.00f, 1.00f },	// blue
		{ 0.25f, 0.00f, 1.00f, 1.00f },
		{ 0.50f, 0.00f, 1.00f, 1.00f },	// purple
		{ 0.75f, 0.00f, 1.00f, 1.00f },
		{ 1.00f, 0.00f, 1.00f, 1.00f },	// magenta
		{ 1.00f, 0.00f, 0.75f, 1.00f },
		{ 1.00f, 0.00f, 0.50f, 1.00f },	// rose
		{ 1.00f, 0.00f, 0.25f, 1.00f },
	};
	const a3vec4 grey4[] = {
		{ 0.5f, 0.5f, 0.5f, 1.0f },	// solid grey
		{ 0.5f, 0.5f, 0.5f, 0.5f },	// translucent grey
	};
	const a3ui32 hueCount = sizeof(rgba4) / sizeof(*rgba4),
		redIndex = 0, orangeIndex = 2, yellowIndex = 4, limeIndex = 6,
		greenIndex = 8, aquaIndex = 10, cyanIndex = 12, skyIndex = 14,
		blueIndex = 16, purpleIndex = 18, magentaIndex = 20, roseIndex = 22;
	const a3real
		* const red = rgba4[redIndex].v, * const orange = rgba4[orangeIndex].v, * const yellow = rgba4[yellowIndex].v, * const lime = rgba4[limeIndex].v,
		* const green = rgba4[greenIndex].v, * const aqua = rgba4[aquaIndex].v, * const cyan = rgba4[cyanIndex].v, * const sky = rgba4[skyIndex].v,
		* const blue = rgba4[blueIndex].v, * const purple = rgba4[purpleIndex].v, * const magenta = rgba4[magentaIndex].v, * const rose = rgba4[roseIndex].v,
		* const grey = grey4[0].v, * const grey_t = grey4[1].v;

	// camera used for drawing
	const sProjectorComponent* activeCamera = pluginState->proj_camera_main;
	const sSceneObjectComponent* activeCameraObject = activeCamera->sceneObjectPtr;
	//const sSceneObjectComponent* currentSceneObject, * endSceneObject;

	// final model matrix and full matrix stack
	a3mat4 projectionMat = activeCamera->projectorMatrixStackPtr->projectionMat;
	a3mat4 projectionMatInv = activeCamera->projectorMatrixStackPtr->projectionMatInverse;
	a3mat4 viewProjectionMat = activeCamera->projectorMatrixStackPtr->viewProjectionMat;
	a3mat4 modelMat, modelViewProjectionMat;

	// FSQ matrix
	const a3mat4 fsq = {
		2.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 2.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	// light count
	const a3i32 lightCount = rendererArrayLen(pluginState->ltdata);


	//-------------------------------------------------------------------------

	// bind scene FBO
	a3framebufferActivate(pluginState->renderer->fbo_c16x4_d24s8);
	glDisable(GL_STENCIL_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// textures
	a3textureActivate(pluginState->renderer->tex_checker, a3tex_unit00);
	a3textureActivate(pluginState->renderer->tex_checker, a3tex_unit01);

	// select program based on settings
/*	currentDemoProgram = pluginState->renderer->prog_drawPhong;
	a3shaderProgramActivate(currentDemoProgram->program);

	// send shared data: 
	//	- projection matrix
	//	- light data
	//	- activate shared textures including atlases if using
	//	- shared animation data
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uP, 1, projectionMat.mm);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uP_inv, 1, projectionMatInv.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor0, hueCount, rgba4->v);

	// send lighting uniforms and bind blocks where appropriate
	a3shaderUniformBufferActivate(pluginState->renderer->ubo_transform + 0, renderer_program_blockTransformStack);
	a3shaderUniformBufferActivate(pluginState->renderer->ubo_light + 0, renderer_program_blockLight);
	a3shaderUniformSendInt(a3unif_single, currentDemoProgram->uCount, 1, &lightCount);

	// forward shading algorithms
	for (currentSceneObject = pluginState->obj,
		endSceneObject = currentSceneObject + rendererArrayLen(pluginState->obj);
		currentSceneObject <= endSceneObject; ++currentSceneObject)
	{
		// send index as uniform and draw; all other model data is shared
		j = currentSceneObject->sceneHierarchyIndex;
		a3shaderUniformSendInt(a3unif_single, currentDemoProgram->uIndex, 1, (a3i32*)&j);
		a3vertexDrawableActivateAndRender(drawable[j]);
	}*/

	// test render
	//j = 0;
	//a3shaderUniformSendInt(a3unif_single, currentDemoProgram->uIndex, 1, (a3i32*)&j);
	//a3vertexDrawableActivateAndRender(pluginState->renderer->draw_teapot);


	// render instanced program
	currentDemoProgram = pluginState->renderer->prog_drawPhong_instanced;
	a3shaderProgramActivate(currentDemoProgram->program);

	// render client visuals
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uP, 1, projectionMat.mm);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uP_inv, 1, projectionMatInv.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, magenta);

	// send lighting uniforms and bind blocks where appropriate
	a3shaderUniformBufferActivate(pluginState->renderer->ubo_transform + 0, renderer_program_blockTransformStack);
	a3shaderUniformBufferActivate(pluginState->renderer->ubo_light + 0, renderer_program_blockLight);
	a3shaderUniformSendInt(a3unif_single, currentDemoProgram->uCount, 1, &lightCount);

	j = rendererArrayLen(pluginState->obj_client);
	a3shaderUniformBufferActivate(pluginState->renderer->ubo_transform + 1, renderer_program_blockTransformStack);
	a3vertexDrawableActivateAndRenderInstanced(pluginState->renderer->draw_teapot, j);


	//-------------------------------------------------------------------------
	// COMPOSITION
	//	- activate target framebuffer
	//	- draw background if applicable

	// revert to back buffer and disable depth testing
	a3framebufferDeactivateSetViewport(a3fbo_depthDisable,
		-pluginState->renderer->frameBorder, -pluginState->renderer->frameBorder,
		pluginState->renderer->frameWidth, pluginState->renderer->frameHeight);

	if (pluginState->renderer->displaySkybox)
	{
		// skybox clear: just draw skybox
		modelMat = pluginState->obj_skybox->modelMatrixStackPtr->modelMat;
		renderer_drawModelTexturedColored_invertModel(
			modelViewProjectionMat.m, viewProjectionMat.m, modelMat.m, a3mat4_identity.m,
			pluginState->renderer->prog_drawTexture, pluginState->renderer->draw_unit_box,
			pluginState->renderer->tex_skybox_clouds, a3vec4_one.v);
	}
	else
	{
		// clear all color buffers
		glClear(GL_COLOR_BUFFER_BIT);
	}

	// forward shading
	//	- paste scene image in front of skybox
	currentDemoProgram = pluginState->renderer->prog_drawTexture;
	a3shaderProgramActivate(currentDemoProgram->program);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, fsq.mm);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uAtlas, 1, a3mat4_identity.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, a3vec4_one.v);
	a3framebufferBindColorTexture(pluginState->renderer->fbo_c16x4_d24s8, a3tex_unit00, 0);	// scene color


	//-------------------------------------------------------------------------
	// PREPARE FOR POST-PROCESSING
	//	- double buffer swap (if applicable)
	//	- ensure blending is disabled
	//	- re-activate FSQ drawable IF NEEDED (i.e. changed in previous step)

	// draw FSQ to complete forward/deferred composition pipeline
	renderer_enableCompositeBlending();
	a3vertexDrawableActivateAndRender(pluginState->renderer->draw_unit_plane_z);


	//-------------------------------------------------------------------------
	// OVERLAYS: done after FSQ so they appear over everything else
	//	- disable depth testing
	//	- draw overlays appropriately

	// enable alpha
	renderer_enableCompositeBlending();

	// scene overlays
	if (pluginState->renderer->displayGrid)
	{
		// activate scene FBO and clear color; reuse depth
		a3framebufferActivate(pluginState->renderer->fbo_c16x4_d24s8);
		glDisable(GL_STENCIL_TEST);
		glClear(GL_COLOR_BUFFER_BIT);
		
		renderer_drawModelSolidColor(modelViewProjectionMat.m, viewProjectionMat.m, a3mat4_identity.m,
			pluginState->renderer->prog_drawColorUnif, pluginState->renderer->draw_grid, blue);

		// display color target with scene overlays
		a3framebufferDeactivateSetViewport(a3fbo_depthDisable,
			-pluginState->renderer->frameBorder, -pluginState->renderer->frameBorder,
			pluginState->renderer->frameWidth, pluginState->renderer->frameHeight);
		currentDemoProgram = pluginState->renderer->prog_drawTexture;
		a3vertexDrawableActivate(pluginState->renderer->draw_unit_plane_z);
		a3shaderProgramActivate(currentDemoProgram->program);
		a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, fsq.mm);
		a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uAtlas, 1, a3mat4_identity.mm);
		a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, a3vec4_one.v);
		a3framebufferBindColorTexture(pluginState->renderer->fbo_c16x4_d24s8, a3tex_unit00, 0);
		a3vertexDrawableRenderActive();
	}

	// overlays with no depth
	glDisable(GL_DEPTH_TEST);

	// superimpose axes
	// draw coordinate axes in front of everything
	currentDemoProgram = pluginState->renderer->prog_drawColorAttrib;
	a3shaderProgramActivate(currentDemoProgram->program);
	a3vertexDrawableActivate(pluginState->renderer->draw_axes);

	// center of world from current viewer
	// also draw other viewer/viewer-like object in scene
	if (pluginState->renderer->displayWorldAxes)
	{
		a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, viewProjectionMat.mm);
		a3vertexDrawableRenderActive();
	}

	// individual objects (based on scene graph)
	if (pluginState->renderer->displayObjectAxes)
	{
	/*	for (currentSceneObject = pluginState->obj,
			endSceneObject = currentSceneObject + rendererArrayLen(pluginState->obj);
			currentSceneObject <= endSceneObject; ++currentSceneObject)
		{
			j = currentSceneObject->sceneHierarchyIndex;
			modelMat = currentSceneObject->modelMatrixStackPtr->modelMat;
			renderer_drawModelSimple(modelViewProjectionMat.m, viewProjectionMat.m, modelMat.m, currentDemoProgram);
		}*/
	}
}
