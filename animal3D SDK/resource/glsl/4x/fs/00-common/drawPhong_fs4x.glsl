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
	
	drawPhong_fs4x.glsl
	Output Phong shading.
*/

#version 450

// ****TO-DO: 
//	-> start with list from "drawLambert_fs4x"
//		(hint: can put common stuff in "utilCommon_fs4x" to avoid redundancy)
//	-> calculate view vector, reflection vector and Phong coefficient
//	-> calculate Phong shading model for multiple lights

layout (location = 0) out vec4 rtFragColor;

uniform vec4 uLightPos[4]; // World/camera
uniform float uLightRadius;

in vec4 vNormal;
in vec4 vPosition;

in vec2 vTexcoord;

uniform sampler2D uImage00;

uniform vec4 uColor;

void main()
{
	vec4 N, L;
	float kd = 0.0f;
	float spec;
	for(int i = 0; i < 4; i++)
	{
		N = normalize(vNormal);
		L = normalize(uLightPos[i] - vPosition);
		kd += dot(N,L) * uLightRadius;

		//float specular = 0.0f;
		vec4 reflection = reflect(-L, N); //https://learnopengl.com/Lighting/Basic-Lighting, https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/reflect.xhtml
		vec4 view = normalize(-vPosition);

		spec += dot(reflection, view);
	}
	
	// Output
	vec4 color = uColor * texture2D(uImage00, vTexcoord);
	float ks = kd * spec;
	rtFragColor = ks * color;
}
