// AxesMV.c++

#include <iostream>

#include "AxesMV.h"
#include "ShaderIF.h"

ShaderIF* AxesMV::shaderIF = NULL;
int AxesMV::numInstances = 0;
GLuint AxesMV::shaderProgram = 0;
GLint AxesMV::pvaLoc_mcPosition = -1;

GLint AxesMV::ppuLoc_mc_ec = -1;
GLint AxesMV::ppuLoc_ec_lds = -1;
GLint AxesMV::ppuLoc_color = -1;

AxesMV::AxesMV(double xminIn, double xmaxIn, double dx, double fx,
               double yminIn, double ymaxIn, double dy, double fy,
	       double zminIn, double zmaxIn, double dz, double fz) :
		xmin(xminIn), xmax(xmaxIn), ymin(yminIn), ymax(ymaxIn), zmin(zminIn), zmax(zmaxIn)
{
	if (AxesMV::shaderProgram == 0)
	{
		// Create the common shader program:
		AxesMV::shaderIF = new ShaderIF("Common.vsh", "AxesMV.fsh");
		AxesMV::shaderProgram = shaderIF->getShaderPgmID();
		fetchGLSLVariableLocations();
	}

	// Now do instance-specific initialization:
	validateData(dx, fx, dy, fy, dz, fz);
	defineModel(dx, fx, dy, fy, dz, fz);
	AxesMV::numInstances++;
}

AxesMV::~AxesMV()
{
	glDeleteBuffers(1, vertexBuffer);
	glDeleteVertexArrays(1, vao);
	if (--AxesMV::numInstances == 0)
	{
		AxesMV::shaderIF->destroy();
		delete AxesMV::shaderIF;
		AxesMV::shaderIF = NULL;
		AxesMV::shaderProgram = 0;
	}
}

void AxesMV::defineModel(double dx, double fx, double dy, double fy, double dz, double fz)
{
	typedef float vec3[3];
	// Determine number of tic marks
	int nTicksOnX = (xmax - xmin) / dx;
	int nTicksOnY = (ymax - ymin) / dy;
	int nTicksOnZ = (zmax - zmin) / dz;

	// compute number points needed for axes and ticks:
	nPoints = 2 * (nTicksOnX + nTicksOnY + nTicksOnZ) + 6; // "+6": axes themselves

	vec3* points = new vec3[nPoints];

	// axes themselves:
	double yOfXAxis,zOfXAxis;
	if ((ymin <= 0.0) && (ymax >= 0.0))
		yOfXAxis = 0.0;
	else // 0 not in y range
		yOfXAxis = ymin;
	if ((zmin <= 0.0) && (zmax >= 0.0))
		zOfXAxis = 0.0;
	else // 0 not in y range
		zOfXAxis = zmin;
	points[0][0] = xmin; points[0][1] = yOfXAxis; points[0][2] = zOfXAxis;
	points[1][0] = xmax; points[1][1] = yOfXAxis; points[1][2] = zOfXAxis;
	double xOfYAxis,zOfYAxis;
	if ((xmin <= 0.0) && (xmax >= 0.0))
		xOfYAxis = 0.0;
	else // 0 not in x range
		xOfYAxis = xmin;
	if ((zmin <= 0.0) && (zmax >= 0.0))
		zOfYAxis = 0.0;
	else // 0 not in x range
		zOfYAxis = zmin;
	points[2][0] = xOfYAxis; points[2][1] = ymin; points[2][2] = zOfYAxis;
	points[3][0] = xOfYAxis; points[3][1] = ymax; points[3][2] = zOfYAxis;
	double xOfZAxis,yOfZAxis;	
	if ((xmin <= 0.0) && (xmax >= 0.0))
		xOfZAxis = 0.0;
	else // 0 not in z range
		xOfZAxis = xmin;
	if ((ymin <= 0.0) && (ymax >= 0.0))
		yOfZAxis = 0.0;
	else // 0 not in z range
		yOfZAxis = ymin;
	points[4][0] = xOfYAxis; points[4][1] = yOfZAxis; points[4][2] = zmin;
	points[5][0] = xOfYAxis; points[4][1] = yOfZAxis; points[5][2] = zmax;
	
	// create tic marks. Starting location in "points" is 6:
	int loc = 6;
	
	// first tics along x axis
	double xTicMin, xTicMax, yTicMin, yTicMax, zTicMin, zTicMax;
	double halfHeight = fx * dx;
	if (halfHeight <= 0.0)
	{
		yTicMin = ymin; yTicMax = ymax;
		zTicMin = zmin; zTicMax = zmax;
	}
	else
	{
		yTicMin = yOfXAxis - halfHeight; zTicMin = zOfXAxis - halfHeight;
		yTicMax = yOfXAxis + halfHeight; zTicMax = zOfXAxis + halfHeight;
	}
	double x = xmin;
	for (int i=0 ; i<nTicksOnX ; i++)
	{
		points[loc][0] = x; points[loc][1] = yTicMin; points[loc][2] = zTicMin;
		loc++;
		points[loc][0] = x; points[loc][1] = yTicMax; points[loc][2] = zTicMax;
		loc++;
		x += dx;
	}
	// now tics along y axis
	halfHeight = fy * dy;
	if (halfHeight <= 0.0)
	{
		xTicMin = xmin; xTicMax = xmax;
		zTicMin = zmin;	zTicMax = zmax;
	}
	else
	{
		xTicMin = xOfYAxis - halfHeight; zTicMin = zOfYAxis - halfHeight;
		xTicMax = xOfYAxis + halfHeight; zTicMax = zOfYAxis + halfHeight;
	}
	double y = ymin;
	for (int i=0 ; i<nTicksOnY ; i++)
	{
		points[loc][0] = xTicMin; points[loc][1] = y; points[loc][2] = zTicMin;
		loc++;
		points[loc][0] = xTicMax; points[loc][1] = y; points[loc][2] = zTicMax;
		loc++;
		y += dy;
	}
	// now tics along z axis
	halfHeight = fz * dz;
	if (halfHeight <= 0.0)
	{
		xTicMin = xmin; xTicMax = xmax;
		yTicMin = ymin;	yTicMax = ymax;
	}
	else
	{
		xTicMin = xOfZAxis - halfHeight; zTicMin = yOfZAxis - halfHeight;
		xTicMax = xOfZAxis + halfHeight; zTicMax = yOfZAxis + halfHeight;
	}
	double z = zmin;
	for (int i=0 ; i<nTicksOnY ; i++)
	{
		points[loc][0] = xTicMin; points[loc][1] = yTicMin; points[loc][2] = z;
		loc++;
		points[loc][0] = xTicMax; points[loc][1] = yTicMax; points[loc][2] = z;
		loc++;
		z += dz;
	}


	// send data to GPU:
	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);

	glGenBuffers(1, vertexBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, nPoints*sizeof(vec3), points, GL_STATIC_DRAW);
	glVertexAttribPointer(AxesMV::pvaLoc_mcPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(AxesMV::pvaLoc_mcPosition);
}

void AxesMV::fetchGLSLVariableLocations()
{
	if (AxesMV::shaderProgram > 0)
	{
		pvaLoc_mcPosition = pvAttribLocation(shaderProgram, "mcPosition");
		ppuLoc_color = ppUniformLocation(shaderProgram, "color");
		ppuLoc_mc_ec = ppUniformLocation(shaderProgram, "mc_ec");
		ppuLoc_ec_lds = ppUniformLocation(shaderProgram, "ec_lds");
	}
}

// xyzLimits: {mcXmin, mcXmax, mcYmin, mcYmax, mcZmin, mcZmax}
void AxesMV::getMCBoundingBox(double* xyzLimits) const
{
	xyzLimits[0] = xmin;
	xyzLimits[1] = xmax;
	xyzLimits[2] = ymin;
	xyzLimits[3] = ymax;
	xyzLimits[4] = zmin;
	xyzLimits[5] = zmax;
}

void AxesMV::render()
{
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

	// Make axes black:
	glUniform4f(ppuLoc_color, 0.0, 0.0, 0.0, 1.0);
	glDrawArrays(GL_LINES, 0, nPoints);

	// restore the previous program
	glUseProgram(pgm);
}

void AxesMV::validateData(double& dx, double& fx, double& dy, double& fy, double&dz, double& fz)
{
	if (xmin > xmax)
	{
		double t = xmin; xmin = xmax; xmax = t;
	}
	if (ymin > ymax)
	{
		double t = ymin; ymin = ymax; ymax = t;
	}
	if (zmin > zmax)
	{
		double t = zmin; zmin = zmax; zmax = t;
	}
	if (dx <= 0.0)
		dx = 0.1 * (xmax - xmin);
	if (dy <= 0.0)
		dy = 0.1 * (ymax - ymin);
	if (dz <= 0.0)
		dz = 0.1 * (zmax - zmin);
}
