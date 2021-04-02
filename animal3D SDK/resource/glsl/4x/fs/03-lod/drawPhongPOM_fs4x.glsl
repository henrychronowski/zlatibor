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
	
	drawPhongPOM_fs4x.glsl
	Output Phong shading with parallax occlusion mapping (POM).
*/

#version 450

#define MAX_LIGHTS 1024

in vbVertexData {
	mat4 vTangentBasis_view;
	vec4 vTexcoord_atlas;
};

struct sPointLight
{
	vec4 viewPos, worldPos, color, radiusInfo;
};

uniform ubLight
{
	sPointLight uPointLight[MAX_LIGHTS];
};

uniform int uCount;

uniform vec4 uColor;

uniform float uSize;

uniform sampler2D uTex_dm, uTex_sm, uTex_nm, uTex_hm;

const vec4 kEyePos = vec4(0.0, 0.0, 0.0, 1.0);

layout (location = 0) out vec4 rtFragColor;
layout (location = 1) out vec4 rtFragNormal;

void calcPhongPoint(out vec4 diffuseColor, out vec4 specularColor, in vec4 eyeVec,
	in vec4 fragPos, in vec4 fragNrm, in vec4 fragColor,
	in vec4 lightPos, in vec4 lightRadiusInfo, in vec4 lightColor);

// Referenced the class powerpoint and the following article	
// https://learnopengl.com/Advanced-Lighting/Parallax-Mapping
vec3 calcParallaxCoord(in vec3 coord, in vec3 viewVec, const int steps)
{
	// ****D0NE:
	//	-> step along view vector until intersecting height map
	//	-> determine precise intersection point, return resulting coordinate

	// Sample height map
	float height = texture(uTex_hm, coord.xy).r;
	vec2 p = viewVec.xy / viewVec.z * uSize;
	coord.z = 1.0;

	// Set initial end from heightmap sample and declare variables for stepping
	vec3 end = vec3(coord.xy - p, 0.0);
	vec3 offsetCoord;
	float dt = 1.0 / float(steps);
	float bPrev = height, bCur;
	vec3 hPrev = coord;

	// Step through ray, checking if heightmap is larger than the current sample
	for(int i = 0; i < steps; i++)
	{
		// Interp to get the offstep coord for this step
		offsetCoord = mix(coord, end, float(i) * dt);

		// Sample the heightmap at the current offstep coord
		bCur = texture(uTex_hm, offsetCoord.xy).r;

		// Check if the heightmap is taller than the current offset coord
		if(offsetCoord.z < bCur)
		{
			// Get the x coordinate of the intersection point between the ray and the heightmap
			float x = (hPrev.p - bPrev)/ ((bCur - bPrev) - (offsetCoord.b - hPrev.z));

			// Interp using the calculated x coordinate to get the final offset coordinate
			coord = mix(hPrev, offsetCoord, x);
			break;
		}

		// Iterate the previous height and coordinate variables
		bPrev = bCur;
		hPrev = offsetCoord;
	}

	return coord;
}

void main()
{
	vec4 diffuseColor = vec4(0.0), specularColor = diffuseColor, dd, ds;
	
	// view-space tangent basis
	vec4 tan_view = normalize(vTangentBasis_view[0]);
	vec4 bit_view = normalize(vTangentBasis_view[1]);
	vec4 nrm_view = normalize(vTangentBasis_view[2]);
	vec4 pos_view = vTangentBasis_view[3];
	
	// view-space view vector
	vec4 viewVec = normalize(kEyePos - pos_view);
	
	// ****D0N3:
	//	-> convert view vector into tangent space
	//		(hint: the above TBN bases convert tangent to view, figure out 
	//		an efficient way of representing the required matrix operation)
	// tangent-space view vector

	mat4 invTBN = transpose(mat4(tan_view, bit_view, nrm_view, vec4(vec3(0.0),1.0)));
	vec3 viewVec_tan = (invTBN * viewVec).xyz;
	
	// parallax occlusion mapping
	vec3 texcoord = vec3(vTexcoord_atlas.xy, uSize);
	texcoord = calcParallaxCoord(texcoord, viewVec_tan, 256);
	
	// read and calculate view normal
	vec4 sample_nm = texture(uTex_nm, texcoord.xy);
	nrm_view = mat4(tan_view, bit_view, nrm_view, kEyePos)
		* vec4((sample_nm.xyz * 2.0 - 1.0), 0.0);
	
	int i;
	for (i = 0; i < uCount; ++i)
	{
		calcPhongPoint(dd, ds, viewVec, pos_view, nrm_view, uColor, 
			uPointLight[i].viewPos, uPointLight[i].radiusInfo,
			uPointLight[i].color);
		diffuseColor += dd;
		specularColor += ds;
	}

	vec4 sample_dm = texture(uTex_dm, texcoord.xy);
	vec4 sample_sm = texture(uTex_sm, texcoord.xy);
	rtFragColor = sample_dm * diffuseColor + sample_sm * specularColor;
	rtFragColor.a = sample_dm.a;
	
	// MRT
	rtFragNormal = vec4(nrm_view.xyz * 0.5 + 0.5, 1.0);
}
