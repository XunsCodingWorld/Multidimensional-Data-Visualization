// PCA.h -- This class was derived from pca_eigen.cpp, downloaded 2014/02/11 from:
// http://codingplayground.blogspot.com/2010/01/pca-dimensional-reduction-in-eigen.html

#ifndef PCA_H
#define PCA_H

#include <vector>
#include "Eigen/Core"
#include "Eigen/Eigen"

typedef std::pair<double, int> myPair;
typedef std::vector<myPair> PermutationIndices;

class PCA
{
public:
	// In following, each row is a sample; columns are dimensions
	PCA(float** vbls, int nDimensionsIn, int nSamplesIn);
	virtual ~PCA();

	// i=0 ==> largest; i==1 ==> next largest; etc.
	void getIthLargestEigenValueEigenVector(int i, float& eigenValue, float* eigenVector) const;
	int getNumDimensions() const { return nDimensions; }
	int getNumSamples() const { return nSamples; }

	static void setDebug(bool b) { debug = b; }

protected:
	// Useful only for subclasses that are going to fill the MatrixXd
	// instance in a different way and then call finishConstruction.
	PCA(int nDimensionsIn, int nSamplesIn);
	int nDimensions, nSamples;
	void finishConstruction(Eigen::MatrixXd& DataPoints);

	static bool debug;

private:
	PermutationIndices pi;
	Eigen::MatrixXd eigenVectors;
};

#endif
