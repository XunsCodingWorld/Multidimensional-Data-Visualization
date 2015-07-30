// ModelViewWithLighting.h

#ifndef PHONGMODELVIEW_H
#define PHONGMODELVIEW_H

#include <GL/gl.h>

#include "ModelView.h"
#include "PhongMaterial.h"

class ShaderIF;

enum OnOrOff
{
	ON, OFF
};

static const int ambientBit = 256, diffuseBit = 512, specularBit = 1024;
static const int MAX_NUM_LIGHTS = 3; // MUST BE KEPT SAME AS IN phong.vsh

// fragment shader color generation modes:
enum ColorGenerationMode
{
	JUST_colorToFS = 0,
	JUST_textureColor = 10,
	PRODUCT_colorToFS_textureColor = 20,
	SUM_colorToFS_textureColor,
	DIFFERENCE_colorToFS_textureColor,
	DIFFERENCE_textureColor_colorToFS,
	QUOTIENT_colorToFS_textureColor,
	QUOTIENT_textureColor_colorToFS
};

class ModelViewWithLighting : public ModelView
{
public:
	ModelViewWithLighting();
	virtual ~ModelViewWithLighting();

	void handleCommand(unsigned char key, double ldsX, double ldsY);
	void handleCommand(unsigned char key, int num, double ldsX, double ldsY);
	void handleSpecialKey(int key, double ldsX, double ldsY);

	void printKeyboardKeyList(bool firstCall) const;

	// following specify the lighting environment.
	static void setGlobalAmbient(float* strength); // (r,g,b)
	// For the rest we require: 0 <= whichLight < MAX_NUM_LIGHTS
	// All light positions are initially (0,0,1,0)
	static void setLightPosition(int whichLight, float* xyzw, bool inMC);
	// Light 0 initially has srength (1,1,1). All others initially have
	// strength (0,0,0).
	static void setLightStrength(int whichLight, float* strength); // (r,g,b)
	// All lights are initially 'on' (but note initial strengths above)
	static void turnLight(int whichLight, OnOrOff onOff);
	static bool validColorGenerationMode(int mode);
	// Alter location of shader files
	static void setShaderSources(const std::string& vShader, const std::string& fShader);

	// The folllowing are mostly useful for demonstration purposes:
	static void turnAmbient(OnOrOff onOff);
	static void turnDiffuse(OnOrOff onOff);
	static void turnSpecular(OnOrOff onOff);
protected:
	static ShaderIF* shaderIF;
	static GLuint shaderProgram;

	// GLSL per-vertex attributes for geometry and texture coordinates
	static GLint pvaLoc_mcPosition, pvaLoc_mcNormal, pvaLoc_texCoords;

	// GLSL attributes for material properties (might be per-primitive -OR- per-vertex)
	static GLint loc_ka, loc_kd, loc_ks, loc_shininess, loc_alpha;

	// GLSL per-primitive attributes for lighting environment
	static GLint ppuLoc_actualNumLights;
	static GLint ppuLoc_ecLightPosition, ppuLoc_lightStrength,
	             ppuLoc_globalAmbient;

	// Other GLSL per-primitive attributes
	static GLint ppuLoc_mc_ec, ppuLoc_ec_lds;
	static GLint ppuLoc_colorGenerationMode, ppuLoc_textureSource, ppuLoc_textureMap,
	             ppuLoc_useTextureColorMultiplier, ppuLoc_textureColorMultiplier,
	             ppuLoc_lmFlags;

	// The following uses the light sources and their on/off state as
	// defined here.
	void defineLightingEnvironment();

	// convenience instance variables that subclasses can populate and use
	PhongMaterial matl;
	char shiftedArrowTarget; // shininess or ambient target?
	int textureSource, colorGenerationMode;

private:
	std::string offOnString(int bit) const;
	static void fetchGLSLVariableLocations();
	static std::string vShaderSource, fShaderSource;

	static int numInstances;
	static unsigned int lmFlags;

	// lighting environment
	static float lightPos[4*MAX_NUM_LIGHTS]; // (x,y,z,w) for each light
	static bool posInModelCoordinates[MAX_NUM_LIGHTS]; // pos is in MC or EC?
	static float lightStrength[3*MAX_NUM_LIGHTS]; // (r,g,b) for each light
	static float globalAmbient[3]; // (r,g,b) for ambient term, A
};

#endif
