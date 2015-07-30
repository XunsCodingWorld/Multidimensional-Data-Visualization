#version 420 core

// basic.vsh: A vertex shader that hands information off to a fragment shader
//            that implements the Phong lighting model on a per-fragment basis.

// Per-vertex attributes
layout (location = 0) in vec3 mcPosition; // position in model coordinates
in vec3 mcNormal; // normal vector in model coordinates
in vec2 texCoords; // (s,t)

in vec3 ka, kd, ks;
in float shininess, alpha;

// Output to fragment shader:
out vec3 ecPositionToFS;
out vec3 ecNormalToFS;
out vec2 texCoordsToFS; // (s,t)

out PVA
{
	vec3 ka;
	vec3 kd;
	vec3 ks;
	float shininess;
	float alpha;
} pvaOut;

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

	pvaOut.ka = ka;
	pvaOut.kd = kd;
	pvaOut.ks = ks;
	pvaOut.shininess = shininess;
	pvaOut.alpha = alpha;

	// need to compute projection coordinates for given point
	gl_Position = ec_lds * p_ecPosition;
}
