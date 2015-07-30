// ModelViewWithLighting.c++

#include <GL/freeglut.h>

#include "ModelViewWithLighting.h"
#include "ShaderIF.h"

// shader program id and locations of attribute and uniform variables inside it
GLuint ModelViewWithLighting::shaderProgram = 0;
int ModelViewWithLighting::numInstances = 0;
ShaderIF* ModelViewWithLighting::shaderIF = NULL;

std::string ModelViewWithLighting::vShaderSource = "../glslutil/shaders/ppu_matl/phong.vsh";
std::string ModelViewWithLighting::fShaderSource = "../glslutil/shaders/ppu_matl/incomingColorAndTexture.fsh";

GLint ModelViewWithLighting::pvaLoc_mcPosition = -1,
      ModelViewWithLighting::pvaLoc_mcNormal = -1,
      ModelViewWithLighting::pvaLoc_texCoords = -1;

// These might be pp or pv, depending on which shader program is used:
GLint ModelViewWithLighting::loc_ka = -1,
      ModelViewWithLighting::loc_kd = -1,
      ModelViewWithLighting::loc_ks = -1,
      ModelViewWithLighting::loc_shininess = -1,
      ModelViewWithLighting::loc_alpha = -1;

GLint ModelViewWithLighting::ppuLoc_actualNumLights = -1,
      ModelViewWithLighting::ppuLoc_ecLightPosition = -1,
      ModelViewWithLighting::ppuLoc_lightStrength = -1,
      ModelViewWithLighting::ppuLoc_globalAmbient = -1,
      ModelViewWithLighting::ppuLoc_mc_ec = -1,
      ModelViewWithLighting::ppuLoc_ec_lds = -1,
      ModelViewWithLighting::ppuLoc_colorGenerationMode = -1,
      ModelViewWithLighting::ppuLoc_textureSource = -1,
      ModelViewWithLighting::ppuLoc_textureMap = -1,
      ModelViewWithLighting::ppuLoc_useTextureColorMultiplier = -1,
      ModelViewWithLighting::ppuLoc_textureColorMultiplier = -1,
      ModelViewWithLighting::ppuLoc_lmFlags = -1;

unsigned int ModelViewWithLighting::lmFlags = 255 + ambientBit + diffuseBit + specularBit;

float ModelViewWithLighting::lightPos[4*MAX_NUM_LIGHTS] =
	{
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 1.0, 0.0
	};
// Are coordinates in "lightPos" stored in MC or EC?
bool ModelViewWithLighting::posInModelCoordinates[MAX_NUM_LIGHTS] = { false, false, false };
// The following is the buffer actually sent to GLSL. It will contain a copy of
// the (x,y,z,w) for light sources defined in EC; it will contain the coordinates
// after transformation to EC if the position was originally specified in MC.
float posToGLSL[4*MAX_NUM_LIGHTS];

float ModelViewWithLighting::lightStrength[3*MAX_NUM_LIGHTS] =
	{
		1.0, 1.0, 1.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0
	};
	
float ModelViewWithLighting::globalAmbient[] = { 0.2, 0.2, 0.2 };

ModelViewWithLighting::ModelViewWithLighting() : shiftedArrowTarget(' '),
	// For "textureSource":
	// 0: image-based texture read from file;
	// 1: image-based texture*textureColorModifier
	// >0: a fragment shader texture
	textureSource(0),
	colorGenerationMode(JUST_colorToFS)
{
	if (ModelViewWithLighting::shaderIF == NULL)
	{
		ModelViewWithLighting::shaderIF = new ShaderIF(vShaderSource, fShaderSource);
		ModelViewWithLighting::shaderProgram = shaderIF->getShaderPgmID();
		fetchGLSLVariableLocations();
	}
	ModelViewWithLighting::numInstances++;
}

ModelViewWithLighting::~ModelViewWithLighting()
{
	if (--ModelViewWithLighting::numInstances == 0)
	{
		ModelViewWithLighting::shaderIF->destroy();
		delete ModelViewWithLighting::shaderIF;
		ModelViewWithLighting::shaderIF = NULL;
		ModelViewWithLighting::shaderProgram = 0;
	}
}

void ModelViewWithLighting::defineLightingEnvironment()
{
	glUniform1i(ppuLoc_actualNumLights, MAX_NUM_LIGHTS);
	cryph::Matrix4x4 mc_ec, ec_lds;
	getMatrices(mc_ec, ec_lds);
	int baseLoc = 0;
	for (int i=0 ; i<MAX_NUM_LIGHTS ; i++, baseLoc+=4)
	{
		if (posInModelCoordinates[i])
			mc_ec.multiply(&lightPos[baseLoc], &posToGLSL[baseLoc]);
		else
			for (int k=0 ; k<4 ; k++)
				posToGLSL[baseLoc+k] = lightPos[baseLoc+k];
	}
	glUniform4fv(ppuLoc_ecLightPosition, MAX_NUM_LIGHTS, posToGLSL);
	glUniform3fv(ppuLoc_lightStrength, MAX_NUM_LIGHTS, lightStrength);
	glUniform3fv(ppuLoc_globalAmbient, 1, globalAmbient);
	glUniform1i(ppuLoc_lmFlags, lmFlags);
}

void ModelViewWithLighting::fetchGLSLVariableLocations()
{
	if (ModelViewWithLighting::shaderProgram > 0)
	{
		// record attribute locations
		ModelViewWithLighting::pvaLoc_mcPosition = pvAttribLocation(shaderProgram, "mcPosition");
		ModelViewWithLighting::pvaLoc_mcNormal = pvAttribLocation(shaderProgram, "mcNormal");
		ModelViewWithLighting::pvaLoc_texCoords = pvAttribLocation(shaderProgram, "texCoords");
		// record material property attribute locations
		ModelViewWithLighting::loc_ka = glGetUniformLocation(shaderProgram, "ka");
		if (loc_ka < 0)
		{
			// must be per-vertex
			ModelViewWithLighting::loc_ka = pvAttribLocation(shaderProgram, "ka");
			ModelViewWithLighting::loc_kd = pvAttribLocation(shaderProgram, "kd");
			ModelViewWithLighting::loc_ks = pvAttribLocation(shaderProgram, "ks");
			ModelViewWithLighting::loc_shininess = pvAttribLocation(shaderProgram, "shininess");
			ModelViewWithLighting::loc_alpha = pvAttribLocation(shaderProgram, "alpha");
		}
		else
		{
			ModelViewWithLighting::loc_kd = ppUniformLocation(shaderProgram, "kd");
			ModelViewWithLighting::loc_ks = ppUniformLocation(shaderProgram, "ks");
			ModelViewWithLighting::loc_shininess = ppUniformLocation(shaderProgram, "shininess");
			ModelViewWithLighting::loc_alpha = ppUniformLocation(shaderProgram, "alpha");
		}
		// record uniform locations
		ModelViewWithLighting::ppuLoc_ecLightPosition = ppUniformLocation(shaderProgram, "ecLightPosition");
		ModelViewWithLighting::ppuLoc_actualNumLights = ppUniformLocation(shaderProgram, "actualNumLights");
		ModelViewWithLighting::ppuLoc_lightStrength = ppUniformLocation(shaderProgram, "lightStrength");
		ModelViewWithLighting::ppuLoc_globalAmbient = ppUniformLocation(shaderProgram, "globalAmbient");
		ModelViewWithLighting::ppuLoc_mc_ec = ppUniformLocation(shaderProgram, "mc_ec");
		ModelViewWithLighting::ppuLoc_ec_lds = ppUniformLocation(shaderProgram, "ec_lds");
		ModelViewWithLighting::ppuLoc_colorGenerationMode = ppUniformLocation(shaderProgram, "colorGenerationMode");
		ModelViewWithLighting::ppuLoc_textureSource = ppUniformLocation(shaderProgram, "textureSource");
		ModelViewWithLighting::ppuLoc_textureMap = ppUniformLocation(shaderProgram, "textureMap");
		ModelViewWithLighting::ppuLoc_useTextureColorMultiplier = ppUniformLocation(shaderProgram, "useTextureColorMultiplier");
		ModelViewWithLighting::ppuLoc_textureColorMultiplier = ppUniformLocation(shaderProgram, "textureColorMultiplier");
		ModelViewWithLighting::ppuLoc_lmFlags = ppUniformLocation(shaderProgram, "lmFlags");
	}
}

void ModelViewWithLighting::handleCommand(unsigned char key, int num, double ldsX, double ldsY)
{
	if ((key == 'T') && (num >= 0) && (num < 5))
		textureSource = num;
	else if ((key == 'c') && (num >= 0))
	{
		if (validColorGenerationMode(num))
			colorGenerationMode = num;
	}
	else if ((key == 'L') && (num >= 0) && (num < MAX_NUM_LIGHTS))
		turnLight(num, ON);
	else if ((key == 'l') && (num >= 0) && (num < MAX_NUM_LIGHTS))
		turnLight(num, OFF);
	else if ((key == 'w') && (num >= 0) && (num < MAX_NUM_LIGHTS))
		posInModelCoordinates[num] = true;
	else if ((key == 'e') && (num >= 0) && (num < MAX_NUM_LIGHTS))
		posInModelCoordinates[num] = false;
	else // pass back up...
		ModelView::handleCommand(key, num, ldsX, ldsY);

	glutPostRedisplay();
}

void ModelViewWithLighting::handleCommand(unsigned char key, double ldsX, double ldsY)
{
	if (key == 'a')
	{
		shiftedArrowTarget = 'a';
		turnAmbient(OFF);
	}
	else if (key == 'A')
	{
		shiftedArrowTarget = 'a';
		turnAmbient(ON);
	}
	else if (key == 'd')
		turnDiffuse(OFF);
	else if (key == 'D')
		turnDiffuse(ON);
	else if (key == 's')
		turnSpecular(OFF);
	else if (key == 'S')
		turnSpecular(ON);
	else if (key == 'm')
		shiftedArrowTarget = 'm';
	else
		ModelView::handleCommand(key, ldsX, ldsY);
	glutPostRedisplay();
}

void ModelViewWithLighting::handleSpecialKey(int key, double ldsX, double ldsY)
{
	static const float deltaA = 0.05;
	// 'm' is 'shininess':
	static const float deltaM = 1.0;
	static const float minM = 1.0;
	if (key == GLUT_KEY_UP)
	{
		if (shiftedArrowTarget == 'a')
		{
			float ar = globalAmbient[0] + deltaA;
			float ag = globalAmbient[1] + deltaA;
			float ab = globalAmbient[2] + deltaA;
			if ((ar <= 1.0) && (ag <= 1.0) && (ab <= 1.0))
			{
				globalAmbient[0] = ar;
				globalAmbient[1] = ag;
				globalAmbient[2] = ab;
			}
		}
		else if (shiftedArrowTarget == 'm')
		{
			matl.shininess += deltaM;
			std::cout << "m = " << matl.shininess << '\n';
		}
	}
	else if (key == GLUT_KEY_DOWN)
	{
		if (shiftedArrowTarget == 'a')
		{
			float ar = globalAmbient[0] - deltaA;
			float ag = globalAmbient[1] - deltaA;
			float ab = globalAmbient[2] - deltaA;
			if ((ar >= 0.0) && (ag >= 0.0) && (ab >= 0.0))
			{
				globalAmbient[0] = ar;
				globalAmbient[1] = ag;
				globalAmbient[2] = ab;
			}
		}
		else if (shiftedArrowTarget == 'm')
		{
			matl.shininess -= deltaM;
			if (matl.shininess < minM)
				matl.shininess = minM;
			std::cout << "m = " << matl.shininess << '\n';
		}
	}
}

std::string ModelViewWithLighting::offOnString(int bit) const
{
	if ((lmFlags & bit) == 0)
		return "OFF";
	return "ON";
}

void ModelViewWithLighting::printKeyboardKeyList(bool firstCall) const
{
	if (!firstCall)
		return;

	ModelView::printKeyboardKeyList(firstCall);

	std::string amb = offOnString(ambientBit);
	std::string diff = offOnString(diffuseBit);
	std::string spec = offOnString(specularBit);
	std::string light0 = offOnString(1);
	std::string light1 = offOnString(2);
	std::string light2 = offOnString(4);
	std::cout << "ModelViewWithLighting:\n";
	std::cout << "\ta/A, d/D, s/S - turn ambient, diffuse, specular off/on (currently: "
	          << amb << ", " + diff + ", " + spec + ")\n";
	std::cout << "\tL@0, L@1, etc. (l@0, l@1, etc.) - turn light 0, 1, ... on (off) (currently: "
	          << light0 << ", " << light1 << ", " << light2 << ")\n";
	std::cout << "\tw@0, ... - light source defined in MC\n";
	std::cout << "\te@0, ... - light source defined in EC\n";

	std::cout << "\tT@0, T@1, T@2, T@3, T@4: set texture source\n";
	std::cout << "\tc#iii$: color generation mode:\n";
	std::cout << "\t\t 0: just colorToFS\n";
	std::cout << "\t\t10: just textureColor\n";
	std::cout << "\t\t20: colorToFS * textureColor\n";
	std::cout << "\t\t21: colorToFS + textureColor\n";
	std::cout << "\t\t22: colorToFS - textureColor\n";
	std::cout << "\t\t23: textureColor - colorToFS\n";
	std::cout << "\t\t24: colorToFS / textureColor\n";
	std::cout << "\t\t25: textureColor / colorToFS\n";

	std::cout << "\tShifted up/down arrow keys: increase/decrease either m or ambient\n";
}

void ModelViewWithLighting::setGlobalAmbient(float* strength)
{
	for (int i=0 ; i<3 ; i++)
		globalAmbient[i] = strength[i];
}

void ModelViewWithLighting::setLightPosition(int whichLight, float* xyzw, bool inMC)
{
	if ((whichLight < 0) || (whichLight >= MAX_NUM_LIGHTS))
		return;
	int loc = whichLight * 4;
	for (int i=0 ; i<4 ; i++)
		lightPos[loc+i] = xyzw[i];
	posInModelCoordinates[whichLight] = inMC;
}

void ModelViewWithLighting::setLightStrength(int whichLight, float* strength)
{
	if ((whichLight < 0) || (whichLight >= MAX_NUM_LIGHTS))
		return;
	int loc = whichLight * 3;
	for (int i=0 ; i<3 ; i++)
		lightStrength[loc+i] = strength[i];
}

void ModelViewWithLighting::setShaderSources(const std::string& vShader, const std::string& fShader)
{
	vShaderSource = vShader;
	fShaderSource = fShader;
}

void ModelViewWithLighting::turnAmbient(OnOrOff onOff)
{
	if (onOff == ON)
		lmFlags |= ambientBit;
	else
		lmFlags &= ~ambientBit;
}

void ModelViewWithLighting::turnDiffuse(OnOrOff onOff)
{
	if (onOff == ON)
		lmFlags |= diffuseBit;
	else
		lmFlags &= ~diffuseBit;
}

void ModelViewWithLighting::turnSpecular(OnOrOff onOff)
{
	if (onOff == ON)
		lmFlags |= specularBit;
	else
		lmFlags &= ~specularBit;
}

void ModelViewWithLighting::turnLight(int whichLight, OnOrOff onOff)
{
	if ((whichLight < 0) || (whichLight >= MAX_NUM_LIGHTS))
		return;
	int bit = 1 << whichLight;
	if (onOff == ON)
		lmFlags |= bit;
	else
		lmFlags &= ~bit;
}

bool ModelViewWithLighting::validColorGenerationMode(int mode)
{
	if (mode == (int) JUST_colorToFS)
		return true;
    if (mode == (int) JUST_textureColor)
		return true;
    if (mode == (int) PRODUCT_colorToFS_textureColor)
		return true;
    if (mode == (int) SUM_colorToFS_textureColor)
		return true;
    if (mode == (int) DIFFERENCE_colorToFS_textureColor)
		return true;
    if (mode == (int) DIFFERENCE_textureColor_colorToFS)
		return true;
    if (mode == (int) QUOTIENT_colorToFS_textureColor)
		return true;
    if (mode == (int) QUOTIENT_textureColor_colorToFS)
		return true;
	return false;
}
