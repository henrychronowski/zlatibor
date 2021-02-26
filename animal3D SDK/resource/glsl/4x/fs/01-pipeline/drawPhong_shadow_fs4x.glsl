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

	Phong shading method from previous project
	
	drawPhong_shadow_fs4x.glsl
	Output Phong shading with shadow mapping.
*/

#version 450

// ****DONE:
// 1) Phong shading
//	-> identical to outcome of last project
// 2) shadow mapping
//	-> declare shadow map texture
//	-> declare shadow coordinate varying
//	-> perform manual "perspective divide" on shadow coordinate
//	-> perform "shadow test" (explained in class)

layout (location = 0) out vec4 rtFragColor;

uniform int uCount;

struct sPointLight
{
	vec4 viewPosition;
	vec4 worldPosition;
	vec4 color;
	float radius;
};

uniform ubLight {
	sPointLight uLights[8];
};

uniform sampler2D uImage00;
uniform sampler2D uTex_shadow;

uniform vec4 uColor;

in vec4 vShadowCoord;
in vec4 vNormal;
in vec4 vPosition;

in vec2 vTexcoord;


void main()
{

	//Phong code from project 1, slightly modified to work with proj 2 structure
	vec4 N, L;
	vec4 color;
	vec4 kd = vec4(0.0f);
	vec4 spec = vec4(0.0f);
	for(int i = 0; i < uCount; i++)
	{
		N = normalize(vNormal);
		L = normalize(uLights[i].viewPosition - vPosition);
		vec4 view = normalize(-vPosition);
		vec4 reflection = reflect(-L, N); //https://learnopengl.com/Lighting/Basic-Lighting, https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/reflect.xhtml

		//Calculate diffuse value
		float attenuation = length(uLights[i].viewPosition) / uLights[i].radius * 3;
		float attenuationAlbedo = 1.0f / ((attenuation * attenuation) + 1);
		kd += max(dot(N,L), 0.0) * attenuationAlbedo;	 //using max to ensure positivity(OpenGL blue book)

		//Calculate specular value
		spec += pow(max(dot(reflection, view), 0.0f), 128) * attenuationAlbedo; //see 63 (OpenGL blue book)
		color = uLights[i].color * texture2D(uImage00, vTexcoord);
	}
	
	//Output color modified by diffuse, specular, and ambient values
	
	vec4 ks = kd + spec;

	//Shadow Mapping
	vec4 shadowCoordDiv = vShadowCoord / vShadowCoord.w; //Perspective divide
	vec4 shadowTexture = texture(uTex_shadow, shadowCoordDiv.xy);

	//Shadow test - looked at these sources for inspiration: https://fabiensanglard.net/shadowmapping/index.php, http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
	float bias = 0.005;
	float shadowVisability = 1.0;
	shadowVisability = shadowTexture.r < shadowCoordDiv.z - bias ? 0.5 : 1.0;

	vec4 phong = ks * color;

	rtFragColor = shadowVisability * phong;
}
