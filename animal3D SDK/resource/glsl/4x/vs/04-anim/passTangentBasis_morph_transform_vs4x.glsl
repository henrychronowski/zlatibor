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

	Completed by Henry Chronowski and Ethan Heil
	
	passTangentBasis_morph_transform_vs4x.glsl
	Calculate and pass morphed tangent basis.
*/

#version 450

#define MAX_OBJECTS 128

// ****D0N3: 
//	-> declare morph target attributes
//	-> declare and implement morph target interpolation algorithm
//	-> declare interpolation time/param/keyframe uniform
//	-> perform morph target interpolation using correct attributes
//		(hint: results can be stored in local variables named after the 
//		complete tangent basis attributes provided before any changes)

// Struct to represent morph targets
struct sMorp
{
	vec3 position;
	vec3 normal;
	vec3 tangent;
}; 

layout (location = 0) in sMorp aMorp[5];
layout (location = 15) in vec2 aTexcoord;

struct sModelMatrixStack
{
	mat4 modelMat;						// model matrix (object -> world)
	mat4 modelMatInverse;				// model inverse matrix (world -> object)
	mat4 modelMatInverseTranspose;		// model inverse-transpose matrix (object -> world skewed)
	mat4 modelViewMat;					// model-view matrix (object -> viewer)
	mat4 modelViewMatInverse;			// model-view inverse matrix (viewer -> object)
	mat4 modelViewMatInverseTranspose;	// model-view inverse transpose matrix (object -> viewer skewed)
	mat4 modelViewProjectionMat;		// model-view-projection matrix (object -> clip)
	mat4 atlasMat;						// atlas matrix (texture -> cell)
};

uniform ubTransformStack
{
	sModelMatrixStack uModelMatrixStack[MAX_OBJECTS];
};

uniform int uIndex;
uniform float uTime;

out vbVertexData {
	mat4 vTangentBasis_view;
	vec4 vTexcoord_atlas;
};

flat out int vVertexID;
flat out int vInstanceID;

void main()
{
	// Morph target index and interp parameter
	int index = int(uTime);
	float param = uTime - index;

	// Current morph targets
	sMorp p0 = aMorp[index];
	sMorp p1 = aMorp[++index % 5];

	// Interpolate between morph targets based on interp param and calc bitangent
	vec3 position = mix(p0.position, p1.position, param);
	vec3 normal = mix(p0.normal, p1.normal, param);
	vec3 tangent = mix(p0.tangent, p1.tangent, param);
	vec3 bitangent = cross(normal, tangent);

	
	sModelMatrixStack t = uModelMatrixStack[uIndex];
	
	// Construct TBN matrix
	vTangentBasis_view = t.modelViewMatInverseTranspose * mat4(tangent, 0.0, bitangent, 0.0, normal, 0.0, vec4(0.0));

	// Transform position and texcoord then pass them through
	vTangentBasis_view[3] = t.modelViewMat * vec4(position, 1.0);
	gl_Position = t.modelViewProjectionMat * vec4(position, 1.0);
	vTexcoord_atlas = t.atlasMat * vec4(aTexcoord, 0.0.xx);
	vVertexID = gl_VertexID;
	vInstanceID = gl_InstanceID;
}
