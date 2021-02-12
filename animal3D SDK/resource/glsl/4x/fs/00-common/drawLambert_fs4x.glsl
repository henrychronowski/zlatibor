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
	
	drawLambert_fs4x.glsl
	Output Lambertian shading.
*/

#version 450

// ****DONE: 
//	-> declare varyings to receive lighting and shading variables\
//	-> declare lighting uniforms
//		(hint: in the render routine, consolidate lighting data 
//		into arrays; read them here as arrays)
//	-> calculate Lambertian coefficient\
//	-> implement Lambertian shading model and assign to output
//		(hint: coefficient * attenuation * light color * surface color)
//	-> implement for multiple lights
//		(hint: there is another uniform for light count)

uniform vec4 uLightPos[4]; // World/camera
uniform vec4 uDiffuseAlbedo;
uniform vec4 uAmbient;

layout (location = 0) out vec4 rtFragColor;

in vec4 vNormal;
in vec4 vPosition;

in vec2 vTexcoord;

uniform sampler2D uImage00;

uniform vec4 uColor;

void main()
{
	vec4 N, L;
	vec4 kd = vec4(0.0f);

	// diffuse coefficient = dot(unit surface normal, unit light normal)
	for(int i = 0; i < 4; i++)
	{
		N = normalize(vNormal);
		L = normalize(uLightPos[i] - vPosition);

		kd += max(dot(N,L), 0.0) * uDiffuseAlbedo;
	}

	// Output
	vec4 color = uColor * texture2D(uImage00, vTexcoord);
	rtFragColor = kd * color + uAmbient;
	//rtFragColor = kd + color;
}
