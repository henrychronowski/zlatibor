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
	
	postBlend_fs4x.glsl
	Blending layers, composition.
*/

#version 450

// ****DONE:
//	-> declare texture coordinate varying and set of input textures
//	-> implement some sort of blending algorithm that highlights bright areas
//		(hint: research some Photoshop blend modes)

layout (location = 0) out vec4 rtFragColor;

uniform sampler2D uImage00, uImage01, uImage02, uImage03;

in vec4 vTexcoord_atlas;


// Attempt to implement a proper overlay type blend (https://en.wikipedia.org/wiki/Blend_modes#Overlay), works but does not look great
//vec3 Overlay_Blend (vec3 a, vec3 b)
//{
//	vec3 col;
//
//	float lumA = dot(a, vec3(0.299, 0.587, 0.144));
//	float lumB = dot(b, vec3(0.299, 0.587, 0.144));
//
//	// Would convert to a mix
//	if(lumA < 0.5)
//	{
//		col = 2 * (a * b);
//	}
//	else
//	{
//		col = vec3(1.0) - 2.0 * (vec3(1.0) - a) * (vec3(1.0) - b);
//	}
//
//	return col;
//}

void main()
{
	// All resultant textures
	vec3 col0 = texture(uImage00, vTexcoord_atlas.xy).rgb;
	vec3 col1 = texture(uImage01, vTexcoord_atlas.xy).rgb;
	vec3 col2 = texture(uImage02, vTexcoord_atlas.xy).rgb;
	vec3 col3 = texture(uImage03, vTexcoord_atlas.xy).rgb;

	// Simply additively blend the textures with a scale value for bloom
	vec3 bloom = col1 + col2 + col3;
	float bloomScale = 0.4;
	rtFragColor = vec4(col0 + bloomScale * bloom, 1.0);
}
