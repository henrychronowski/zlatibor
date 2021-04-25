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
	animal3D SDK: Minimal 3D Animation Framework
	By Daniel S. Buckstein
	
	gpro-net-callbacks.c/.cpp
	Main implementation of window callback hooks.

	********************************************
	*** THIS IS THE LIBRARY'S CALLBACKS FILE ***
	*** App hooks your demo via this file.   ***
	********************************************
*/

#include "gpro-net/gpro-net/gpro-net-util/gpro-net-lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//-----------------------------------------------------------------------------
// complete plugin state

#define GPRO_USING_A3
#include "sPluginState/sPluginState.h"


//-----------------------------------------------------------------------------
// callback prototypes

#ifdef __cplusplus
extern "C"
{
#endif	// __cplusplus

GPRO_NET_SYMBOL sPluginState* gproNet_load(sPluginState* state, int hotbuild);
GPRO_NET_SYMBOL sPluginState* gproNet_unload(sPluginState* state, int hotbuild);
GPRO_NET_SYMBOL int gproNet_display(sPluginState* state);
GPRO_NET_SYMBOL int gproNet_idle(sPluginState* state);
GPRO_NET_SYMBOL void gproNet_windowActivate(sPluginState* state);
GPRO_NET_SYMBOL void gproNet_windowDeactivate(sPluginState* state);
GPRO_NET_SYMBOL void gproNet_windowMove(sPluginState* state, int newWindowPosX, int newWindowPosY);
GPRO_NET_SYMBOL void gproNet_windowResize(sPluginState* state, int newWindowWidth, int newWindowHeight);
GPRO_NET_SYMBOL void gproNet_keyPress(sPluginState* state, int virtualKey);
GPRO_NET_SYMBOL void gproNet_keyHold(sPluginState* state, int virtualKey);
GPRO_NET_SYMBOL void gproNet_keyRelease(sPluginState* state, int virtualKey);
GPRO_NET_SYMBOL void gproNet_keyCharPress(sPluginState* state, int asciiKey);
GPRO_NET_SYMBOL void gproNet_keyCharHold(sPluginState* state, int asciiKey);
GPRO_NET_SYMBOL void gproNet_mouseClick(sPluginState* state, int button, int cursorX, int cursorY);
GPRO_NET_SYMBOL void gproNet_mouseDoubleClick(sPluginState* state, int button, int cursorX, int cursorY);
GPRO_NET_SYMBOL void gproNet_mouseRelease(sPluginState* state, int button, int cursorX, int cursorY);
GPRO_NET_SYMBOL void gproNet_mouseWheel(sPluginState* state, int delta, int cursorX, int cursorY);
GPRO_NET_SYMBOL void gproNet_mouseMove(sPluginState* state, int cursorX, int cursorY);
GPRO_NET_SYMBOL void gproNet_mouseLeave(sPluginState* state);

#ifdef __cplusplus
}
#endif	// __cplusplus


//-----------------------------------------------------------------------------
// renderer callbacks

#ifdef __cplusplus
extern "C"
{
#endif	// __cplusplus

// input callbacks
void renderer_input_keyCharPress(sRenderer* renderer, a3i32 const asciiKey);
void renderer_input_keyCharHold(sRenderer* renderer, a3i32 const asciiKey);

// update callbacks
void renderer_update_objects(a3f64 const dt, sSceneObjectComponent const* sceneObjectArray,
	a3ui32 const count, a3boolean const useZYX);
void renderer_update_defaultAnimation(a3f64 const dt, sSceneObjectComponent const* sceneObjectArray,
	a3ui32 const count, a3ui32 const axis, a3boolean const updateAnimation);
void renderer_update_bindSkybox(sSceneObjectComponent const* sceneObject_skybox,
	sProjectorComponent const* projector_active);
void renderer_update_pointLight(sPointLightComponent const* pointLightArray,
	sProjectorComponent const* projector_active, a3ui32 const count);

// render callbacks
void renderer_render_controls_gen(sRenderer const* renderer, a3_TextRenderer const* text, a3vec4 const col,
	a3f32 const textAlign, a3f32 const textDepth, a3f32 const textOffsetDelta, a3f32 textOffset);
void renderer_render_controls_xcontrol(a3_TextRenderer const* text, a3vec4 const col,
	a3f32 const textAlign, a3f32 const textDepth, a3f32 const textOffsetDelta, a3f32 textOffset);
void renderer_render_controls_trad(a3_TextRenderer const* text, a3vec4 const col,
	a3f32 const textAlign, a3f32 const textDepth, a3f32 const textOffsetDelta, a3f32 textOffset);
void renderer_render_timer(a3_Timer const* timer, a3_TextRenderer const* text, a3vec4 const col,
	a3f32 const textAlign, a3f32 const textDepth, a3f32 const textOffsetDelta, a3f32 textOffset);

// loading callbacks
void renderer_loadGeometry(sRenderer* renderer);
void renderer_loadShaders(sRenderer* renderer);
void renderer_loadTextures(sRenderer* renderer);
void renderer_loadFramebuffers(sRenderer* renderer);
void renderer_loadValidate(sRenderer* renderer);

// unloading callbacks
void renderer_unloadGeometry(sRenderer* renderer);
void renderer_unloadShaders(sRenderer* renderer);
void renderer_unloadTextures(sRenderer* renderer);
void renderer_unloadFramebuffers(sRenderer* renderer);
void renderer_unloadValidate(sRenderer const* renderer);

void network_update_position(sPluginState* state);

#ifdef __cplusplus
}
#endif	// __cplusplus


//-----------------------------------------------------------------------------
// miscellaneous renderer functions

// consistent text initialization
inline int renderer_initializeText(sRenderer* renderer)
{
	a3textInitialize(renderer->text + 0, 18, 1, 0, 0, 0);
	a3textInitialize(renderer->text + 1, 16, 0, 0, 0, 0);
	return a3true;
}

inline int renderer_releaseText(sRenderer* renderer)
{
	a3textRelease(renderer->text + 0);
	a3textRelease(renderer->text + 1);
	return a3false;
}


//-----------------------------------------------------------------------------
// miscellaneous plugin functions

// get the size of the persistent state to allocate
//	(good idea to keep it relatively constant, so that addresses don't change 
//	when the library is reloaded... that would mess everything up!)
inline unsigned int plugin_getPersistentStateSize()
{
	const unsigned int minimum = sizeof(sPluginState);
	unsigned int size = 1;
	while (size < minimum)
		size += size;
	return size;
}

void plugin_load(sPluginState* state)
{
	// geometry
	renderer_loadGeometry(state->renderer);

	// shaders
	renderer_loadShaders(state->renderer);

	// textures
	renderer_loadTextures(state->renderer);


	state->renderer->displayGrid = state->renderer->displaySkybox = 1;
	state->renderer->displayWorldAxes = state->renderer->displayObjectAxes = 1;
}

void plugin_unload(sPluginState* state)
{
	renderer_unloadGeometry(state->renderer);
	renderer_unloadShaders(state->renderer);
	renderer_unloadTextures(state->renderer);
	renderer_unloadFramebuffers(state->renderer);
}

void plugin_loadValidate(sPluginState* state)
{
	void plugin_load_init(sPluginState * pluginState);
	plugin_load_init(state);
}

void plugin_unloadValidate(sPluginState* state)
{
	void plugin_unload_term(sPluginState * pluginState);
	plugin_unload_term(state);
}

void plugin_input(sPluginState* state, double const dt)
{
	void plugin_input_control(sPluginState * pluginState, double const dt);
	plugin_input_control(state, dt);
}

void plugin_update(sPluginState* state, double const dt)
{
	void plugin_update_simulate(sPluginState * pluginState, double const dt);
	plugin_update_simulate(state, dt);
}

void network_update_position(sPluginState* state)
{

}

void plugin_render(sPluginState const* state, double const dt)
{
	void plugin_render_display(sPluginState const* pluginState, double const dt);

	// amount to offset text as each line is rendered
	a3f32 const textAlign = -0.98f;
	a3f32 const textDepth = -1.00f;
	a3f32 const textOffsetDelta = -0.08f;
	a3f32 textOffset;


	// draw
	plugin_render_display(state, dt);


	// deactivate things
	a3vertexDrawableDeactivate();
	a3shaderProgramDeactivate();
	a3framebufferDeactivateSetViewport(a3fbo_depthDisable, 0, 0, state->renderer->windowWidth, state->renderer->windowHeight);
	a3textureDeactivate(a3tex_unit00);

	// text
	if (state->renderer->textInit)
	{
		// control display mode
		void (* const control)(a3_TextRenderer const*, a3vec4 const, a3f32 const, a3f32 const, a3f32 const, a3f32)
			= a3XboxControlIsConnected(state->xcontrol)
			? renderer_render_controls_xcontrol
			: renderer_render_controls_trad;

		// 5 passes of text to get decent visibility
		a3_TextRenderer const* text = state->renderer->text;
		a3vec4 const bg = a3vec4_w, fg = a3vec4_one;
		a3real const px = state->renderer->windowWidthInv * a3real_two, py = state->renderer->windowHeightInv * a3real_two;

		a3vec4 const color[5] = { bg, bg, bg, bg, fg };
		a3real const x_offset[5] = { -px, +px, -px, +px, a3real_zero };
		a3real const y_offset[5] = { -py, -py, +py, +py, a3real_zero };

		a3vec4 col;
		a3real x, y;
		a3ui32 i;
		for (i = 0; i < 5; ++i)
		{
			textOffset = +1.00f;
			col = color[i];
			x = x_offset[i];
			y = y_offset[i];

			// choose text render mode
			switch (state->renderer->textMode)
			{
				// controls for current mode
			case renderer_textControls:
				break;

				// controls for general
			case renderer_textControls_gen:
				renderer_render_controls_gen(state->renderer, text, col, textAlign + x, textDepth, textOffsetDelta, textOffset + y);
				break;

				// general data
			case renderer_textData:
				renderer_render_timer(state->timer_display, text, col, textAlign + x, textDepth, textOffsetDelta, textOffset + y);
				break;
			}

			// controls
			if (state->renderer->textMode != renderer_textDisabled)
				control(text, col, textAlign + x, textDepth, textOffsetDelta, (textOffset = -0.60f) + y);
		}
	}
}

void plugin_extra_xcontrol(sPluginState* state)
{
	if (a3XboxControlIsConnected(state->xcontrol))
	{
		// toggle text mode
		if (a3XboxControlIsPressed(state->xcontrol, a3xbox_DPAD_up))
			rendererCtrlIncLoop(state->renderer->textMode, renderer_text_max);
		if (a3XboxControlIsPressed(state->xcontrol, a3xbox_DPAD_down))
			rendererCtrlDecLoop(state->renderer->textMode, renderer_text_max);

		// toggle text initialization
		if (a3XboxControlIsPressed(state->xcontrol, a3xbox_back))
		{
			if (!a3textIsInitialized(state->renderer->text))
				state->renderer->textInit = renderer_initializeText(state->renderer);
			else
				state->renderer->textInit = renderer_releaseText(state->renderer);
		}

		// reload shaders
		if (a3XboxControlIsPressed(state->xcontrol, a3xbox_start))
		{
			renderer_unloadShaders(state->renderer);
			renderer_loadShaders(state->renderer);
		}
	}
}

void plugin_extra_trad(sPluginState* state, int asciiKey)
{
	// handle special cases immediately
	switch (asciiKey)
	{
		// uncomment to make escape key kill the current demo
		// if disabled, use 'exit demo' menu option
//	case 27: 
//		state->exitFlag = 1;
//		break;


		// toggle text mode
		rendererCtrlCaseIncLoop(state->renderer->textMode, renderer_text_max, 't');

		// reload text
	case 'T':
		if (!a3textIsInitialized(state->renderer->text))
			state->renderer->textInit = renderer_initializeText(state->renderer);
		else
			state->renderer->textInit = renderer_releaseText(state->renderer);
		break;


		// reload all shaders in real-time
	case 'P':
		renderer_unloadShaders(state->renderer);
		renderer_loadShaders(state->renderer);
		break;
	}
}

void plugin_idle(sPluginState* state, double const dt)
{
	// main idle loop
	plugin_input(state, dt);
	plugin_update(state, dt);
	network_update_position(state);
	plugin_render(state, dt);

	// network messaging
	state->client->MessageLoop();

	// update input (reset wheel)
	a3mouseSetStateWheel(state->mouse, a3mws_neutral);
	a3mouseUpdate(state->mouse);
	a3keyboardUpdate(state->keyboard);
	a3XboxControlUpdate(state->xcontrol);

	// extra controls otherwise handled by window callbacks
	plugin_extra_xcontrol(state);
}


//-----------------------------------------------------------------------------
// callback implementations

// demo is loaded
GPRO_NET_SYMBOL sPluginState *gproNet_load(sPluginState *state, int hotbuild)
{
	unsigned int const stateSize = plugin_getPersistentStateSize();
	unsigned int const trigSamplesPerDegree = 4;
	
/*	// do any re-allocation tasks
	if (state && hotbuild)
	{
		// example 1: copy memory directly
		sPluginState *state_copy = (sPluginState*)malloc(stateSize);
		if (state_copy)
		{
			// copy and release
			*state_copy = *state;
			free(state);
			state = state_copy;
		}
		
		// reset pointers
		a3trigInitSetTables(trigSamplesPerDegree, state->trigTable);

		// call refresh to re-link pointers in case demo state address changed
		plugin_loadValidate(state);
	}
	else if (state = (sPluginState*)malloc(stateSize))
*/
	// NOT ANTICIPATING NEED FOR HOTBUILD
	// use new operator to invoke C++ constructors of state members
	
	// do any initial allocation tasks
	// HEAP allocate persistent state
	// stack object will be deleted at the end of the function
	// good idea to set the whole block of memory to zero
	if (state = new sPluginState)
	{
		// reset renderer
		memset(state->renderer, 0, sizeof(state->renderer));

		// reset input
		memset(state->mouse, 0, sizeof(state->mouse));
		memset(state->keyboard, 0, sizeof(state->keyboard));
		memset(state->xcontrol, 0, sizeof(state->xcontrol));

		// set up trig table (A3DM)
		memset(state->trigTable, 0, sizeof(state->trigTable));
		a3trigInit(trigSamplesPerDegree, state->trigTable);

		// text
		renderer_initializeText(state->renderer);
		state->renderer->textInit = a3true;
		state->renderer->textMode = renderer_textData;

		// enable asset streaming between loads
		state->renderer->streaming = a3true;

		// create directory for data
		a3fileStreamMakeDirectory("./data");

		// set default GL state
		renderer_setDefaultGraphicsState();

		// demo modes
		plugin_loadValidate(state);
		plugin_load(state);

		// initialize state variables
		// e.g. timer, thread, etc.
		memset(state->timer_display, 0, sizeof(state->timer_display));
		a3timerReset(state->timer_display);
		a3timerSet(state->timer_display, 60.0);
		a3timerStart(state->timer_display);
	}

	// return persistent state pointer
	return state;
}

// demo is unloaded; option to unload to prep for hotbuild
GPRO_NET_SYMBOL sPluginState *gproNet_unload(sPluginState *state, int hotbuild)
{
	// release things that need releasing always, whether hotbuilding or not
	// e.g. kill thread
	// nothing in this example, but then...

	// release persistent state if not hotbuilding
	// good idea to release in reverse order that things were loaded...
	//	...normally; however, in this case there are reference counters 
	//	to make sure things get deleted when they need to, so there is 
	//	no need to reverse!
	if (state)
	{
		if (!hotbuild)
		{
			// free fixed objects
			renderer_releaseText(state->renderer);

			// free graphics objects
			plugin_unload(state);

			// validate unload
			plugin_unloadValidate(state);

			// erase other stuff
			a3trigFree();

			// erase persistent state
			//free(state);
			delete state;
			state = 0;
		}
	}

	// return state pointer
	return state;
}

// window updates display
// **NOTE: DO NOT USE FOR RENDERING**
GPRO_NET_SYMBOL int gproNet_display(sPluginState *state)
{
	// do nothing, should just return 1 to indicate that the 
	//	window's display area is controlled by the demo
	return 1;
}

// window idles
GPRO_NET_SYMBOL int gproNet_idle(sPluginState *state)
{
	// perform any idle tasks, such as rendering
	if (!state->renderer->exitFlag)
	{
		if (a3timerUpdate(state->timer_display) > 0)
		{
			// render timer ticked, update demo state and draw
			double const dt = state->timer_display->secondsPerTick;
			plugin_idle(state, dt);

			// render occurred this idle: return +1
			return +1;
		}

		// nothing happened this idle: return 0
		return 0;
	}

	// demo should exit now: return -1
	return -1;
}

// window gains focus
GPRO_NET_SYMBOL void gproNet_windowActivate(sPluginState *state)
{
	// nothing really needs to be done here...
	//	but it's here just in case
}

// window loses focus
GPRO_NET_SYMBOL void gproNet_windowDeactivate(sPluginState *state)
{
	// reset input; it won't track events if the window is inactive, 
	//	active controls will freeze and you'll get strange behaviors
	a3keyboardReset(state->keyboard);
	a3mouseReset(state->mouse);
	a3XboxControlReset(state->xcontrol);
	a3XboxControlSetRumble(state->xcontrol, 0, 0);
}

// window moves
GPRO_NET_SYMBOL void gproNet_windowMove(sPluginState *state, int newWindowPosX, int newWindowPosY)
{
	// nothing needed here
}

// window resizes
GPRO_NET_SYMBOL void gproNet_windowResize(sPluginState *state, int newWindowWidth, int newWindowHeight)
{
	// account for borders here
	const int frameBorder = 0;
	const unsigned int frameWidth = newWindowWidth + frameBorder + frameBorder;
	const unsigned int frameHeight = newWindowHeight + frameBorder + frameBorder;
	const a3real windowAspect = (a3real)newWindowWidth / (a3real)newWindowHeight;
	const a3real frameAspect = (a3real)frameWidth / (a3real)frameHeight;

	// copy new values to demo state
	state->renderer->windowWidth = newWindowWidth;
	state->renderer->windowHeight = newWindowHeight;
	state->renderer->windowWidthInv = a3recip((a3real)newWindowWidth);
	state->renderer->windowHeightInv = a3recip((a3real)newWindowHeight);
	state->renderer->windowAspect = windowAspect;
	state->renderer->frameWidth = frameWidth;
	state->renderer->frameHeight = frameHeight;
	state->renderer->frameWidthInv = a3recip((a3real)frameWidth);
	state->renderer->frameHeightInv = a3recip((a3real)frameHeight);
	state->renderer->frameAspect = frameAspect;
	state->renderer->frameBorder = frameBorder;

	// framebuffers should be initialized or re-initialized here 
	//	since they are likely dependent on the window size
	renderer_unloadFramebuffers(state->renderer);
	renderer_loadFramebuffers(state->renderer);

	// use framebuffer deactivate utility to set viewport
	a3framebufferDeactivateSetViewport(a3fbo_depthDisable, -frameBorder, -frameBorder, state->renderer->frameWidth, state->renderer->frameHeight);

	// viewing info for projection matrix
	state->proj_camera_main->dataPtr->aspect = frameAspect;
}

// any key is pressed
GPRO_NET_SYMBOL void gproNet_keyPress(sPluginState *state, int virtualKey)
{
	// persistent state update
	a3keyboardSetState(state->keyboard, (a3_KeyboardKey)virtualKey, a3input_down);
}

// any key is held
GPRO_NET_SYMBOL void gproNet_keyHold(sPluginState *state, int virtualKey)
{
	// persistent state update
	a3keyboardSetState(state->keyboard, (a3_KeyboardKey)virtualKey, a3input_down);
}

// any key is released
GPRO_NET_SYMBOL void gproNet_keyRelease(sPluginState *state, int virtualKey)
{
	// persistent state update
	a3keyboardSetState(state->keyboard, (a3_KeyboardKey)virtualKey, a3input_up);
}

// ASCII key is pressed (immediately preceded by "any key" pressed call above)
// NOTE: there is no release counterpart
GPRO_NET_SYMBOL void gproNet_keyCharPress(sPluginState *state, int asciiKey)
{
	// persistent state update
	a3keyboardSetStateASCII(state->keyboard, (a3byte)asciiKey);

	// extra controls
	plugin_extra_trad(state, asciiKey);

	// demo callback
	renderer_input_keyCharPress(state->renderer, asciiKey);
}

// ASCII key is held
GPRO_NET_SYMBOL void gproNet_keyCharHold(sPluginState *state, int asciiKey)
{
	// persistent state update
	a3keyboardSetStateASCII(state->keyboard, (a3byte)asciiKey);

	// demo callback
	renderer_input_keyCharHold(state->renderer, asciiKey);
}

// mouse button is clicked
GPRO_NET_SYMBOL void gproNet_mouseClick(sPluginState *state, int button, int cursorX, int cursorY)
{
	// persistent state update
	a3mouseSetState(state->mouse, (a3_MouseButton)button, a3input_down);
	a3mouseSetPosition(state->mouse, cursorX, cursorY);
}

// mouse button is double-clicked
GPRO_NET_SYMBOL void gproNet_mouseDoubleClick(sPluginState *state, int button, int cursorX, int cursorY)
{
	// persistent state update
	a3mouseSetState(state->mouse, (a3_MouseButton)button, a3input_down);
	a3mouseSetPosition(state->mouse, cursorX, cursorY);
}

// mouse button is released
GPRO_NET_SYMBOL void gproNet_mouseRelease(sPluginState *state, int button, int cursorX, int cursorY)
{
	// persistent state update
	a3mouseSetState(state->mouse, (a3_MouseButton)button, a3input_up);
	a3mouseSetPosition(state->mouse, cursorX, cursorY);
}

// mouse wheel is turned
GPRO_NET_SYMBOL void gproNet_mouseWheel(sPluginState *state, int delta, int cursorX, int cursorY)
{
	// persistent state update
	a3mouseSetStateWheel(state->mouse, (a3_MouseWheelState)delta);
	a3mouseSetPosition(state->mouse, cursorX, cursorY);
}

// mouse moves
GPRO_NET_SYMBOL void gproNet_mouseMove(sPluginState *state, int cursorX, int cursorY)
{
	// persistent state update
	a3mouseSetPosition(state->mouse, cursorX, cursorY);
}

// mouse leaves window
GPRO_NET_SYMBOL void gproNet_mouseLeave(sPluginState *state)
{
	// reset mouse state or any buttons pressed will freeze
	a3mouseReset(state->mouse);
}


//-----------------------------------------------------------------------------
