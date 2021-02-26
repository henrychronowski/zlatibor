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
	
	drawPhong_shadow_fs4x.glsl
	Output Phong shading with shadow mapping.
*/

#version 450

// ****TO-DO:
// 1) Phong shading
//	-> identical to outcome of last project
// 2) shadow mapping
//	-> declare shadow map texture
//	-> declare shadow coordinate varying
//	-> perform manual "perspective divide" on shadow coordinate
//	-> perform "shadow test" (explained in class)

layout (location = 0) out vec4 rtFragColor;

uniform int uCount;
uniform vec4 uLightPos[4]; // World/camera
uniform vec4 uDiffuseAlbedo;
uniform vec4 uSpecularAlbedo;
uniform float uSpecularPower;
uniform vec4 uAmbient;

uniform sampler2D uImage00;

uniform vec4 uColor;

in vec4 vNormal;
in vec4 vPosition;

in vec2 vTexcoord;


void main()
{

	//Phong code from project 1
	vec4 N, L;
	vec4 kd = vec4(0.0f);
	vec4 spec = vec4(0.0f);
	for(int i = 0; i < 4; i++)
	{
		N = normalize(vNormal);
		L = normalize(uLightPos[i] - vPosition);
		vec4 view = normalize(-vPosition);
		vec4 reflection = reflect(-L, N); //https://learnopengl.com/Lighting/Basic-Lighting, https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/reflect.xhtml

		// Calculate diffuse value
		kd += max(dot(N,L), 0.0) * uDiffuseAlbedo;	// using max to ensure positivity (OpenGL blue book)

		// Calculate specular value
		spec += pow(max(dot(reflection, view), 0.0f), uSpecularPower) * uSpecularAlbedo;  // see 63 (OpenGL blue book)
	}
	
	// Output color modified by diffuse, specular, and ambient values
	vec4 color = uColor * texture2D(uImage00, vTexcoord);
	vec4 ks = kd + spec;

	rtFragColor = ks * color + uAmbient;

	//rtFragColor = vec4(1.0, 0.5, 0.0, 1.0);
}
