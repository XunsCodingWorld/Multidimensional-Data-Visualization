// PointsMV.h

#ifndef POINTSMV_H
#define POINTSMV_H

class ShaderIF;

#include <GL/gl.h>

#include "ModelView.h"

class PointsMV : public ModelView
{
public:
	PointsMV(const cryph::AffPoint* pts, float* sps, float* sz, float* crs, int nPointsIn, GLenum modeIn);
	virtual ~PointsMV();

	// xyzLimits: {mcXmin, mcXmax, mcYmin, mcYmax, mcZmin, mcZmax}
	void getMCBoundingBox(double* xyzLimitsF) const;
	void render();

	float sizeFactor;
	float cutForCross, cutForCircle, cutForHourglass;
	float cutForRed, cutForGreen;
	int useForShape, useForSize, useForColor;
private:
	// structures to convey geometry to OpenGL/GLSL:
	GLuint vao[1];
	GLuint vertexBuffer[3];

	int nPoints;
	GLenum mode;
	double minMax[6];

	static ShaderIF* shaderIF;
	static int numInstances;
	static GLuint shaderProgram;
	static GLint pvaLoc_mcPosition, pvaLoc_pvaSet1, pvaLoc_pvaSet2;
	static GLint ppuLoc_color, ppuLoc_mc_ec, ppuLoc_ec_lds;
	static GLint ppuLoc_xFactor, ppuLoc_yFactor, ppuLoc_sizeFactor;
	static GLint ppuLoc_attrToCutForCross, ppuLoc_attrToCutForHourglass, ppuLoc_attrToCutForCircle;
	static GLint ppuLoc_attrToUseForShape, ppuLoc_attrToUseForSize, ppuLoc_attrToUseForColor;
	static GLint ppuLoc_attrToCutForRed, ppuLoc_attrToCutForGreen;

	void defineModel(const cryph::AffPoint* pts, float* sps, float* sz, float* crs);
	void normalAttributes();
	static void fetchGLSLVariableLocations();

};

#endif
