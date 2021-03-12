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
	
	drawPhongNM_fs4x.glsl
	Output Phong shading with normal mapping.
*/

#version 450

#define MAX_LIGHTS 1024

// ****TO-DO:
//	-> declare view-space varyings from vertex shader
//	-> declare point light data structure and uniform block
//	-> declare uniform samplers (diffuse, specular & normal maps)
//	-> calculate final normal by transforming normal map sample
//	-> calculate common view vector
//	-> declare lighting sums (diffuse, specular), initialized to zero
//	-> implement loop in main to calculate and accumulate light
//	-> calculate and output final Phong sum

uniform int uCount;

layout (location = 0) out vec4 rtFragColor;

in vec4 vPosition;
in vec4 vNormal;
in vec4 vTexcoord;
in vec4 vTangent;
in vec4 vBitangent;

in vec4 vPosition_screen;
in mat4 vTBN;

uniform sampler2D uImage00; //Diffuse atlas
uniform sampler2D uImage01; //Specular atlas
uniform sampler2D uImage04; //Texcoord g-buffer
uniform sampler2D uImage05; //Normal g-buffer

//uniform mat4 u

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
	vec4 normal = texture(uImage05, vTexcoord.xy) + normalize(vNormal); // texture(uImage05, vTexcoord.xy); //normalize(vNormal);//texture(uImage05, vTexcoord.xy) + normalize(vNormal);
	normal = normalize(normal *  2.0 - 1.0);
	//normal = normalize(vTBN * normal) * 0.5 + 0.5; //https://learnopengl.com/Advanced-Lighting/Normal-Mapping

	//normal = normalize(normal) * 0.5 + 0.5;

	vec4 diffTotal = vec4(0.0f);
	vec4 specTotal = vec4(0.0f);
	vec4 diff;
	vec4 spec;


	for(int i = 0; i < uCount; i++)
	{
		
		calcPhongPoint(
			diff,
			spec, 
			normalize(kEyePos_view - vPosition),
			vPosition,
			normal,
			vec4(1),
			uLights[i].viewPosition,
			uLights[i].radius,
			uLights[i].color
		);

		diffTotal += diff;
		specTotal += spec;
	}

	rtFragColor = diffTotal * texture2D(uImage00, vTexcoord.xy) + specTotal * texture2D(uImage01, vTexcoord.xy);


	//rtFragColor = normal;

	rtFragColor.a = 1.0;

//	rtFragColor = normal;
//	rtFragColor.a = 1.0;
}
