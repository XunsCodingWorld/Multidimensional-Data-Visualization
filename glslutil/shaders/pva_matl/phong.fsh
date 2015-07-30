#version 420 core

// phong.fsh - an implementation of the Phong model for a fragment shader

// Per-vertex attributes from vertex shader
in vec3 ecPositionToFS; // position in model coordinates
in vec3 ecNormalToFS; // normal vector in model coordinates

in PVA
{
    vec3 ka;
    vec3 kd;
    vec3 ks;
    float shininess;
    float alpha;
} pvaIn;

// Output:
out vec4 fragColor;

// =============== BEGIN: PHONG LIGHTING MODEL ==============================

// Per-primitive attributes
// 1. Light sources
const int MAX_NUM_LIGHTS = 3;
uniform int actualNumLights = 0;
// Light source positions assumed to be given in eye coordinates:
uniform vec4 ecLightPosition[MAX_NUM_LIGHTS];
uniform vec3 lightStrength[MAX_NUM_LIGHTS]; // (r,g,b) strength
uniform vec3 globalAmbient = vec3(0.1, 0.1, 0.1);
// 2. Transformation
uniform mat4 mc_ec, ec_lds;

uniform int lmFlags = 2047; // all lights and lighting model pieces enabled.

float compute_liHat(in int i, in vec3 Q, out vec3 liHat)
{
	if (ecLightPosition[i].w > 0.0)
	{
		vec3 li = ecLightPosition[i].xyz - Q;
		// Compute distance between surface and light position (in case we add
		// attenuation later)
		float d = length(li);
		// Normalize the vector from surface to light position
		liHat = li/d;
		return d; // distance from point light to point on surface
	}
	// directional light - no distance attenuation
	liHat = normalize(ecLightPosition[i].xyz);
	return 0.0;
}

void handleLight(in int i, in vec3 liHat, in float distToLight, in vec3 nHat,
	in vec3 Q, in bool isParallel, in vec3 pDirHat,
	inout vec3 Diffuse, inout vec3 Specular)
{
	// All input to this function is assumed to be in eye coordinates. The
	// function operates completely in eye coordinates.

	// 1. COMPUTE UNIT VECTOR FROM SURFACE POINT TO EYE.
	// If this is a parallel projection, the eye is at infinity, and we use
	// the projection direction for vHat. Otherwise the vector to the eye is
	// E-Q=(0,0,0)-Q.
	vec3 vHat;
	if (isParallel)
		vHat = pDirHat;
	else
		vHat = -normalize(Q);

	// 2. DETERMINE IF SURFACE NORMAL VECTOR NEEDS TO BE FLIPPED.
	vec3 use_nHat = nHat;
	if (dot(vHat,nHat) < 0.0)
		// we are looking at this surface from behind; flip the normal
		use_nHat = -use_nHat;

	// For diffuse term:
	float nHatDotLiHat = dot(use_nHat, liHat);

	// if translucent, light can shine through
	if ((pvaIn.alpha < 1.0) && (nHatDotLiHat < 0.0))
		nHatDotLiHat = -(1.0 - pvaIn.alpha)*nHatDotLiHat;

	if (nHatDotLiHat > 0.0) // Light source on correct side of surface, so:
	{
		Diffuse  += lightStrength[i] * nHatDotLiHat;
		vec3 riHat = 2.0*nHatDotLiHat*use_nHat - liHat;
		float riHatDotVHat = dot(riHat,vHat);
		if (riHatDotVHat > 0.0) // Viewer on correct side of normal vector, so:
			Specular += lightStrength[i] * pow(riHatDotVHat, pvaIn.shininess);
	}
}

vec3 phong(in vec3 ec_Q, in vec3 ec_nHat, in bool isParallel, in vec3 pDirHat)
{
	// "nHat" has unit length on entry, and it is given in eye coordinates.

	// Declare and initialize the light intensity accumulators
	vec3 Diffuse  = vec3 (0.0);
	vec3 Specular = vec3 (0.0);
	vec3 liHat;

	for (int lsi=0 ; lsi<actualNumLights ; lsi++)
	{
		if ((lmFlags & (1 << lsi)) != 0)
		{
			float dist = compute_liHat(lsi, ec_Q, liHat);
			handleLight(lsi, liHat, dist, ec_nHat, ec_Q, isParallel, pDirHat, Diffuse, Specular);
		}
	}

	vec3 color = vec3(0.0);
	int materialFlags = lmFlags / 256;
	if ((materialFlags%2) != 0)
		color += globalAmbient * pvaIn.ka;
	if (((materialFlags/2)%2) != 0)
		color += Diffuse  * pvaIn.kd;
	if (((materialFlags/4) % 2) != 0)
		color += Specular * pvaIn.ks;
	return clamp( color, 0.0, 1.0 );
}

bool parallelProjection(in mat4 M, out vec3 pDir)
{
	// GLSL matrices are actually transposed from the way we
	// normally think of them. Hence testing for the bottom row
	// (i.e., row 3) starting with (0,0,0), we need to check for
	// column 3 starting with (0,0,0):
	if ((M[0][3] == 0.0) && (M[1][3] == 0.0) && (M[2][3] == 0.0))
	{
		pDir[0] = -M[2][0]/M[0][0];
		pDir[1] = -M[2][1]/M[1][1];
		pDir[2] = 1.0;
		return true;
	}
	return false;
}

// ================= END: PHONG LIGHTING MODEL ==============================
// =============== BEGIN: TEXTURE CODE ===================================

in vec2 texCoordsToFS; // (s,t)

// Modals describing how color is to be generated

// 1. colorGenerationMode:
//      0: just baseColor
//
//     10: just texture color
//
//     20: baseColor * texture color
//     21: baseColor + texture color
//     22: baseColor - texture color
//     23: texture color - baseColor
//     24: baseColor / texture color
//     25: texture color / baseColor

uniform int colorGenerationMode = 0;

// 2. If "colorGenerationMode" includes texture:
//
// 2.1 textureSource (Where does the texture come from):
//      0: a sampler2D (see 2.2 below)
//      OR here are a few example procedural textures:
//      1: checkerboard, no blend
//      2: checkerboard, blend
//      3: mandelbrot set
//      4: julia set
//      OR add your own!!!
uniform int textureSource = -1; // initialized to unknown source

// 2.2 There may be an input texture map:
uniform sampler2D textureMap;

// 2.3 There may be a texture color multiplier:
uniform int useTextureColorMultiplier = 0; // 0: no; 1: yes
uniform vec4 textureColorMultiplier = vec4(1.0, 1.0, 1.0, 1.0);

// END: Modals describing how color is to be generated

// parameters for checkerboard.
// Exercise for those interested: Modify the code so that these are imported
// as uniform variables.

const vec4 requestedColorOdd = vec4(1.0, 0.0, 0.0, 1.0), requestedColorEven = vec4(0.0, 1.0, 1.0, 1.0);
const vec4 lightGray = vec4(0.7, 0.7, 0.7, 1.0), darkGray = vec4(0.2, 0.2, 0.2, 1.0);

// END: parameters for checkerboard

vec4 checkerboard(int blendAcrossRow, int blendAcrossCol, int numRowsCols, vec4 colorOdd, vec4 colorEven)
{
	float ldsX = texCoordsToFS.s;
	float ldsY = texCoordsToFS.t;
	float row0To1 = 0.5 * (ldsY + 1.0);
	float col0To1 = 0.5 * (ldsX + 1.0);
	vec4 fColor;
	if (numRowsCols <= 0)
	{
		if ((1.0-row0To1) < col0To1)
			fColor = vec4(0.5, 0.5, 0.7, 1.0);
		else
			fColor = vec4(0.7, 0.5, 0.5, 1.0);
	}
	else
	{
		// The (row, col) coming in are in a 0..1 range. We want to scale:
		float r = row0To1 * numRowsCols;
		float c = col0To1 * numRowsCols;
		float r0or1 = mod(floor(r), 2.0);
		float c0or1 = mod(floor(c), 2.0);
		bool odd = ((r0or1 + c0or1) == 1.0);
		
		if ((blendAcrossRow == 0) && (blendAcrossCol == 0))
		{
			if (odd)
				fColor = colorOdd;
			else
				fColor = colorEven;
		}
		else if ((blendAcrossRow == 0) && (blendAcrossCol == 1))
		{
			float colFract = fract(c);
			if (odd)
				fColor = mix(colorOdd, colorEven, colFract);
			else
				fColor = mix(colorEven, colorOdd, colFract);
		}
		else if ((blendAcrossCol == 0) && (blendAcrossRow == 1))
		{
			float rowFract = fract(r);
			if (odd)
				fColor = mix(colorOdd, colorEven, rowFract);
			else
				fColor = mix(colorEven, colorOdd, rowFract);
		}
		else // blending across the row AND the column
		{
			float colFract = fract(c);
			vec4 c1;
			if (odd)
				c1 = mix(colorOdd, colorEven, colFract);
			else
				c1 = mix(colorEven, colorOdd, colFract);
			float rowFract = fract(r);
			vec4 c2;
			if (odd)
				c2 = mix(colorOdd, colorEven, rowFract);
			else
				c2 = mix(colorEven, colorOdd, rowFract);
			fColor = mix(c1, c2, 0.5);
		}
	}
	return fColor;
}
//    A fragment shader function drawing Mandlebrot and Julia sets.
//    Source adapted from "The Orange Book" example authored by Dave Baldwin,
//    Steve Koren, and Randi Rost (which they say was based on a shader by
//    Michael Rivero).

// parameters. Exercise: import these as uniform variables.
const int   MaxIterations = 50;
const float MaxRSquared = 4.0;
const vec3 InnerColor = vec3(0.0, 0.0, 0.0);
const vec3 OuterColor1 = vec3(1.0, 0.0, 0.0);
const vec3 OuterColor2 = vec3(1.0, 1.0, 0.0);

const float Jreal = -0.765;
const float Jimag = 0.11;
// END: parameters

vec4 colorFrom(float real, float imag, float Creal, float Cimag)
{
	float rSquared = 0.0;
	
	int iter;
	for (iter=0; (iter < MaxIterations) && (rSquared < MaxRSquared) ; iter++)
	{
		float tempreal = real;
		real = tempreal*tempreal - imag*imag + Creal;
		imag = 2.0*tempreal*imag + Cimag;
		rSquared = real*real + imag*imag;
	}
	
	// Base the color on the number of iterations
	
	vec3 color;
	if (rSquared < MaxRSquared)
		color = InnerColor;
	else
		color = mix(OuterColor1, OuterColor2, fract(float(iter)/float(MaxIterations)));//*0.05));
	return vec4(color, 1.0);
}

vec4 computeMandelOrJulia(int MandelOrJulia)
{
	float real = 2.5*texCoordsToFS.s - 1.95;
	float imag = 2.5*texCoordsToFS.t - 1.25;
	
	// Not all GLSL implementations support conditional returns, so we
	// conditionally assign to "theColor" and unconditionally return it.
	vec4 theColor;
	if (MandelOrJulia == 0)
		// Mandelbrot set
		theColor = colorFrom(real,imag,real,imag);
	else
		// Julia set
		theColor = colorFrom(real,imag,Jreal,Jimag);
	
	return theColor;
}
void divide(in float a, in float b, out float r)
{
	if (a <= 0.0)
		r = 0.0;
	else if (b <= 0.0)
		r = 1.0;
	else
		r = a / b;
}

void divide(in vec4 cA, in vec4 cB, out vec4 cR)
{
	divide(cA.r, cB.r, cR.r);
	divide(cA.g, cB.g, cR.g);
	divide(cA.b, cB.b, cR.b);
	divide(cA.a, cB.a, cR.a);
}

vec4 composeColor(vec4 baseColor) 
{
	vec4 retColor = vec4(0.0, 0.0, 0.0, 1.0);
	if (colorGenerationMode == 0)
		retColor = baseColor;
	else 
	{
		// Need texture; let's get the texture color, then we'll figure out
		// what to do with it.
		vec4 tColor;
		if (textureSource == -1)
			// internal signal that either a sampler-based texture was requested,
			// but it is missing, OR no texture source has been specified (see
			// initialization above)
			tColor = checkerboard(0, 0, 50, lightGray, darkGray);
		else if (textureSource == 0)
		{
			tColor = texture(textureMap, texCoordsToFS);
			if (useTextureColorMultiplier == 1)
			{
				if (tColor[3] == 0) // accommodation to make text work correctly
					discard;
			}
		}
		else if (textureSource == 1)
			tColor = checkerboard(0, 0, 10, requestedColorOdd, requestedColorEven);
		else if (textureSource == 2)
			tColor = checkerboard(1, 1, 10, requestedColorOdd, requestedColorEven);
		else if (textureSource == 3)
			tColor = computeMandelOrJulia(0);
		else if (textureSource == 4)
			tColor = computeMandelOrJulia(1);
		else // UNKNOWN SOURCE!!!
			tColor = vec4(1.0, 0.7, 0.8, 1.0);

		if (useTextureColorMultiplier == 1)
			tColor *= textureColorMultiplier;

		switch (colorGenerationMode)
		{
			case 10:
				retColor = tColor;
				break;
			case 20:
				retColor = baseColor * tColor;
				break;
			case 21:
				retColor = baseColor + tColor;
				break;
			case 22:
				retColor = baseColor - tColor;
				break;
			case 23:
				retColor = tColor - baseColor;
				break;
			case 24:
				divide(baseColor, tColor, retColor);
				break;
			case 25:
				divide(tColor, baseColor, retColor);
			default:
				retColor = vec4(1.0, 0.6, 1.0, 1.0);
		}
	}
	return clamp(retColor, 0.0, 1.0);
}
// ================= END: TEXTURE CODE ===================================

void main (void)
{
	vec3 pDir;
	bool isParallel = parallelProjection(ec_lds, pDir);
	if (isParallel)
		pDir = normalize(pDir);
	vec4 f = vec4(phong(ecPositionToFS, ecNormalToFS, isParallel, pDir), pvaIn.alpha);
	fragColor = composeColor(f);
}
