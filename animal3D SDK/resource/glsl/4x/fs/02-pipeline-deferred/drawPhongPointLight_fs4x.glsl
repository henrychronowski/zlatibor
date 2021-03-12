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
	
	drawPhongPointLight_fs4x.glsl
	Output Phong shading components while drawing point light volume.
*/

#version 450

#define MAX_LIGHTS 1024

// ****TO-DO:
//	-> declare biased clip coordinate varying from vertex shader
//	-> declare point light data structure and uniform block
//	-> declare pertinent samplers with geometry data ("g-buffers")
//	-> calculate screen-space coordinate from biased clip coord
//		(hint: perspective divide)
//	-> use screen-space coord to sample g-buffers
//	-> calculate view-space fragment position using depth sample
//		(hint: same as deferred shading)
//	-> calculate final diffuse and specular shading for current light only

flat in int vInstanceID; // Current light

in vec4 vBaisedClipPos;

// 1 sphere = 1 light

//layout (location = 0) out vec4 rtFragColor;
layout (location = 0) out vec4 rtDiffuseLight;
layout (location = 1) out vec4 rtSpecularLight;

uniform sampler2D uImage00; //Diffuse atlas
uniform sampler2D uImage01; //Specular atlas
uniform sampler2D uImage05; //Normal g-buffer
uniform sampler2D uImage07; //depth g-buffer

struct sPointLight
{
	vec4 viewPosition, worldPosition, color, radius;//, radiusSq, radiusInv, radiusInvSq;
};

uniform ubLight
{
	sPointLight uLights[MAX_LIGHTS];
};

// location of viewer in its own space is the origin
const vec4 kEyePos_view = vec4(0.0, 0.0, 0.0, 1.0);

// declaration of Phong shading model
//	(implementation in "utilCommon_fs4x.glsl")
//		param diffuseColor: resulting diffuse color (function writes value)
//		param specularColor: resulting specular color (function writes value)
//		param eyeVec: unit direction from surface to eye
//		param fragPos: location of fragment in target space
//		param fragNrm: unit normal vector at fragment in target space
//		param fragColor: solid surface color at fragment or of object
//		param lightPos: location of light in target space
//		param lightRadiusInfo: description of light size from struct
//		param lightColor: solid light color
void calcPhongPoint(
	out vec4 diffuseColor, out vec4 specularColor,
	in vec4 eyeVec, in vec4 fragPos, in vec4 fragNrm, in vec4 fragColor,
	in vec4 lightPos, in vec4 lightRadiusInfo, in vec4 lightColor
);

void main()
{
	vec4 position_screen = vBaisedClipPos / vBaisedClipPos.w;

	vec4 diffuseSample = texture(uImage00, position_screen.xy);
	vec4 specularSample = texture(uImage01, position_screen.xy);
	vec4 normal = texture(uImage05, position_screen.xy);

	vec4 depthSample = texture(uImage07, position_screen.xy);

	vec4 position_view = position_screen;
	position_view.z = depthSample.r;

	vec4 diffTotal = vec4(0.0f);
	vec4 specTotal = vec4(0.0f);
	vec4 diff;
	vec4 spec;

	calcPhongPoint(
			diff,
			spec, 
			normalize(kEyePos_view - position_view),
			position_view,
			normal,
			vec4(1),
			uLights[vInstanceID].viewPosition,
			uLights[vInstanceID].radius,
			uLights[vInstanceID].color
		);

		diffTotal += diff;
		specTotal += spec;

		rtDiffuseLight = diffuseSample * diffTotal;
		rtSpecularLight = specularSample * specTotal;
}
