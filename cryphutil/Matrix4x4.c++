// Matrix4x4.c++ -- 4x4 transformation matrices
// This is OPEN SOURCE software developed by James R. Miller (jrmiller@ku.edu)
// Original version: ca. 1996. See README_*.txt for more information.

#include <stdlib.h>
#include <math.h>

#include <iomanip>
using namespace std;

#include "Inline.h"
#include "Tolerances.h"

#include "Matrix4x4.h"

namespace cryph
{

// ---------- public Global constants

// 1. Special Matrices
const Matrix4x4 Matrix4x4::IdentityMatrix =
	Matrix4x4(
		1.0 , 0.0 , 0.0 , 0.0 ,
		0.0 , 1.0 , 0.0 , 0.0 ,
		0.0 , 0.0 , 1.0 , 0.0 ,
		0.0 , 0.0 , 0.0 , 1.0
		);

const Matrix4x4 Matrix4x4::ZeroMatrix =
	Matrix4x4(
		0.0 , 0.0 , 0.0 , 0.0 ,
		0.0 , 0.0 , 0.0 , 0.0 ,
		0.0 , 0.0 , 0.0 , 0.0 ,
		0.0 , 0.0 , 0.0 , 0.0
		);

// END: public Global constants

// Following used simply to make it obvious where we are assuming one
// dimension or another:
const int DIM3 = 3;
const int DIM4 = 4;

Matrix4x4::Matrix4x4()
{
	this->copy(Matrix4x4::IdentityMatrix);
}

Matrix4x4::Matrix4x4(const Matrix4x4& m)
{
	this->copy(m);
}

Matrix4x4::Matrix4x4(
	double m11, double m12, double m13, double m14,
	double m21, double m22, double m23, double m24,
	double m31, double m32, double m33, double m34,
	double m41, double m42, double m43, double m44)
{
	mElem[0][0] = m11; mElem[0][1] = m12; mElem[0][2] = m13; mElem[0][3] = m14;
	mElem[1][0] = m21; mElem[1][1] = m22; mElem[1][2] = m23; mElem[1][3] = m24;
	mElem[2][0] = m31; mElem[2][1] = m32; mElem[2][2] = m33; mElem[2][3] = m34;
	mElem[3][0] = m41; mElem[3][1] = m42; mElem[3][2] = m43; mElem[3][3] = m44;
}

Matrix4x4::~Matrix4x4()
{
}

void Matrix4x4::copy(const Matrix4x4& rhs)
{
	for (int i=0 ; i<DIM4 ; i++)
		for (int j=0 ; j<DIM4 ; j++)
			this->mElem[i][j] = rhs.mElem[i][j];
}

double Matrix4x4::elementAt(int i, int j) const
{
	if ( (i < 4) && (j < 4) )
		return mElem[i][j];

	return -99999.0;
}

float* Matrix4x4::extractColMajor(float m[16]) const
{
	int		k = 0;
	for (int col=0 ; col<DIM4 ; col++)
		for (int row=0 ; row<DIM4 ; row++)
			m[k++] = static_cast<float>(this->mElem[row][col]);
	return &m[0];
}

double* Matrix4x4::extractColMajor(double m[16]) const
{
	int		k = 0;
	for (int col=0 ; col<DIM4 ; col++)
		for (int row=0 ; row<DIM4 ; row++)
			m[k++] = this->mElem[row][col];
	return &m[0];
}

float* Matrix4x4::extractRowMajor(float m[16]) const
{
	int		k = 0;
	for (int row=0 ; row<DIM4 ; row++)
		for (int col=0 ; col<DIM4 ; col++)
			m[k++] = static_cast<float>(this->mElem[row][col]);
	return &m[0];
}

double* Matrix4x4::extractRowMajor(double m[16]) const
{
	int		k = 0;
	for (int row=0 ; row<DIM4 ; row++)
		for (int col=0 ; col<DIM4 ; col++)
			m[k++] = this->mElem[row][col];
	return &m[0];
}

Matrix4x4 Matrix4x4::fromColMajor(const float* m)
{
	Matrix4x4 M;
	int k = 0;
	for (int col=0 ; col<DIM4 ; col++)
		for (int row=0 ; row<DIM4 ; row++)
			M.mElem[row][col] = m[k++];
	return M;
}

Matrix4x4 Matrix4x4::fromColMajor(const double* m)
{
	Matrix4x4 M;
	int k = 0;
	for (int col=0 ; col<DIM4 ; col++)
		for (int row=0 ; row<DIM4 ; row++)
			M.mElem[row][col] = m[k++];
	return M;
}

Matrix4x4 Matrix4x4::fromRowMajor(const float* m)
{
	Matrix4x4 M;
	int k = 0;
	for (int row=0 ; row<DIM4 ; row++)
		for (int col=0 ; col<DIM4 ; col++)
			M.mElem[row][col] = m[k++];
	return M;
}

Matrix4x4 Matrix4x4::fromRowMajor(const double* m)
{
	Matrix4x4 M;
	int k = 0;
	for (int row=0 ; row<DIM4 ; row++)
		for (int col=0 ; col<DIM4 ; col++)
			M.mElem[row][col] = m[k++];
	return M;
}

bool Matrix4x4::getECvw(const cryph::AffPoint& eye,
        const cryph::AffPoint& center, const cryph::AffVector& up,
		cryph::AffVector& v, cryph::AffVector& w)
{
	w = eye - center;
	if (w.normalize() < BasicDistanceTol)
	{
		std::cerr << "Matrix4x4::getECvw: eye and center are coincident.\n";
		return false;
	}
	AffVector par;
	w.decompose(up,par, v);
	if (v.normalize() < BasicUnitTol)
	{
		std::cerr << "Matrix4x4::getECvw: 'up' is zero or parallel to line of sight.\n";
		return false;
	}
	return true;

}

Matrix4x4 Matrix4x4::lookAt(const cryph::AffPoint& eye,
		const cryph::AffPoint& center, const cryph::AffVector& up)
{
	AffVector v, w;
	if (!getECvw(eye, center, up, v, w))
		return IdentityMatrix;
	AffVector u = v.cross(w);
	u.normalize();
	double tx = -u.dot(eye);
	double ty = -v.dot(eye);
	double tz = -w.dot(eye);
	return Matrix4x4(
	               u[DX],u[DY],u[DZ], tx,
				   v[DX],v[DY],v[DZ], ty,
				   w[DX],w[DY],w[DZ], tz,
	                 0.0,  0.0,  0.0, 1.0);
}

void Matrix4x4::multiply(const double a[], double b[], int nElements) const
{
	if (nElements < 1)
		return;
	if (nElements > 4)
		nElements = 4;
	for (int i=0 ; i<nElements ; i++)
	{
		double sum = 0.0;
		for (int j=0 ; j<4 ; j++)
		{
			double aj = 0.0;
			if (j < nElements)
				aj = a[j];
			else if (j == 3)
				aj = 1.0; // w
			sum += mElem[i][j]*aj;
		}
		b[i] = sum;
	}
}

void Matrix4x4::multiply(const float a[], float b[], int nElements) const
{
	if (nElements < 1)
		return;
	if (nElements > 4)
		nElements = 4;
	for (int i=0 ; i<nElements ; i++)
	{
		float sum = 0.0;
		for (int j=0 ; j<4 ; j++)
		{
			double aj = 0.0;
			if (j < nElements)
				aj = a[j];
			else if (j == 3)
				aj = 1.0; // w
			sum += static_cast<float>(mElem[i][j]*aj);
		}
		b[i] = sum;
	}
}

Matrix4x4 Matrix4x4::oblique(double zpp, double xmin, double xmax,
	                         double ymin, double ymax, double zmin, double zmax,
                             const cryph::AffVector& projDir)
{
	if ((xmin >= xmax) || (ymin >= ymax) || (zmin >= zmax))
	{
		std::cerr << "Matrix4x4::oblique: one or more min values >= max.\n";
		return IdentityMatrix;
	}
	double pDZ = projDir[DZ];
	if (fabs(pDZ) < BasicUnitTol)
	{
		std::cerr << "Matrix4x4::oblique: invalid projection direction: " << projDir << '\n';
		return IdentityMatrix;
	}
	double dx = xmax - xmin;
	double dy = ymax - ymin;
	double dz = zmin - zmax; // note swapped order
	double pDX = projDir[DX];
	double pDY = projDir[DY];
	return cryph::Matrix4x4(
		2.0/dx, 0.0, -2.0*pDX/(pDZ*dx), 2.0*zpp*pDX/(pDZ*dx) - (xmin+xmax)/dx,
		0.0, 2.0/dy, -2.0*pDY/(pDZ*dy), 2.0*zpp*pDY/(pDZ*dy) - (ymin+ymax)/dy,
		0.0, 0.0, 2.0/dz, -(zmin + zmax)/dz,
		0.0, 0.0, 0.0, 1.0
	);
}

Matrix4x4 Matrix4x4::operator=(const Matrix4x4& rhs)
{
	this->copy(rhs);
	return *this;
}

Matrix4x4 Matrix4x4::operator*=(const Matrix4x4& rhs)
{
	*this = (*this) * rhs;
	return *this;
}

Matrix4x4 Matrix4x4::operator*=(double f)
{
	for (int i=0 ; i<DIM4 ; i++)
		for (int j=0 ; j<DIM4 ; j++)
			this->mElem[i][j] *= f;
	return *this;
}

Matrix4x4 Matrix4x4::operator+=(const Matrix4x4& rhs)
{
	for (int i=0 ; i<DIM4 ; i++)
		for (int j=0 ; j<DIM4 ; j++)
			this->mElem[i][j] += rhs.mElem[i][j];
	return *this;
}

AffPoint Matrix4x4::operator*(const AffPoint& p) const
{
	double	c[4] = { p[X] , p[Y] , p[Z], 1.0 };
	double	d[4];

	for (int i=0 ; i<4 ; i++)
	{
		double sum = 0.0;
		for (int j=0 ; j<4 ; j++)
			sum += (this->mElem[i][j] * c[j]);
		d[i] = sum;
	}

	if (fabs(d[3]) > BasicDistanceTol)
		return AffPoint(d[0]/d[3],d[1]/d[3],d[2]/d[3]);

	return AffPoint(d[0],d[1],d[2]);
}

AffVector Matrix4x4::operator*(const AffVector& v) const
{
	double	c[3] = { v[DX] , v[DY] , v[DZ] };
	double	d[3];

	for (int i=0 ; i<3 ; i++)
	{
		double sum = 0.0;
		for (int j=0 ; j<3 ; j++)
			sum += (this->mElem[i][j] * c[j]);
		d[i] = sum;
	}
	return AffVector(d[0],d[1],d[2]);
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& m2) const
{
	Matrix4x4 mOut;

	for (int i=0 ; i<DIM4 ; i++)
		for (int j=0 ; j<DIM4 ; j++)
		{
			double sum = 0.0;
			for (int k=0 ; k<DIM4 ; k++)
				sum += mElem[i][k]*m2.mElem[k][j];
			mOut.mElem[i][j] = sum;
		}

	return mOut;
}

Matrix4x4 Matrix4x4::operator+(const Matrix4x4& m2) const
{
	Matrix4x4	result(*this);

	for (int i=0 ; i<DIM4 ; i++)
		for (int j=0 ; j<DIM4 ; j++)
			result.mElem[i][j] += m2.mElem[i][j];
	return result;
}

Matrix4x4 Matrix4x4::operator-(const Matrix4x4& m2) const
{
	Matrix4x4	result(*this);

	for (int i=0 ; i<DIM4 ; i++)
		for (int j=0 ; j<DIM4 ; j++)
			result.mElem[i][j] -= m2.mElem[i][j];
	return result;
}

Matrix4x4 operator*(double f, const Matrix4x4& m) // external friend function
{
	Matrix4x4	result(m);

	for (int i=0 ; i<DIM4 ; i++)
		for (int j=0 ; j<DIM4 ; j++)
			result.mElem[i][j] *= f;
	return result;
}

ostream& operator<<(ostream& os, const Matrix4x4& m)
{
	for (int i=0 ; i<DIM4 ; i++)
	{
		for (int j=0 ; j<DIM4 ; j++)
		{
			os << m.mElem[i][j] << ' ';
		}
	}
	return os;
}

istream& operator>>(istream& is, Matrix4x4& m)
{
	for (int i=0 ; i<DIM4 ; i++)
	{
		for (int j=0 ; j<DIM4 ; j++)
		{
			is >> m.mElem[i][j];
		}
	}
	return is;
}

Matrix4x4 Matrix4x4::orthogonal(double xmin, double xmax, double ymin,
	                            double ymax, double zmin, double zmax)
{
	if ((xmin >= xmax) || (ymin >= ymax) || (zmin >= zmax))
	{
		std::cerr << "Matrix4x4::orthogonal: one or more min values >= max.\n";
		return IdentityMatrix;
	}
	return Matrix4x4(
		2.0/(xmax-xmin), 0.0, 0.0, -(xmax+xmin)/(xmax-xmin),
		0.0, 2.0/(ymax-ymin), 0.0, -(ymax+ymin)/(ymax-ymin),
		0.0, 0.0, -2.0/(zmax-zmin), (zmax+zmin)/(zmax-zmin),
		0.0, 0.0, 0.0, 1.0
	);
}

Matrix4x4 Matrix4x4::perspective(double zpp, double xmin, double xmax,
	                         double ymin, double ymax, double zmin, double zmax)
{
	if ((xmin >= xmax) || (ymin >= ymax) || (zmin >= zmax))
	{
		std::cerr << "Matrix4x4::perspective: one or more min values >= max.\n";
		return IdentityMatrix;
	}
	if ((zmax >= 0.0) || (zpp >= 0.0))
	{
		std::cerr << "Matrix4x4::perspective: must have zmin < zmax < 0 AND zpp < 0\n";
		return IdentityMatrix;
	}
	return Matrix4x4(
		-2.0*zpp/(xmax - xmin),                    0.0, (xmax + xmin)/(xmax - xmin),                            0.0,
		                   0.0, -2.0*zpp/(ymax - ymin), (ymax + ymin)/(ymax - ymin),                            0.0,
		                   0.0,                    0.0, (zmax + zmin)/(zmax - zmin), -2.0*zmin*zmax / (zmax - zmin),
	                       0.0,                    0.0,                        -1.0,                           0.0);
}

Matrix4x4 Matrix4x4::scale(double sx, double sy, double sz)
{
	Matrix4x4 m;
	m.mElem[0][0] = sx;
	m.mElem[1][1] = sy;
	m.mElem[2][2] = sz;
	return m;
}

void Matrix4x4::setElementAt(int i, int j, double newValue)
{
	if ( (i < 4) && (j < 4) )
		mElem[i][j] = newValue;
}

Matrix4x4 Matrix4x4::translation(const AffVector& translation)
{
	Matrix4x4 M;
	M.mElem[0][3] = translation[DX];
	M.mElem[1][3] = translation[DY];
	M.mElem[2][3] = translation[DZ];
	return M;
}

Matrix4x4 Matrix4x4::xRotationDegrees(double angle)
{
	double radians = degreesToRadians(angle);
	double c = cos(radians), s = sin(radians);
	return Matrix4x4(
		1.0, 0.0, 0.0, 0.0,
		0.0,   c,  -s, 0.0,
		0.0,   s,   c, 0.0,
		0.0, 0.0, 0.0, 1.0
		);
}

Matrix4x4 Matrix4x4::xRotationRadians(double angle)
{
	double c = cos(angle), s = sin(angle);
	return Matrix4x4(
		1.0, 0.0, 0.0, 0.0,
		0.0,   c,  -s, 0.0,
		0.0,   s,   c, 0.0,
		0.0, 0.0, 0.0, 1.0
		);
}

Matrix4x4 Matrix4x4::yRotationDegrees(double angle)
{
	double radians = degreesToRadians(angle);
	double c = cos(radians), s = sin(radians);
	return Matrix4x4(
		  c, 0.0,   s, 0.0,
		0.0, 1.0, 0.0, 0.0,
		 -s, 0.0,   c, 0.0,
		0.0, 0.0, 0.0, 1.0
		);
}

Matrix4x4 Matrix4x4::yRotationRadians(double angle)
{
	double c = cos(angle), s = sin(angle);
	return Matrix4x4(
		  c, 0.0,   s, 0.0,
		0.0, 1.0, 0.0, 0.0,
		 -s, 0.0,   c, 0.0,
		0.0, 0.0, 0.0, 1.0
		);
}

Matrix4x4 Matrix4x4::zRotationDegrees(double angle)
{
	double radians = degreesToRadians(angle);
	double c = cos(radians), s = sin(radians);
	return Matrix4x4(
		  c,  -s, 0.0, 0.0,
		  s,   c, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
		);
}

Matrix4x4 Matrix4x4::zRotationRadians(double angle)
{
	double c = cos(angle), s = sin(angle);
	return Matrix4x4(
		  c,  -s, 0.0, 0.0,
		  s,   c, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
		);
}

} // end namespace
