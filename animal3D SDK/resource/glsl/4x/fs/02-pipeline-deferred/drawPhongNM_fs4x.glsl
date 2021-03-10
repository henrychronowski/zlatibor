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

in vec4 vPosition_screen;

uniform sampler2D uImage00; //Diffuse atlas
uniform sampler2D uImage01; //Specular atlas
uniform sampler2D uImage04; //Texcoord g-buffer
uniform sampler2D uImage05; //Normal g-buffer

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
	// DUMMY OUTPUT: all fragments are OPAQUE MAGENTA
	//rtFragColor = vec4(1.0, 0.0, 1.0, 1.0);

	vec4 normal = texture(uImage05, vTexcoord.xy);


	//Phong code from project 1, slightly modified to work with proj 2 structure
	vec4 N, L;
	vec4 lightResult;
	vec4 kd = vec4(0.0f);
	vec4 spec = vec4(0.0f);
//	for(int i = 0; i < uCount; i++)
//	{
//		N = texture(uImage05, vTexcoord.xy);//normalize(texture(uImage05, vTexcoord.xy));
//		L = normalize(uLights[i].viewPosition - vPosition);
//		vec4 view = normalize(-vPosition);
//		vec4 reflection = reflect(-L, N); //https://learnopengl.com/Lighting/Basic-Lighting, https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/reflect.xhtml
//
//		//Calculate diffuse value
//		float attenuation = length(uLights[i].viewPosition) / uLights[i].radius * 3;
//		float attenuationAlbedo = 1.0f / ((attenuation * attenuation) + 1);
//		kd += max(dot(N,L), 0.0) * attenuationAlbedo;	 //using max to ensure positivity(OpenGL blue book)
//
//		//Calculate specular value
//		spec += pow(max(dot(reflection, view), 0.0f), 128) * attenuationAlbedo; //see 63 (OpenGL blue book)
//		lightResult += (kd * uLights[i].color * texture2D(uImage00, vTexcoord.xy)) + (spec* uLights[i].color * texture2D(uImage00, vTexcoord.xy));
//	}
	
	//Output color modified by diffuse, specular, and ambient values

	for(int i = 0; i < uCount; i++)
	{
		calcPhongPoint(kd,
		spec, 
		normalize(kEyePos_view - vPosition),
		vPosition,
		normal,
		texture2D(uImage00, vTexcoord.xy),
		uLights[i].viewPosition,
		uLights[i].radius,
		uLights[i].color);

		lightResult += kd + spec;
	}

	rtFragColor = lightResult;
}
