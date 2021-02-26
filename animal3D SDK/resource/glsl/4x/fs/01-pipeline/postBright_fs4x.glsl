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
	
	postBright_fs4x.glsl
	Bright pass filter.
*/

#version 450

// ****TO-DO:
//	-> declare texture coordinate varying and input texture
//	-> implement relative luminance function
//	-> implement simple "tone mapping" such that the brightest areas of the 
//		image are emphasized, and the darker areas get darker

layout (location = 0) out vec4 rtFragColor;

in vec2 vTexcoord;
in vec4 vTexcoord_atlas;

//uniform float uBrightThreshold;

//uniform sampler2D uImage00;
uniform sampler2D uTex_dm;

void main()
{
	// vec3 luminanceFormula = vec3(0.2126, 0.7152, 0.0722);	// https://en.wikipedia.org/wiki/Relative_luminance
	vec3 luminanceFormula = vec3(0.299, 0.587, 0.144);	// From the blue book listing 9.26


	vec3 color = texture2D(uTex_dm, vTexcoord_atlas.xy).rgb;
	float luminance = dot(luminanceFormula, color.xyz);
	color *= 3.0 * luminance * luminance; // This was found too look cool from experimentation initially based on the formula in the blue book

	rtFragColor = vec4(color, 1.0f);
}
