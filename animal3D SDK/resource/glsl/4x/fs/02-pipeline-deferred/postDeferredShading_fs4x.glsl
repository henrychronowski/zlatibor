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
	
	postDeferredShading_fs4x.glsl
	Calculate full-screen deferred Phong shading.
*/

#version 450

#define MAX_LIGHTS 1024

// ****TO-DO:
//	-> this one is pretty similar to the forward shading algorithm (Phong NM) 
//		except it happens on a plane, given images of the scene's geometric 
//		data (the "g-buffers"); all of the information about the scene comes 
//		from screen-sized textures, so use the texcoord varying as the UV
//	-> declare point light data structure and uniform block
//	-> declare pertinent samplers with geometry data ("g-buffers")
//	-> use screen-space coord (the inbound UV) to sample g-buffers
//	-> calculate view-space fragment position using depth sample
//		(hint: modify screen-space coord, use appropriate matrix to get it 
//		back to view-space, perspective divide)
//	-> calculate and accumulate final diffuse and specular shading

in vec4 vTexcoord_atlas;

uniform int uCount;
uniform sampler2D uImage00; //Diffuse atlas
uniform sampler2D uImage01; //Specular atlas

uniform sampler2D uImage04; //Texcoord g-buffer
uniform sampler2D uImage05; //Normal g-buffer
//uniform sampler2D uImage06; //Position g-buffer
uniform sampler2D uImage07; //depth g-buffer

uniform mat4 uPB_inv; //inverse bias projection

//testing
//uniform sampler2D uImage02, uImage03; //rnm, ht

layout (location = 0) out vec4 rtFragColor;

void main()
{
	// DUMMY OUTPUT: all fragments are OPAQUE ORANGE
	//rtFragColor = vec4(1.0, 0.5, 0.0, 1.0);

	vec4 sceneSample = texture(uImage04, vTexcoord_atlas.xy);
	vec4 diffuseSample = texture(uImage00, sceneSample.xy);
	vec4 specularSample = texture(uImage01, sceneSample.xy);

	vec4 position_screen = vTexcoord_atlas;
	position_screen.z = texture(uImage07, vTexcoord_atlas.xy).r;

	vec4 position_view = uPB_inv * position_screen;
	position_view /= position_view.w;


	vec4 normal = texture(uImage05, vTexcoord_atlas.xy);
	normal = (normal - 0.5) * 2.0;

	//Phong Shading:
	// ambient + diffuse color * diffuse light + specular color * specular light
	//have:
	// -> colors
	//have not:
	// -> light stuff
	//		-> light data -> light data struct -> uniform buffer (ubo_light)
	//		-> normals, view position, depth -> geometry buffers
	// -> texture coordinates -> g-buffer


	rtFragColor = diffuseSample;

	//DEBUG
	//rtFragColor = texture(uImage07, vTexcoord_atlas.xy);
	//rtFragColor = position_screen;
	//rtFragColor = position_view;
	rtFragColor = normal;

	// final transparency
	rtFragColor.a = diffuseSample.a;
}
