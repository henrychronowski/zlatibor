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

// ****DONE: 
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
	int i0 = gl_PrimitiveID; // Current Patch
	int i1 = (i0 + 1) % uCount; // Next Patch
	int i2 = (i0 - 1) % uCount; // Previous Patch
	int i3 = (i1 + 1) % uCount; // Patch After Next

	float t = gl_TessCoord.x;
	float t2 = t * t;

	vec4 p0 = uCurveWaypoint[i2]; // Previous point
	vec4 p1 = uCurveWaypoint[i0]; // Start point
	vec4 p2 = uCurveWaypoint[i1]; // Next point 
	vec4 p3 = uCurveWaypoint[i3]; // Point after next

	//Cubic Interpolation: http://paulbourke.net/miscellaneous/interpolation/
	vec4 a0 = -0.5 * p0 + 1.5 * p1 - 1.5* p2 + 0.5 * p3;
    vec4 a1 = p0 - 2.5 * p1 + 2 * p2 - 0.5 * p3;
    vec4 a2 = -0.5 * p0 + 0.5 * p2;
    vec4 a3 = p1;

	vec4 point = a0 * t * t2 + a1 * t2 + a2 * t + a3;

	//Project result
	gl_Position = uP *  point;

	vColor = vec4(0.5, 0.5, t, 1.0);
}
