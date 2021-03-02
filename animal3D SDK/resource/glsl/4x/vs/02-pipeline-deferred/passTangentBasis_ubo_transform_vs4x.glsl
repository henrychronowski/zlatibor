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
	
	passTangentBasis_ubo_transform_vs4x.glsl
	Calculate and pass tangent basis using uniform buffers.
*/

#version 450

#define MAX_OBJECTS 128

// ****TO-DO:
//	-> declare attributes related to lighting
//		(hint: normal [2], texcoord [8], tangent [10], bitangent [11])
//	-> declare view-space varyings related to lighting
//		(hint: one per attribute)
//	-> calculate final clip-space position and view-space varyings
//		(hint: complete tangent basis [TBNP] transformed to view-space)
//		(hint: texcoord transformed to atlas coordinates in a similar fashion)

layout (location = 0) in vec4 aPosition;
layout (location = 2) in vec3 aNormal;
layout (location = 8) in vec4 aTexcoord;

out vec4 vPosition;
out vec4 vNormal;
out vec4 vTexcoord;

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

flat out int vVertexID;
flat out int vInstanceID;

void main()
{
	gl_Position = uModelMatrixStack[uIndex].modelViewProjectionMat * aPosition;

	//transform things to common space
	vPosition = uModelMatrixStack[uIndex].modelViewMat * aPosition;
	vNormal = uModelMatrixStack[uIndex].modelViewMatInverseTranspose * vec4(aNormal, 0.0);
	vTexcoord = uModelMatrixStack[uIndex].atlasMat * aTexcoord;

	vVertexID = gl_VertexID;
	vInstanceID = gl_InstanceID;
}
