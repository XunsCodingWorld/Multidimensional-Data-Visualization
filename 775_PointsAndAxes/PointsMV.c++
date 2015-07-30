// PointsMV.c++

#include <iostream>
#include "PointsMV.h"
#include "ShaderIF.h"

typedef float vec3[3];
GLuint* vbo;

ShaderIF* PointsMV::shaderIF = NULL;
int PointsMV::numInstances = 0;
GLuint PointsMV::shaderProgram = 0;
GLint PointsMV::pvaLoc_mcPosition = -1;
GLint PointsMV::pvaLoc_pvaSet1 = -1;
GLint PointsMV::pvaLoc_pvaSet2 = -1;
GLint PointsMV::ppuLoc_color = -1;
GLint PointsMV::ppuLoc_mc_ec = -1;
GLint PointsMV::ppuLoc_ec_lds = -1;
GLint PointsMV::ppuLoc_xFactor = -1;
GLint PointsMV::ppuLoc_yFactor = -1;
GLint PointsMV::ppuLoc_sizeFactor = -1;
GLint PointsMV::ppuLoc_attrToCutForCross = -1;
GLint PointsMV::ppuLoc_attrToCutForHourglass = -1;
GLint PointsMV::ppuLoc_attrToCutForCircle = -1;
GLint PointsMV::ppuLoc_attrToUseForShape = -1;
GLint PointsMV::ppuLoc_attrToUseForSize = -1;
GLint PointsMV::ppuLoc_attrToUseForColor = -1;
GLint PointsMV::ppuLoc_attrToCutForRed = -1;
GLint PointsMV::ppuLoc_attrToCutForGreen = -1;

static ShaderIF::ShaderSpec glslProg[] =
	{
		{ "PointsMV.vsh", GL_VERTEX_SHADER },
		{ "PointsToShapes.gsh", GL_GEOMETRY_SHADER },
		{ "PointsMV.fsh", GL_FRAGMENT_SHADER }
	};

PointsMV::PointsMV(const cryph::AffPoint* pts, float* sps, float* sz, float* crs, int nPointsIn, GLenum modeIn) :
	nPoints(nPointsIn), mode(modeIn)
{
	if (PointsMV::shaderProgram == 0)
	{
		// create the shader program:
		PointsMV::shaderIF = new ShaderIF(glslProg, 3);
		PointsMV::shaderProgram = shaderIF->getShaderPgmID();
		fetchGLSLVariableLocations();
	}

	// Now do instance-specific initialization:
	defineModel(pts, sps, sz, crs);
	PointsMV::numInstances++;
}

PointsMV::~PointsMV()
{
	glDeleteBuffers(3, vertexBuffer);
	glDeleteVertexArrays(1, vao);
	if (--PointsMV::numInstances == 0)
	{
		PointsMV::shaderIF->destroy();
		delete PointsMV::shaderIF;
		PointsMV::shaderIF = NULL;
		PointsMV::shaderProgram = 0;
	}
	delete [] vbo;
}

void PointsMV::defineModel(const cryph::AffPoint* pts, float* sps, float* sz, float* crs)
{
	typedef float vec3[3];
	typedef float vec4[4];

	float* points = new float[3*nPoints];
	vec4* pvaSet = new vec4[nPoints];
	vbo = new GLuint[2]; // one for coords, one for pvaSet1
	
	for (int i=0 ; i<nPoints ; i++)
	{
		pts[i].aCoords(points, 3*i);
		if (i == 0)
		{
			minMax[0] = minMax[1] = pts[0].x;
			minMax[2] = minMax[3] = pts[0].y;
			minMax[4] = minMax[5] = pts[0].z;
		}
		else
		{
			if (pts[i].x < minMax[0])
				minMax[0] = pts[i].x;
			else if (pts[i].x > minMax[1])
				minMax[1] = pts[i].x;
			if (pts[i].y < minMax[2])
				minMax[2] = pts[i].y;
			else if (pts[i].y > minMax[3])
				minMax[3] = pts[i].y;
			if (pts[i].z < minMax[4])
				minMax[4] = pts[i].z;
			else if (pts[i].z > minMax[5])
				minMax[5] = pts[i].z;
		}
		pvaSet[i][0] = sps[i];//shape
		pvaSet[i][1] = sz[i];//size
		pvaSet[i][2] = crs[i];//color

	}

	// send vertex data to GPU:
	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);

	glGenBuffers(3, vertexBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, nPoints*sizeof(vec3), points, GL_STATIC_DRAW);
	glVertexAttribPointer(pvaLoc_mcPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(PointsMV::pvaLoc_mcPosition);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, nPoints*sizeof(vec4), pvaSet, GL_STATIC_DRAW);
	glVertexAttribPointer(pvaLoc_pvaSet1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(PointsMV::pvaLoc_pvaSet1);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[2]);
	glBufferData(GL_ARRAY_BUFFER, nPoints*sizeof(vec4), NULL, GL_STATIC_DRAW);
	glVertexAttribPointer(pvaLoc_pvaSet2, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(PointsMV::pvaLoc_pvaSet2);

	delete [] pvaSet;
	delete [] points;
}

void PointsMV::fetchGLSLVariableLocations()
{
	if (PointsMV::shaderProgram > 0)
	{
		pvaLoc_mcPosition = pvAttribLocation(shaderProgram, "mcPosition");
		pvaLoc_pvaSet1 = pvAttribLocation(shaderProgram, "pvaSet1");
		pvaLoc_pvaSet2 = pvAttribLocation(shaderProgram, "pvaSet2");
		ppuLoc_color = ppUniformLocation(shaderProgram, "color");
		ppuLoc_mc_ec = ppUniformLocation(shaderProgram, "mc_ec");
		ppuLoc_ec_lds = ppUniformLocation(shaderProgram, "ec_lds");
		ppuLoc_xFactor = ppUniformLocation(shaderProgram, "xFactor");
		ppuLoc_yFactor = ppUniformLocation(shaderProgram, "yFactor");
		ppuLoc_sizeFactor = ppUniformLocation(shaderProgram, "sizeFactor");
		ppuLoc_attrToCutForCross = ppUniformLocation(shaderProgram, "attrToCutForCross");
		ppuLoc_attrToCutForCircle = ppUniformLocation(shaderProgram, "attrToCutForCircle");		
		ppuLoc_attrToCutForHourglass = ppUniformLocation(shaderProgram, "attrToCutForHourglass");	
		ppuLoc_attrToUseForShape = ppUniformLocation(shaderProgram, "attrToUseForShape");
		ppuLoc_attrToUseForSize = ppUniformLocation(shaderProgram, "attrToUseForSize");
		ppuLoc_attrToUseForColor = ppUniformLocation(shaderProgram, "attrToUseForColor");	
		ppuLoc_attrToCutForRed = ppUniformLocation(shaderProgram, "attrToCutForRed");
		ppuLoc_attrToCutForGreen = ppUniformLocation(shaderProgram, "attrToCutForGreen");
	}
}

void PointsMV::normalAttributes()
{
	if(useForShape != 0) useForShape = 0;
	if(useForSize != 1) useForSize = 1;
	if(useForColor != 2) useForColor = 2;
}

// xyzLimits: {mcXmin, mcXmax, mcYmin, mcYmax, mcZmin, mcZmax}
void PointsMV::getMCBoundingBox(double* xyzLimits) const
{
	for (int i=0 ; i<6 ; i++)
		xyzLimits[i] = minMax[i];
}

void PointsMV::render()
{
	float xFactor(1.0), yFactor(1.0);
	// save the current GLSL program in use
	GLint pgm;
	glGetIntegerv(GL_CURRENT_PROGRAM, &pgm);
	// draw the triangles using our vertex and fragment shaders
	glUseProgram(shaderProgram);

	// Retrieve and establish view mapping
	cryph::Matrix4x4 mc_ec, ec_lds;
	float buf[16];
	ModelView::getMatrices(mc_ec, ec_lds);
	glUniformMatrix4fv(ppuLoc_mc_ec, 1, false, mc_ec.extractColMajor(buf));
	glUniformMatrix4fv(ppuLoc_ec_lds, 1, false, ec_lds.extractColMajor(buf));

	glBindVertexArray(vao[0]);
	normalAttributes();

	glUniform4f(ppuLoc_color, 1.0, 0.0, 0.0, 1.0); // Red

	// Aspect ratio considerations for geometry shader:
	// (We need to talk about this in class - remind me if I forget!
	
	if (ModelView::aspectRatioPreservationEnabled)
	{
		float ratio = ModelView::ecDeltaY / ModelView::ecDeltaX;
		if (ratio > 1.0)
			yFactor = 1.0 / ratio;
		else
			xFactor = ratio;
	}
	glUniform1f(ppuLoc_xFactor, xFactor);
	glUniform1f(ppuLoc_yFactor, yFactor);
	// END: Aspect ratio considerations for geometry shader

	glUniform1f(ppuLoc_sizeFactor, sizeFactor);

	glUniform1f(ppuLoc_attrToCutForCross, cutForCross);
	glUniform1f(ppuLoc_attrToCutForCircle, cutForCircle);
	glUniform1f(ppuLoc_attrToCutForHourglass, cutForHourglass);

	glUniform1i(ppuLoc_attrToUseForShape, useForShape);
	glUniform1i(ppuLoc_attrToUseForSize, useForSize);
	glUniform1i(ppuLoc_attrToUseForColor, useForColor);

	glUniform1f(ppuLoc_attrToCutForRed, cutForRed);
	glUniform1f(ppuLoc_attrToCutForGreen, cutForGreen);

	glPointSize(3.0); // just in case mode == GL_POINTS
	glDrawArrays(mode, 0, nPoints);

	// restore the previous program
	glUseProgram(pgm);
}
