// ModelView.h - abstract base class for models managed by a Controller

#ifndef MODELVIEW_H
#define MODELVIEW_H

#include <string>
#include <GL/gl.h>

#include "AffPoint.h"
#include "AffVector.h"
#include "Matrix4x4.h"
#include "ProjectionType.h"

class ModelView
{
public:
	ModelView();
	virtual ~ModelView();
	
	virtual void getMCBoundingBox(double* xyzLimits) const = 0; // { xmin, xmax, ymin, ymax, zmin, zmax }
	virtual void handleCommand(unsigned char key, double ldsX, double ldsY);
	virtual void handleCommand(unsigned char key, int num, double ldsX, double ldsY);
	virtual void handleSpecialKey(int key, double ldsX, double ldsY) { }
	virtual bool picked(double ldsX, double ldsY) { return false; }
	virtual void render() = 0;

	// common 3D global (i.e., applies to entire scene) dynamic viewing requests
	static void addToGlobalPan(double dxInLDS, double dyInLDS, double dzInLDS);
	static void addToGlobalRotationDegrees(double rx, double ry, double rz);
	static void addToGlobalZoom(double increment);  // scale += increment
	static void resetGlobalDynamic(); // rotation and pan
	static void resetGlobalZoom();
	static void scaleGlobalZoom(double multiplier); // scale *= multiplier
	static void setAspectRatioPreservationEnabled(bool b) { aspectRatioPreservationEnabled = b; }
	static void setEyeCenterUp(cryph::AffPoint E, cryph::AffPoint C, cryph::AffVector up);
	// setFractionalDistEyeToCenterOfRotation: 0=>about eye; 1=>about center of attention
	static void setFractionalDistEyeToCenterOfRotation(double f);
	static void setObliqueProjectionDirection(const cryph::AffVector& dir);
	static void setProjection(ProjectionType pType);
	static void setProjectionPlaneZ(double zppIn);
	static void setUseGlobalZoomIn2D(bool b) { useGlobalZoomIn_computeScaleTrans = b; }

	virtual void printKeyboardKeyList(bool firstCall) const;
protected:
	GLenum polygonMode;

	// Following method is well-suited for concrete ModelView subclasses
	// used in 2D applications
	static void computeScaleTrans(float* sclTrans);
	// Following method is well-suited for concrete ModelView subclasses
	// used in 3D applications (or 2D applications with zoom/pan)
	static void getMatrices(cryph::Matrix4x4& mc_ec_fullOut,
							cryph::Matrix4x4& ec_ldsOut);
	static void linearMap(double fromMin, double fromMax, double toMin, double toMax,
		double& scale, double& trans);
	// "pp": "per-primitive"; "pv": "per-vertex"
	static GLint ppUniformLocation(GLuint glslProgram, const std::string& name);
	static GLint pvAttribLocation(GLuint glslProgram, const std::string& name);
	
public:
	static double mcXMinRegionOfInterest, mcXMaxRegionOfInterest,
		mcYMinRegionOfInterest, mcYMaxRegionOfInterest,
		mcZMinRegionOfInterest, mcZMaxRegionOfInterest;
	// data derived from previous (see also important comments in
	// ModelView.c++ where these are declared and initialized):
	static double mcDeltaX, mcDeltaY, mcDeltaZ;
	static double ecDeltaX, ecDeltaY, ecDeltaZ, ecDeltaMax;
	// for dynamic pan/scale update notifications from Controller
	// (rotations are integrated directly into "dynamic" below)
	static double scale; // for global zoom
	static bool useGlobalZoomIn_computeScaleTrans;
	// View orientation
	struct EyeCoordSystemSpec
	{
		cryph::AffPoint eye, center;
		cryph::AffVector up;
		double distEyeCenter;

		EyeCoordSystemSpec() : eye(0,0,4), center(0,0,0), up(0,1,0),
			distEyeCenter(4)
		{}
	};
	static EyeCoordSystemSpec curEC, origEC;

	static double fractionOfDistEyeCenterToCenterOfRotation;
	static cryph::AffVector mcPanVector2D;
	// Projections
	static ProjectionType projType;
	static cryph::AffVector obliqueProjectionDir;
	static double zpp;
	static bool aspectRatioPreservationEnabled;
	// matrix variables and utilities
	static cryph::Matrix4x4 dynamic; // mouse-based dynamic 3D rotations and pan
	static cryph::Matrix4x4 lookAtMatrix;
	static cryph::Matrix4x4 mc_ec_full; // = rotation * lookAtMatrix
	static cryph::Matrix4x4 ec_lds; // wv * proj

	static void establishLightsAndView();
	static void set_ecDeltas();
	static void setProjectionTransformation();
	// end matrix utilities

	// EXPERIMENTAL: Translating dynamic rotations to eye-center-up
	static bool translateDynamicRotationToEyeUp;
	static bool translateDynamicPanToEyeCenter;
	static cryph::Matrix4x4 originalVOM;
	static bool haveOriginalVOM;
	// END: EXPERIMENTAL
};

#endif
