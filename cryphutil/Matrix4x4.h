// Matrix4x4.h -- 4x4 Matrices
// This is OPEN SOURCE software developed by James R. Miller (jrmiller@ku.edu)
// Original version: ca. 1996. See README_*.txt for more information.

// See comments at start of Matrix3x3.h

#ifndef MATRIX4x4_H
#define MATRIX4x4_H

#include <iostream>

#include "AffPoint.h"
#include "AffVector.h"

namespace cryph
{

class Matrix4x4
{
public:
	Matrix4x4(); // Identity Matrix
	Matrix4x4(const Matrix4x4& m);
	Matrix4x4(
		double m11, double m12, double m13, double m14,
		double m21, double m22, double m23, double m24,
		double m31, double m32, double m33, double m34,
		double m41, double m42, double m43, double m44);

	virtual ~Matrix4x4();

	// class "Factory" methods to create 4x4 matrices:
	// 1. from arrays of length 16:
	static Matrix4x4 fromColMajor(const float* m);  // 'm' assumed to be [16]
	static Matrix4x4 fromColMajor(const double* m); // 'm' assumed to be [16]
	static Matrix4x4 fromRowMajor(const float* m);  // 'm' assumed to be [16]
	static Matrix4x4 fromRowMajor(const double* m); // 'm' assumed to be [16]
	// 2. Rotation affine transformations:
	static Matrix4x4 xRotationDegrees(double angle);
	static Matrix4x4 yRotationDegrees(double angle);
	static Matrix4x4 zRotationDegrees(double angle);
	static Matrix4x4 xRotationRadians(double angle);
	static Matrix4x4 yRotationRadians(double angle);
	static Matrix4x4 zRotationRadians(double angle);
	// 3. Translation affine transformation
	static Matrix4x4 translation(const AffVector& trans);
	// 4. Scale affine transformation
	static Matrix4x4 scale(double sx, double sy, double sz);
	// 5. 3D Viewing interfaces
	static Matrix4x4 lookAt(const cryph::AffPoint& eye,
							const cryph::AffPoint& center,
							const cryph::AffVector& up);
	static Matrix4x4 orthogonal(double xmin, double xmax, double ymin, double ymax,
				double zmin, double zmax);
	static Matrix4x4 oblique(double zpp, double xmin, double xmax, double ymin,
				double ymax, double zmin, double zmax,
				const cryph::AffVector& projDir);
	static Matrix4x4 perspective(double zpp, double xmin, double xmax,
			double ymin, double ymax, double zmin, double zmax);

	// A class method routine primarily designed for internal use while
	// implementing lookAt, but useful by clients if they want to
	// ensure that a given set of parameters are valid. (The bool
	// return is true if and only if the output "v" and "w" parameters
	// have been correctly computed on return.)
	static bool getECvw(const cryph::AffPoint& eye,
			const cryph::AffPoint& center, const cryph::AffVector& up,
			cryph::AffVector& v, cryph::AffVector& w);
	// END: class "Factory" methods to create 4x4 matrices

	// Instance methods

	Matrix4x4 operator=(const Matrix4x4& rhs);

	Matrix4x4 operator*=(const Matrix4x4& rhs);
	Matrix4x4 operator*=(double f);
	Matrix4x4 operator+=(const Matrix4x4& rhs);

	AffPoint operator*(const AffPoint& p) const;
	AffVector operator*(const AffVector& v) const;

	Matrix4x4 operator*(const Matrix4x4& m2) const;
	Matrix4x4 operator+(const Matrix4x4& m2) const;
	Matrix4x4 operator-(const Matrix4x4& m2) const;

	friend Matrix4x4 operator*(double f, const Matrix4x4& m);
	friend std::ostream& operator<<(std::ostream& os, const Matrix4x4& m);
	friend std::istream& operator>>(std::istream& is, Matrix4x4& m);

	double elementAt(int i, int j) const;

	// Extraction routines useful in OpenGL, OpenInventor, etc.
	float* extractColMajor(float m[16]) const;
	double* extractColMajor(double m[16]) const;
	float* extractRowMajor(float m[16]) const;
	double* extractRowMajor(double m[16]) const;

	// In the following two multiply routines, nElements describes the
	// length of the 'a' and 'b' arrays. If nElements<4, then (i) a[3]
	// is assumed to be 1.0, and (ii) a[i] for nElements<=i<3 are
	// assumed to be zero. nElements cannot be larger than 4.
	// Only nElements positions of 'b' will be set.
	void multiply(const double a[], double b[], int nElements=4) const;
	void multiply(const float a[], float b[], int nElements=4) const;

	void setElementAt(int i, int j, double newValue);


    // ---------- Global constants

	// 1. Special Matrices
	static const Matrix4x4 IdentityMatrix;
	static const Matrix4x4 ZeroMatrix;

protected:
	double mElem[4][4];

private:
	void copy(const Matrix4x4& rhs);
};

}

#endif
