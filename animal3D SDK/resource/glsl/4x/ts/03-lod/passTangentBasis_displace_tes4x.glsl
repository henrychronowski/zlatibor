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
	
	passTangentBasis_displace_tes4x.glsl
	Pass interpolated and displaced tangent basis.
*/

#version 450

// ****TO-DO: 
//	-> declare inbound and outbound varyings to pass along vertex data
//		(hint: inbound matches TCS naming and is still an array)
//		(hint: outbound matches GS/FS naming and is singular)
//	-> copy varying data from input to output
//	-> displace surface along normal using height map, project result
//		(hint: start by testing a "pass-thru" shader that only copies 
//		gl_Position from the previous stage to get the hang of it)

layout (triangles, equal_spacing) in;

in vbVertexData_tess {
	mat4 vTangentBasis_view;
	vec4 vTexcoord_atlas;
} vVertexData_tess[];

out vbVertexData {
	mat4 vTangentBasis_view;
	vec4 vTexcoord_atlas;
};

uniform mat4 uP;
uniform sampler2D uTex_hm;

void main()
{
	// Blend the vertex offset into the attributes of vTangentBasis_view
	for(int i = 0; i < 4; i++)
	{
		vTangentBasis_view[i] = (gl_TessCoord.x * vVertexData_tess[0].vTangentBasis_view[i] +
								gl_TessCoord.y * vVertexData_tess[1].vTangentBasis_view[i] +
								gl_TessCoord.z * vVertexData_tess[2].vTangentBasis_view[i]);
	}
	
	// Blend the vertex offset into the texcoords
	vTexcoord_atlas = (gl_TessCoord.x * vVertexData_tess[0].vTexcoord_atlas +
					   gl_TessCoord.y * vVertexData_tess[1].vTexcoord_atlas +
					   gl_TessCoord.z * vVertexData_tess[2].vTexcoord_atlas);

	// Get normal, position, and height at current vertex
	vec4 normal = normalize(vTangentBasis_view[2]);
	vec4 pos = vTangentBasis_view[3];
	float height = texture(uTex_hm, vTexcoord_atlas.xy).x;

	// Scale position by a factor of height in the direction of the normal
	pos += normal * height; // Could implement a scaling factor on height to control the strength of displacement

	// Update position with displaced vertices
	vTangentBasis_view[3] = pos;
	 gl_Position = uP * vTangentBasis_view[3];
}
