#version 420 core

// The following input "layout" indicates that each invocation
// will be given a single point whose coordinates will be stored
// in: "gl_in[0].gl_Position". (gl_in is a predefined variable.)
// Note that it is illegal in this case to access any position
// of gl_in other than [0].
layout ( points ) in;

// The following output "layout" says that we will produce one
// or more triangle_strip primitives on each invocation. In
// other words, this shader turns each input point into one or
// more triangle strip instances.
layout ( triangle_strip, max_vertices = 200 ) out;

// Assumption: incoming coordinates are in LDS space.

in PVA
{
	vec4 pvaSet1;
	vec4 pvaSet2;
} pva_in[]; // Only position [0] is available as noted above.

out PVA
{
	vec4 pvaSet1;
	vec4 pvaSet2;
} pva_out;

// Following makes sure the shapes don't change based on
// differing window and viewport aspect ratios. One of the
// following will be 1.0; the other will be < 1.0. The idea
// is that if we make a square window taller, we need to
// shrink yFactor; similarly, if we make a square window
// wider, we need to shrink xFactor.
uniform float xFactor = 1.0, yFactor = 1.0;

const int CROSS = 1;
const int HOURGLASS = 2;
const int CIRCLE = 3;
const int STAR = 4;

void computeHalfSizes(in float size, out float hsx, out float hsy)
{
	hsx = 0.5 * xFactor * size;
	hsy = 0.5 * yFactor * size;
}

void drawCircle(float size) // CENTERED on gl_in[0]
{
	float PI = 3.1415926;
	float delta = 2 * PI / 24;
	for(float theta = 0.0; theta < 2*PI; theta += delta)	
	{
		gl_Position = vec4(gl_in[0].gl_Position.x + cos(theta)*size*0.5,
			   	   gl_in[0].gl_Position.y + sin(theta)*size*0.5,
			    	   gl_in[0].gl_Position.z, 1.0);
		EmitVertex();
		gl_Position = vec4(gl_in[0].gl_Position.x,
			   	   gl_in[0].gl_Position.y,
			      	   gl_in[0].gl_Position.z, 1.0);
		EmitVertex();
		gl_Position = vec4(gl_in[0].gl_Position.x + cos(theta+delta)*size*0.5,
			   	   gl_in[0].gl_Position.y + sin(theta+delta)*size*0.5,
			    	   gl_in[0].gl_Position.z, 1.0);
		EmitVertex();
		EndPrimitive();
	}
}

void drawCross(float size) // CENTERED on gl_in[0]
{
	float hsx, hsy;
	computeHalfSizes(size, hsx, hsy);
	gl_Position = vec4(gl_in[0].gl_Position.x - hsx,
			   gl_in[0].gl_Position.y - 0.25 * hsy,
			   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	gl_Position = vec4(gl_in[0].gl_Position.x - hsx,
			   gl_in[0].gl_Position.y + 0.25 * hsy,
			   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	gl_Position = vec4(gl_in[0].gl_Position.x + hsx,
			   gl_in[0].gl_Position.y - 0.25 * hsy,
			   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	gl_Position = vec4(gl_in[0].gl_Position.x + hsx,
			   gl_in[0].gl_Position.y + 0.25 * hsy,
			   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	EndPrimitive();

	gl_Position = vec4(gl_in[0].gl_Position.x - 0.25 * hsx,
			   gl_in[0].gl_Position.y - hsy,
			   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	gl_Position = vec4(gl_in[0].gl_Position.x - 0.25 * hsx,
			   gl_in[0].gl_Position.y + hsy,
			   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	gl_Position = vec4(gl_in[0].gl_Position.x + 0.25 * hsx,
			   gl_in[0].gl_Position.y - hsy,
			   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	gl_Position = vec4(gl_in[0].gl_Position.x + 0.25 * hsx,
			   gl_in[0].gl_Position.y + hsy,
			   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	EndPrimitive();
}

void drawStar(float size) // CENTERED on gl_in[0]
{
	float hsx, hsy;
	computeHalfSizes(size, hsx, hsy);
	// Draw two triangles; on pointing "up", one "down"
	float offset = 0.7 * (sqrt(3.0) - 1.0) * hsy;
	gl_Position = vec4(gl_in[0].gl_Position.x - hsx,
			   gl_in[0].gl_Position.y + offset,
		     	   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	gl_Position = vec4(gl_in[0].gl_Position.x,
			   gl_in[0].gl_Position.y - hsy,
			   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	gl_Position = vec4(gl_in[0].gl_Position.x + hsx,
			   gl_in[0].gl_Position.y + offset,
			   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	EndPrimitive();

	gl_Position = vec4(gl_in[0].gl_Position.x - hsx,
			   gl_in[0].gl_Position.y - offset,
			   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	gl_Position = vec4(gl_in[0].gl_Position.x,
			   gl_in[0].gl_Position.y + hsy,
			   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	gl_Position = vec4(gl_in[0].gl_Position.x + hsx,
			   gl_in[0].gl_Position.y - offset,
			   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	EndPrimitive();
}

void drawHourglass(float size) // CENTERED on gl_in[0]
{
	float hsx, hsy;
	computeHalfSizes(size, hsx, hsy);
	float alpha = sqrt(3.0)/3.0 * hsy;
	float beta = alpha + alpha;
	gl_Position = vec4(gl_in[0].gl_Position.x - hsx,
			   gl_in[0].gl_Position.y - (alpha + beta),
			   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	gl_Position = vec4(gl_in[0].gl_Position.x,
			   gl_in[0].gl_Position.y,
		  	   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	gl_Position = vec4(gl_in[0].gl_Position.x + hsx,
			   gl_in[0].gl_Position.y - (alpha + beta),
			   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	EndPrimitive();
	gl_Position = vec4(gl_in[0].gl_Position.x - hsx,
			   gl_in[0].gl_Position.y + (alpha + beta),
			   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	gl_Position = vec4(gl_in[0].gl_Position.x,
			   gl_in[0].gl_Position.y,
		  	   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	gl_Position = vec4(gl_in[0].gl_Position.x + hsx,
			   gl_in[0].gl_Position.y + (alpha + beta),
			   gl_in[0].gl_Position.z, 1.0);
	EmitVertex();
	EndPrimitive();
}


void drawShape(int shape, float size)
{
	if (shape == CROSS)
		drawCross(size);
	else if (shape == HOURGLASS)
		drawHourglass(size);
	else if (shape == CIRCLE)
		drawCircle(size);
	else if (shape == STAR)
		drawStar(size);
}

uniform float attrToCutForCross, attrToCutForCircle, attrToCutForHourglass;

int getShape(float attr)
{
	if (attr < attrToCutForCross)
		return CROSS;
	if (attr < attrToCutForCircle)
		return CIRCLE;
	if (attr < attrToCutForHourglass)
		return HOURGLASS;
	return STAR;
}

uniform float sizeFactor;

float getSize(float attr)
{
	return sizeFactor * attr;
}

uniform int attrToUseForShape, attrToUseForSize;

void main()
{
	// pass all incoming attributes to fragment shader in
	// case it wants to use any when setting color
	pva_out.pvaSet1 = pva_in[0].pvaSet1;
	pva_out.pvaSet2 = pva_in[0].pvaSet2;

	// TODO: make attrToUseForShape and attrToUseForSize
	//       be uniforms.
	//int attrToUseForShape = 0;
	//int attrToUseForSize = 1;
	int shape;
	if (attrToUseForShape < 4)
		shape = getShape(pva_in[0].pvaSet1[attrToUseForShape]);
	else
		shape = getShape(pva_in[0].pvaSet2[attrToUseForShape - 4]);
	float size;
	if (attrToUseForSize < 4)
		size = getSize(pva_in[0].pvaSet1[attrToUseForSize]);
	else
		size = getSize(pva_in[0].pvaSet2[attrToUseForSize - 4]);
	drawShape(shape, size);
}
