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
	
	passColor_interp_tes4x.glsl
	Pass color, outputting result of interpolation.
*/

#version 450

// ****TO-DO: 
//	-> declare uniform block for spline waypoint and handle data
//	-> implement spline interpolation algorithm based on scene object's path
//	-> interpolate along curve using correct inputs and project result

layout (isolines, equal_spacing) in;

uniform ubCurve
{
	vec4 uCurveWaypoint[32];
	vec4 uCurveTangent[32];
};

uniform int uCount;

uniform mat4 uP;

out vec4 vColor;

void main()
{
	int i0 = gl_PrimitiveID; //Current Patch
	int i1 = (i0 + 1) % uCount;

	float t = gl_TessCoord.x;

	vec4 point = mix(uCurveWaypoint[i0], uCurveWaypoint[i1], t);

	//vec4 point = vec4(gl_TessCoord.xy, -1.0, 1.0);

	gl_Position = uP *  point;


	vColor = vec4(0.5, 0.5, t, 1.0);
}
