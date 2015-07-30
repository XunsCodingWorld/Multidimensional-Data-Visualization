// AxesMV.h

#ifndef AXES_H
#define AXES_H

class ShaderIF;

#include <string>
#include <GL/gl.h>

#include "ModelView.h"

class AxesMV : public ModelView
{
public:
	       // dx is the spacing between tic marks; fx*dx will
	       // be the half-height of a tic mark. (If fx<=0, then
	       // tics will be drawn from ymin to ymax):
	AxesMV(double xminIn, double xmaxIn, double dx, double fx,
	       // dy and fy are analogously defined and used:
	       double yminIn, double ymaxIn, double dy, double fy,
	       // dz and fz are analogously defined and used:
 	       double zminIn, double zmaxIn, double dz, double fz);
	virtual ~AxesMV();

	// xyzLimits: {mcXmin, mcXmax, mcYmin, mcYmax, mcZmin, mcZmax}
	void getMCBoundingBox(double* xyzLimitsF) const;
	void render();

private:
	// structures to convey geometry to OpenGL/GLSL:
	GLuint vao[1];
	GLuint vertexBuffer[1]; // Stores points for axes and tics

	int nPoints;
	double xmin, xmax, ymin, ymax, zmin, zmax;

	static ShaderIF* shaderIF;
	static int numInstances;
	static GLuint shaderProgram;
	static GLint pvaLoc_mcPosition;
	static GLint ppuLoc_color, ppuLoc_mc_ec, ppuLoc_ec_lds;

	void defineModel(double dx, double fx, double dy, double fy, double dz, double fz);
	static void fetchGLSLVariableLocations();
	void validateData(double& dx, double& fx, double& dy, double& fy, double& dz, double& fz);
};

#endif
