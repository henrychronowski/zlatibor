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
	
	drawTangentBases_gs4x.glsl
	Draw tangent bases of vertices and/or faces, and/or wireframe shapes, 
		determined by flag passed to program.
*/

#version 450

// ****D0N3: 
//	-> declare varying data to read from vertex shader											ü
//		(hint: it's an array this time, one per vertex in primitive) 
//	-> use vertex data to generate lines that highlight the input triangle						ü
//		-> wireframe: one at each corner, then one more at the first corner to close the loop	ü
//		-> vertex tangents: for each corner, new vertex at corner and another extending away	ü
//			from it in the direction of each basis (tangent, bitangent, normal)
//		-> face tangents: ditto but at the center of the face; need to calculate new bases		ü
//	-> call "EmitVertex" whenever you're done with a vertex										ü
//		(hint: every vertex needs gl_Position set)
//	-> call "EndPrimitive" to finish a new line and restart										ü
//	-> experiment with different geometry effects												ü

// (2 verts/axis * 3 axes/basis * (3 vertex bases + 1 face basis) + 4 to 8 wireframe verts = 28 to 32 verts)
#define MAX_VERTICES 32

layout (triangles) in; // Recieving triangles
// gl_in[3]

layout (line_strip, max_vertices = MAX_VERTICES) out; // Outputting line strip

in vbVertexData {
	mat4 vTangentBasis_view;
	vec4 vTexcoord_atlas;
} vVertexData[];

out vec4 vColor;

uniform mat4 uP;
uniform vec4 uColor0;
uniform float uSize;
uniform int uFlag;

// Draws a line given the two endpoints and a colour
void drawLine(in vec4 start, in vec4 end, in vec4 col = vec4(1.0))
{
	vColor = col;
	gl_Position = start;
	EmitVertex();
	gl_Position = end;
	EmitVertex();
	EndPrimitive();
}

// Draws a basis vector with the uniform given scale
void drawBasisVector(in vec4 start, in vec4 dir, in vec4 col = vec4(1.0))
{
	// Call drawline with a normalized scaled dir in order to make it look like the demo
	drawLine(start, start + normalize(dir) * uSize, col);
}

// Returns the face point in the centre of the triangle by averaging the vertices
vec4 getFacePoint()
{
	return vec4((gl_in[1].gl_Position.xyzw + gl_in[2].gl_Position.xyzw + gl_in[0].gl_Position.xyzw) * 0.33333);
}

// Referencing lecture 10
mat4 getFaceTBN()
{
	mat4 result;
	vec3 del1 = (gl_in[1].gl_Position - gl_in[0].gl_Position).xyz;
	vec3 del2 = (gl_in[2].gl_Position - gl_in[0].gl_Position).xyz;
	vec2 delU = vec2(del1.x - gl_in[0].gl_Position.x, del2.x - gl_in[0].gl_Position.x); //vec2(del1.x, del2.x);
	vec2 delV = vec2(del1.y - gl_in[0].gl_Position.y, del2.y - gl_in[0].gl_Position.y); //vec2(del1.y, del2.y);
	mat2x3 delP = mat2x3(del1, del2);
	mat2 delUV = mat2(vec2(delU.x, delV.x), vec2(delU.y, delV.y));

	vec3 normFace = normalize(cross(del1, del2));
	mat2x3 tb = delP * inverse(delUV);

	


	return uP * mat4(vec4(tb[0], 0.0), vec4(tb[1], 0.0), vec4(normFace, 0.0), vec4(0.0.xxx, 1.0));
}

// Draws a wire frame of the triangle formed by the given vertices
void drawWireframe()
{
	// Draw segments
	drawLine(gl_in[0].gl_Position, gl_in[1].gl_Position, uColor0);
	drawLine(gl_in[1].gl_Position, gl_in[2].gl_Position, uColor0);
	drawLine(gl_in[2].gl_Position, gl_in[0].gl_Position, uColor0);
};

// Draws a tangent basis at each given vertex and the face point of the given triangle
void drawTangentBases()
{
	// Draw tangent bases on the vertices
	drawBasisVector(gl_in[0].gl_Position, uP * vVertexData[0].vTangentBasis_view[0], vec4(1.0, 0.0f, 0.0f, 1.0f));
	drawBasisVector(gl_in[0].gl_Position, uP * vVertexData[0].vTangentBasis_view[1], vec4(0.0, 1.0f, 0.0f, 1.0f));
	drawBasisVector(gl_in[0].gl_Position, uP * vVertexData[0].vTangentBasis_view[2], vec4(0.0, 0.0f, 1.0f, 1.0f));

	drawBasisVector(gl_in[1].gl_Position, uP * vVertexData[1].vTangentBasis_view[0], vec4(1.0, 0.0f, 0.0f, 1.0f));
	drawBasisVector(gl_in[1].gl_Position, uP * vVertexData[1].vTangentBasis_view[1], vec4(0.0, 1.0f, 0.0f, 1.0f));
	drawBasisVector(gl_in[1].gl_Position, uP * vVertexData[1].vTangentBasis_view[2], vec4(0.0, 0.0f, 1.0f, 1.0f));

	drawBasisVector(gl_in[2].gl_Position, uP * vVertexData[2].vTangentBasis_view[0], vec4(1.0, 0.0f, 0.0f, 1.0f));
	drawBasisVector(gl_in[2].gl_Position, uP * vVertexData[2].vTangentBasis_view[1], vec4(0.0, 1.0f, 0.0f, 1.0f));
	drawBasisVector(gl_in[2].gl_Position, uP * vVertexData[2].vTangentBasis_view[2], vec4(0.0, 0.0f, 1.0f, 1.0f));
			 
	// Draw tangent bases on the face points
//	drawBasisVector(getFacePoint(), uP * vVertexData[0].vTangentBasis_view[0], vec4(1.0, 0.0f, 0.0f, 1.0f));
//	drawBasisVector(getFacePoint(), uP * vVertexData[0].vTangentBasis_view[1], vec4(0.0, 1.0f, 0.0f, 1.0f));
//	drawBasisVector(getFacePoint(), uP * vVertexData[0].vTangentBasis_view[2], vec4(0.0, 0.0f, 1.0f, 1.0f));
	drawBasisVector(getFacePoint(), getFaceTBN()[0], vec4(1.0, 0.0f, 0.0f, 1.0f));
	drawBasisVector(getFacePoint(), getFaceTBN()[1], vec4(0.0, 1.0f, 0.0f, 1.0f));
	drawBasisVector(getFacePoint(), getFaceTBN()[2], vec4(0.0, 0.0f, 1.0f, 1.0f));
}

void main()
{
	// Draw the proper geometries given the uniform bitflag
	if(bitfieldExtract(uFlag, 2, 1) != 0)
		drawWireframe();

	if(bitfieldExtract(uFlag, 1, 1) != 0)
		drawTangentBases();
}
