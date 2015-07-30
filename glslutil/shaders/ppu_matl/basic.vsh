#version 420 core

// basic.vsh: A vertex shader that hands information off to a fragment shader
//            that implements the Phong lighting model on a per-fragment basis.

// Per-vertex attributes
layout (location = 0) in vec3 mcPosition; // position in model coordinates
in vec3 mcNormal; // normal vector in model coordinates
in vec2 texCoords; // (s,t)

// Output to fragment shader:
out vec3 ecPositionToFS;
out vec3 ecNormalToFS;
out vec2 texCoordsToFS; // (s,t)

// 2. Transformation
uniform mat4 mc_ec, ec_lds;

void main (void)
{
	// convert current vertex and its associated normal to eye coordinates
	// ("p_" prefix emphasizes it is stored in projective space)
	vec4 p_ecPosition = mc_ec * vec4(mcPosition, 1.0);
	ecPositionToFS = p_ecPosition.xyz;
	mat3 normalMatrix = transpose( inverse( mat3x3(mc_ec) ) );
	ecNormalToFS = normalize(normalMatrix * mcNormal);
	texCoordsToFS = texCoords;

	// need to compute projection coordinates for given point
	gl_Position = ec_lds * p_ecPosition;
}
