// ModelView.c++ - abstract base class for models managed by a Controller

#include <iostream>

#include <GL/freeglut.h>

#include "ModelView.h"
#include "Controller.h"

// We record eye coordinate deltas for use in defining projection
// matrices and converting LDS offsets to eye (and hence model) deltas.
double ModelView::ecDeltaX = 2.0, ModelView::ecDeltaY = 2.0, ModelView::ecDeltaZ = 2.0;
double ModelView::ecDeltaMax = 2.0;

// dynamic zoom is built into the window-viewport mapping portion of the
// ec_lds matrix by expanding/contracting the window
double ModelView::scale = 1.0;
bool ModelView::useGlobalZoomIn_computeScaleTrans = true;
// for MC to Eye:
// init E(0,0,4), C(0,0,0), up(0,1,0)
ModelView::EyeCoordSystemSpec ModelView::curEC, ModelView::origEC;

// Dynamic view panning: A common interface from the Controller is used to
// drive either a 2D pan in computeScaleTrans, or a 3D one in getMatrices.
// For the former, "mcPanVector2D" is computed in model coordinates in addToGlobalPan
// below. 3D view panning can be done in one of (at least) two ways:
// (1) A 3D pan vector is computed in model coordinates and then added to
//     the eye and center.
// (2) Accumulate pan translations into the dynamic matrix.
cryph::AffVector ModelView::mcPanVector2D(0,0,0);
// Default for following is "rotate about center of attention"
double ModelView::fractionOfDistEyeCenterToCenterOfRotation = 1.0;
// Projections
ProjectionType ModelView::projType = PERSPECTIVE;
cryph::AffVector ModelView::obliqueProjectionDir = cryph::AffVector(-0.2, 0.1, 1.0);
double ModelView::zpp = -3.0;
bool ModelView::aspectRatioPreservationEnabled = true;
// Matrices
cryph::Matrix4x4 ModelView::dynamic(1,0,0,0 , 0,1,0,0 , 0,0,1,0, 0,0,0,1);
cryph::Matrix4x4 ModelView::lookAtMatrix;
cryph::Matrix4x4 ModelView::mc_ec_full;
cryph::Matrix4x4 ModelView::ec_lds;

// EXPERIMENTAL:
// 1) Translating dynamic rotations to eye-center-up
// 2) Translating Pan to movements of eye-center
// TODO: How do these two options interact? Must at least one
//       be false, or can I have them both be true?
bool ModelView::translateDynamicRotationToEyeUp = false;
bool ModelView::translateDynamicPanToEyeCenter = false;
cryph::Matrix4x4 ModelView::originalVOM;
bool ModelView::haveOriginalVOM = false;
// END: EXPERIMENTAL

ModelView::ModelView() : polygonMode(GL_FILL)
{
}
ModelView::~ModelView()
{
}

void ModelView::addToGlobalPan(double dxInLDS, double dyInLDS, double dzInLDS)
{
	// In case this is a 2D application/ModelView instance:
    float sclTrans[4];
	computeScaleTrans(sclTrans);
	cryph::AffVector inc(dxInLDS/sclTrans[0], dyInLDS/sclTrans[2], 0.0);
	mcPanVector2D += inc;

	// Now in case this is a 3D application:

	// map the deltas from their (-1,+1) range to distances in eye coordinates:
	double dxInEC = 0.5 * dxInLDS * ecDeltaX;
	double dyInEC = 0.5 * dyInLDS * ecDeltaY;
	double dzInEC = 0.5 * dzInLDS * ecDeltaZ;

	if (ModelView::translateDynamicPanToEyeCenter)
	{
		double m[16];
		mc_ec_full.extractColMajor(m);
		// map the eye coordinate pan vector to a model coordinate pan vector:

		cryph::AffVector mcPanVector3D =
		//                         EC x axis represented in MC:
		                  dxInEC * cryph::AffVector(m[0], m[4], m[8]) +
		//                         EC y axis represented in MC: 
		                  dyInEC * cryph::AffVector(m[1], m[5], m[9]) +
		//                         EC z axis represented in MC:
		                  dzInEC * cryph::AffVector(m[2], m[6], m[10]);
		curEC.eye -= mcPanVector3D;
		curEC.center -= mcPanVector3D;
	}
	else // conventional implementation:
	{
		// For 3D, we build and concatenate a translation matrix:
		cryph::Matrix4x4 trans = cryph::Matrix4x4::translation(
			cryph::AffVector(dxInEC, dyInEC, dzInEC));
		dynamic = trans * dynamic;
	}
}

void ModelView::addToGlobalRotationDegrees(double rx, double ry, double rz)
{
	cryph::Matrix4x4 rxM = cryph::Matrix4x4::xRotationDegrees(rx);
	cryph::Matrix4x4 ryM = cryph::Matrix4x4::yRotationDegrees(ry);
	cryph::Matrix4x4 rzM = cryph::Matrix4x4::zRotationDegrees(rz);
	dynamic = rxM * ryM * rzM * dynamic;
}

void ModelView::addToGlobalZoom(double increment)
{
	if ((scale+increment) > 0.0)
		scale += increment;
}

void ModelView::computeScaleTrans(float* scaleTransF) // USED FOR 2D SCENES
{
	Controller* c = Controller::getCurrentController();
    double xyzLimits[6];
    c->getMCRegionOfInterest(xyzLimits);
    double xmin = xyzLimits[0]-mcPanVector2D[0], xmax = xyzLimits[1]-mcPanVector2D[0];
    double ymin = xyzLimits[2]-mcPanVector2D[1], ymax = xyzLimits[3]-mcPanVector2D[1];

	if (aspectRatioPreservationEnabled)
	{
    	// preserve aspect ratio. Make "region of interest" wider or taller to
    	// match the Controller's viewport aspect ratio.
    	double vAR = c->getViewportAspectRatio();
    	if (vAR > 0.0)
    	{
        	double wHeight = xyzLimits[3] - xyzLimits[2];
        	double wWidth = xyzLimits[1] - xyzLimits[0];
        	double wAR = wHeight / wWidth;
        	if (wAR > vAR)
        	{
            	// make window wider
            	wWidth = wHeight / vAR;
            	double xmid = 0.5 * (xmin + xmax);
            	xmin = xmid - 0.5*wWidth;
            	xmax = xmid + 0.5*wWidth;
        	}
        	else
        	{
            	// make window taller
            	wHeight = wWidth * vAR;
            	double ymid = 0.5 * (ymin + ymax);
            	ymin = ymid - 0.5*wHeight;
            	ymax = ymid + 0.5*wHeight;
        	}
    	}
	}
	
	// We are only concerned with the xy extents for now, hence we will
	// ignore xyzLimits[4] and xyzLimits[5].
	// Map the overall limits to the -1..+1 range expected by the OpenGL engine:
    double scaleTrans[4];
	double ldsD = 1.0;
	if (useGlobalZoomIn_computeScaleTrans)
		ldsD = ModelView::scale;
    linearMap(xmin, xmax, -ldsD, ldsD, scaleTrans[0], scaleTrans[1]);
    linearMap(ymin, ymax, -ldsD, ldsD, scaleTrans[2], scaleTrans[3]);
    for (int i=0 ; i<4 ; i++)
        scaleTransF[i] = static_cast<float>(scaleTrans[i]);
}

void ModelView::getMatrices(cryph::Matrix4x4& mc_ec_fullOut,
							cryph::Matrix4x4& ec_ldsOut)
{
	double preTransDist = fractionOfDistEyeCenterToCenterOfRotation * curEC.distEyeCenter;
	cryph::Matrix4x4 preTrans = cryph::Matrix4x4::translation(
		cryph::AffVector(0.0, 0.0, preTransDist));
	cryph::Matrix4x4 postTrans = cryph::Matrix4x4::translation(
		cryph::AffVector(0.0, 0.0, -preTransDist));
	cryph::Matrix4x4 post_dynamic_pre = postTrans * dynamic * preTrans;

	lookAtMatrix = cryph::Matrix4x4::lookAt(curEC.eye, curEC.center, curEC.up);
	if (ModelView::translateDynamicRotationToEyeUp)
	{
		if (!ModelView::haveOriginalVOM)
		{
			originalVOM = cryph::Matrix4x4::lookAt(
						curEC.eye, curEC.center, curEC.up);
			haveOriginalVOM = true;
		}
		cryph::Matrix4x4 vom = post_dynamic_pre * originalVOM;
		double m[16];
		vom.extractColMajor(m);
		cryph::AffVector wHat(m[2],m[6],m[10]);
		curEC.eye = curEC.center + curEC.distEyeCenter*wHat;
		curEC.up = cryph::AffVector(m[1],m[5],m[9]);
		mc_ec_full = lookAtMatrix;
	}
	else
		mc_ec_full = post_dynamic_pre * lookAtMatrix;

	setProjectionTransformation();

	// Return the two final matrices
	mc_ec_fullOut = mc_ec_full;
	ec_ldsOut = ec_lds;
}

void ModelView::handleCommand(unsigned char key, double ldsX, double ldsY)
{
}

static GLenum pm[3] = { GL_POINT, GL_LINE, GL_FILL };

void ModelView::handleCommand(unsigned char key, int num, double ldsX, double ldsY)
{
	if ((key == 'p') && (num >= 0) && (num < 3))
	{
		polygonMode = pm[num];
		glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
	}
}

void ModelView::linearMap(double fromMin, double fromMax, double toMin, double toMax,
	double& scale, double& trans)
{
	scale = (toMax - toMin) / (fromMax - fromMin);
	trans = toMin - scale*fromMin;
}

GLint ModelView::ppUniformLocation(GLuint glslProgram, const std::string& name)
{
	GLint loc = glGetUniformLocation(glslProgram, name.c_str());
	if (loc < 0)
		std::cerr << "Could not locate per-primitive uniform: '" << name << "'\n";
	return loc;
}

void ModelView::printKeyboardKeyList(bool firstCall) const
{
	if (!firstCall)
		return;

	std::cout << "ModelView:\n";
	std::cout << "\tp@0, p@1, p@2: GL_POINT, GL_LINE, GL_FILL\n";
}

GLint ModelView::pvAttribLocation(GLuint glslProgram, const std::string& name)
{
	GLint loc = glGetAttribLocation(glslProgram, name.c_str());
	if (loc < 0)
		std::cerr << "Could not locate per-vertex attribute: '" << name << "'\n";
	return loc;
}

void ModelView::resetGlobalDynamic()
{
	mcPanVector2D = cryph::AffVector(0,0,0);
	dynamic = cryph::Matrix4x4::IdentityMatrix;
	curEC = origEC;
}

void ModelView::resetGlobalZoom()
{
	scale = 1.0;
}	

void ModelView::scaleGlobalZoom(double multiplier)
{
	if (multiplier > 0.0)
		scale *= multiplier;
}

void ModelView::set_ecDeltas()
{
	Controller* c = Controller::getCurrentController();
    double xyz[6];
    c->getMCRegionOfInterest(xyz);
	double dx = xyz[1] - xyz[0];
	double dy = xyz[3] - xyz[2];
	double dz = xyz[5] - xyz[4];
	cryph::AffVector vecs[7] = {
		cryph::AffVector(dx, 0, 0), cryph::AffVector(0, dy, 0),
		cryph::AffVector(0, 0, dz), cryph::AffVector(dx, dy, 0),
		cryph::AffVector(dx, 0, dz), cryph::AffVector(0, dy, dz),
		cryph::AffVector(dx, dy, dz)
	};

	double m[16];
	lookAtMatrix.extractRowMajor(m);
	cryph::AffVector uvw[3] = {
		cryph::AffVector(m[0], m[1], m[2]),
		cryph::AffVector(m[4], m[5], m[6]),
		cryph::AffVector(m[8], m[9], m[10])
	};

	double ecd[3];
	ecDeltaMax = 0.0;
	for (int i=0 ; i<3 ; i++)
	{
		ecd[i] = 0.0;
		for (int j=0 ; j<7 ; j++)
		{
			double d = uvw[i].dot(vecs[j]);
			if (d > ecd[i])
				ecd[i] = d;
		}
		if (ecd[i] > ecDeltaMax)
			ecDeltaMax = ecd[i];
	}
	ecDeltaX = ecd[0] / scale;
	ecDeltaY = ecd[1] / scale;
	ecDeltaZ = ecd[2]; // This will be reset in setProjectionTransformation

	if (aspectRatioPreservationEnabled)
	{
		double wAR = ecDeltaY / ecDeltaX; // height/width
		double vAR = Controller::getCurrentController()->getViewportAspectRatio();
		if (vAR > wAR)
			ecDeltaY = vAR * ecDeltaX;
		else
			ecDeltaX = ecDeltaY / vAR;
	}
}

void ModelView::setEyeCenterUp(cryph::AffPoint E, cryph::AffPoint C, cryph::AffVector up)
{
	cryph::AffVector v, w;
	if (cryph::Matrix4x4::getECvw(E, C, up, v, w))
	{
		origEC.eye = E;
		origEC.center = C;
		origEC.up = up;
		origEC.distEyeCenter = origEC.eye.distanceTo(origEC.center);
		resetGlobalDynamic();
		resetGlobalZoom();
	}
}

// In following: 0: about eye; 1: about center of attention
void ModelView::setFractionalDistEyeToCenterOfRotation(double f)
{
	fractionOfDistEyeCenterToCenterOfRotation = f;
}

void ModelView::setObliqueProjectionDirection(const cryph::AffVector& dir)
{
	ModelView::obliqueProjectionDir = dir;
}

void ModelView::setProjection(ProjectionType pType)
{
	projType = pType;
}

void ModelView::setProjectionPlaneZ(double zppIn)
{
	zpp = zppIn;
}

// Following is very roughly the 3D counterpart of the 2D "computeScaleTrans"
void ModelView::setProjectionTransformation()
{
	// adjust window deltas to match the aspect ratio of the viewport
	set_ecDeltas();

	// Create the parameters for the desired type of projection

	// Set zmin to avoid depth clipping at z=zmin
	double zmin = -4.0 * ecDeltaMax, zmax = 0.0;

	double halfDx = 0.5 * ecDeltaX;
	double halfDy = 0.5 * ecDeltaY;
	if (projType == PERSPECTIVE)
	{
		zmax = -0.01;
		ec_lds = cryph::Matrix4x4::perspective(zpp, -halfDx, halfDx, -halfDy, halfDy, zmin, zmax);
	}
	else if (projType == ORTHOGONAL)
		ec_lds = cryph::Matrix4x4::orthogonal(-halfDx, halfDx, -halfDy, halfDy, zmin, zmax);
	else // (projType == OBLIQUE)
		ec_lds = cryph::Matrix4x4::oblique(zpp, -halfDx, halfDx, -halfDy, halfDy, zmin, zmax,
			ModelView::obliqueProjectionDir);
	ecDeltaZ = zmax - zmin;
}
