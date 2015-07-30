// BasicShape.h - Interface for common shapes in OpenGL

#ifndef BASICSHAPE_H
#define BASICSHAPE_H

#include <GL/gl.h>
#include <GL/freeglut.h>

#include "AffPoint.h"
#include "AffVector.h"

class BasicShape
{
public:
	virtual ~BasicShape();

	enum Caps { CAP_AT_NEITHER, CAP_AT_BOTTOM, CAP_AT_TOP, CAP_AT_BOTH };

	// Factory methods for creation of shapes:
	// Texture coordinates are generated only if BOTH sMax>sMin AND tMax>tMin

	// 1. Cylinders and Cones:
	// 's' direction is around the circumference of the cylinder/cone; 't' direction
	// is along the axis. "nPointsAroundSide" must be at least 3. "nPointsAlongAxis"
	// must be at least 2. If invalid input is given (e.g., negative radii, coincident
	// points, bad "nPoints*" specs, etc.), NULL is returned.
	// The component of the optional vector "sZero" that is perpendicular to the "axis"
	// will define the direction for texture s coordinate=0. It serves no other
	// externally significant purpose. If the component of "sZero" perpendicular to the
	// axis is the zero vector, an arbitrary vector perpendicular to the axis is used.
	static BasicShape* makeBoundedCone(
		const cryph::AffPoint& Pbottom, const cryph::AffPoint& Ptop,
		double radiusAtBottom, double radiusAtTop,
		int nPointsAroundSide, int nPointsAlongAxis,
		Caps capSpec=CAP_AT_BOTH,
		double sMin=0, double sMax=0, double tMin=0, double tMax=0,
		const cryph::AffVector& sZero=cryph::AffVector::xu);
	
	static BasicShape* makeBoundedCylinder(
		const cryph::AffPoint& Pbottom, const cryph::AffPoint& Ptop,
		double radius,
		int nPointsAroundSide, int nPointsAlongAxis,
		Caps capSpec=CAP_AT_BOTH,
		double sMin=0, double sMax=0, double tMin=0, double tMax=0,
		const cryph::AffVector& sZero=cryph::AffVector::xu);

	// 2. Spheres:
	// 's' and 't' directions are analogous to those for cylinder. (See parameter "upAxis".)
	// nPointsAlongAxis must be at least 3. In general, an odd number will work best because
	// that forces points along the equator to be generated.
	static BasicShape* makeSphere(
		const cryph::AffPoint& center, double radius,
		int nPointsAroundSide, int nPointsAlongAxis,
		double sMin=0, double sMax=0, double tMin=0, double tMax=0,
		const cryph::AffVector& upAxis=cryph::AffVector::yu, // corresponds to 'axis' in cyl/cone
		const cryph::AffVector& sZero=cryph::AffVector::xu);

	// 3. Blocks (a 3D parallelepiped)
	// If any of the lengths (uLength, vLength, wLength) is less than or equal to zero,
	// we use the length of the corresponding vector instead. If uEdge is a zero vector,
	// no shape will be created. If vEdge is either zero or parallel to uEdge,
	// no shape will be created. If wEdge is either zero or in the plane spanned
	// by (uEdge, vEdge), then wEdge will be generated as uEdge x vEdge.
	// Texture coordinates are not currently generated on Blocks, but each face
	// of the block will be individually drawn, hence, knowing the order in which the
	// vertices are stored and the faces are drawn, it is easy for callers to add
	// texture to any or all of the faces. The points are stored as:
	// { (umin,vmin,wmin), (umin,vmin,wmax), (umax,vmin,wmin), (umax,vmin,wmax),
	//   (umax,vmax,wmin), (umax,vmax,wmax), (umin,vmax,wmin), (umin,vmax,wmax) }
	// The three glDrawArrays calls will be made, in order, on (1) vmin face,
	// (2) umax face, (3) vmax face. The three glDrawElements calls will be made,
	// in order, on (1) umin face, (2) wmin face, (3) wmax face.
	static BasicShape* makeBlock(
		const cryph::AffPoint& llCorner,
		const cryph::AffVector& uEdge, double uLength,
		const cryph::AffVector& vEdge, double vLength,
		const cryph::AffVector& wEdge, double wLength);

	// Determine whether factory methods will always generate per-vertex normals
	// (hence generating more point and normal buffer data), or whether they
	// will use index lists and fixed normals when possible for flat faces.
	static bool getAlwaysGeneratePerVertexNormals()
		{ return alwaysGeneratePerVertexNormals; }
	static void setAlwaysGeneratePerVertexNormals(bool b)
		{ alwaysGeneratePerVertexNormals = b; }
	
	// Accessors for use during transmission of definition to GPU (e.g., glBufferData, et al.)
	// 1. number of points, normals, and (if applicable) texture coords:
	int getNumPoints() const { return nPoints; }
	// 2. The actual point, normal, and texture coordinates to be passed to glBufferData:
	const float* getPointCoords() const { return pointCoords; }
	const float* getNormals() const { return normals; }
	const float* getTextureCoords() const { return textureCoords; }

	// Accessors for use while rendering during display callbacks. Client code should always
	// try to access BOTH the "draw arrays" data AND the "index list" data. Zero is returned
	// from the two "int getXxx" calls below if there is no data of that type available for
	// the BasicShape instance.

	// This instance may have data for one or more glDrawArrays calls:
	int getNumDrawArraysCalls() const { return nDrawArraysCalls; }
	// func return is num points in call:
	int getDrawArraysData(int i, GLenum& mode, int& offset,
			bool& canUsePerVertexTexCoords, bool& canUseNormalArray,
			// if there are no per-vertex normals, the following is the per-primitive normal:
			cryph::AffVector& fixedN) const;
	// This instance may also (or instead) have one or more sets of data for glDrawElements calls:
	int getNumIndexLists() const { return nIndexLists; }
	// for each index list, make a glDrawElements call using the following retrieved data:
	const void* getIndexList(int i, GLenum& mode, int& nInList, GLenum& type,
			// following tells you whether the corresponding vertex arrays should be enabled:
			bool& canUsePerVertexTexCoords, bool& canUsePerVertexNormals,
			// if there are no per-vertex normals, the following is the per-primitive normal:
			cryph::AffVector& fixedNormal) const;

	void getMCBoundingBox(double* xyzLimits) const;

private:
	BasicShape(); // use public factory methods instead
	BasicShape(const BasicShape& s) {} // don't make copies

	// common "worker" routines for cones and cylinders:
	void addCapsFixedNormal(Caps capSpec, int nPointsInCap,
			const cryph::AffVector& axis, int nPointsAlongAxis);
	void addCapsPerVertexNormals(Caps capSpec, int nPointsAlongAxis,
			int firstAtBottom, int firstAtTop, int nPointsInOneCap);
	void addDrawArraysDataForConeCylCaps(int nextIndex,
			int firstAtBottom, int firstAtTop, int nPointsInOneCap);
	void allocateIndexListsForConeCyl(int nPointsAlongAxis, Caps capSpec);
	void allocateIndexListsForSphere(int nPointsAlongAxis);
	void makeBlockData(
		const cryph::AffPoint& llCorner,
		const cryph::AffVector& uEdge, double uLength,
		const cryph::AffVector& vEdge, double vLength,
		const cryph::AffVector& wEdge, double wLength);
	void finishBlockDataUsingFixedNormals(
		const cryph::AffVector& uvNormal, const cryph::AffVector& uwNormal,
		const cryph::AffVector& vwNormal, const cryph::AffPoint* vertices);
	void finishBlockDataUsingPerVertexNormals(
		const cryph::AffVector& uvNormal, const cryph::AffVector& uwNormal,
		const cryph::AffVector& vwNormal, const cryph::AffPoint* vertices);
	void makeDrawArraysDataForConeCyl(int nPointsWithoutCaps,
		int firstAtBottom, int firstAtTop); // if nPointsAlongAxis == 2
	void makeEightBlockVertices(
		const cryph::AffPoint& llCorner,
		const cryph::AffVector& u, double uLength,
		const cryph::AffVector& v, double vLength,
		const cryph::AffVector& w, double wLength,
		cryph::AffPoint* vertices);
	void makeIndexLists(int nPointsAroundSide, int nPointsAlongAxis);
	void makeRuledSurfaceBetweenCircles(
		const cryph::AffPoint& Pbottom, cryph::AffVector& axis,
		double height, double radiusAtBottom, double radiusAtTop,
		int nPointsAroundSide, int nPointsAlongAxis,
		Caps capSpec, double sMin, double sMax, double tMin, double tMax,
		const cryph::AffVector& sZero);
	void makeSphere(const cryph::AffPoint& center,
					const cryph::AffVector& u, const cryph::AffVector& v, const cryph::AffVector& w,
					double radius,
					int nPointsAroundSide, int nPointsAlongAxis,
					double sMin, double sMax, double tMin, double tMax);
	static void updateOneCoordLimit(double c, double minMax[2]);
	void updateXYZMinMaxLimits(const cryph::AffPoint& p);

	// geometry and per-vertex attributes
	float* pointCoords;
	float* normals;
	float* textureCoords;
	int nPoints;
	// always generate per-vertex normals, even for flat faces?
	static bool alwaysGeneratePerVertexNormals;
	// for bounding box
	double xyzMinMax[6];

	struct DrawArraysCallData
	{
		GLenum mode;
		int offset;
		int nPointsInCall;
		bool usePerVertexTexCoords, usePerVertexNormals;
		cryph::AffVector useFixedNormal; // if !usePerVertexNormals
	};
	DrawArraysCallData* drawArraysCallData;
	int nDrawArraysCalls;

	struct IndexListData
	{
		GLenum mode;
		int* indices;
		int nIndices;
		bool usePerVertexTexCoords, usePerVertexNormals;
		cryph::AffVector useFixedNormal; // if !usePerVertexNormals
	};
	IndexListData* indexLists;
	int nIndexLists, nIndexListsConstructed;
};

#endif
