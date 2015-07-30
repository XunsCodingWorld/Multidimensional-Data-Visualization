#version 420 core

// PointsMV.vsh: A vertex shader that handles 3D viewing and passes
//               some PVAs to the next shader stage.

// Per-vertex attributes
layout (location = 0) in vec3 mcPosition; // position in model coordinates
in vec4 pvaSet1, pvaSet2;

// Output:
out PVA
{
	vec4 pvaSet1;
	vec4 pvaSet2;
} pva_out;

// 2. Transformation
uniform mat4 mc_ec, ec_lds;

void main (void)
{
	// convert current vertex and its associated normal to eye coordinates
	// ("p_" prefix emphasizes it is stored in projective space)
	vec4 p_ecPosition = mc_ec * vec4(mcPosition, 1.0);

	// Pass on PVAs used to set shapes, sizes, colors, etc.
	pva_out.pvaSet1 = pvaSet1;
	pva_out.pvaSet2 = pvaSet2;

	// need to compute projection coordinates for given point
	gl_Position = ec_lds * p_ecPosition;
}
