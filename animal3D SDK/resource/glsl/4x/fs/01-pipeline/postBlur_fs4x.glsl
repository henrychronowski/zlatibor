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

	Shader written by Henry Chronowski
	Based off of information in the blue book
	
	postBlur_fs4x.glsl
	Gaussian blur.
*/

#version 450

// ****DONE:
//	-> declare texture coordinate varying and input texture
//	-> declare sampling axis uniform (see render code for clue)
//	-> declare Gaussian blur function that samples along one axis
//		(hint: the efficiency of this is described in class)

layout (location = 0) out vec4 rtFragColor;

in vec4 vTexcoord_atlas;

uniform vec2 uAxis;
uniform sampler2D uTex_dm;

// From the blue book listing 9.27
const float weights[] = float[](0.0024499299678342,
0.0043538453346397,
0.0073599963704157,
0.0118349786570722,
0.0181026699707781,
0.0263392293891488,
0.0364543006660986,
0.0479932050577658,
0.0601029809166942,
0.0715974486241365,
0.0811305381519717,
0.0874493212267511,
0.0896631113333857,
0.0874493212267511,
0.0811305381519717,
0.0715974486241365,
0.0601029809166942,
0.0479932050577658,
0.0364543006660986,
0.0263392293891488,
0.0181026699707781,
0.0118349786570722,
0.0073599963704157,
0.0043538453346397,
0.0024499299678342);

void main()
{
	// blurring along an axis
	// -> sample neigboring pixels, output weighted average
	//		-> coordinate offset by some amount (add/sub displacement vector)
	//			-> example: horizontal, dv = vec2(1 / resolution (width), 0)
	//			-> example: vertical, dv = vec2(0, 1 / height)

	int i;
	vec3 col = vec3(0.0);

	// Get displacement vector
	vec2 offset = vTexcoord_atlas.xy - (uAxis * 14.0);

	// Add weighted offset samples to the fragment color
	for(i = 0; i < weights.length(); i++)
	{
		//Tried to use texelFetch as the blue book does, but it was uncooperative so switched to just texture
		col += texture(uTex_dm, offset + (uAxis * i)).rgb * weights[i];
	}

	// Set resulting fragment color
	rtFragColor = vec4(col, 1.0);
}
