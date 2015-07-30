// PCA.c++ -- This class was derived from pca_eigen.cpp, downloaded 2014/02/11 from:
// http://codingplayground.blogspot.com/2010/01/pca-dimensional-reduction-in-eigen.html

#include <iostream>

#include "PCA.h"
using namespace Eigen;

bool PCA::debug = false;

// In following, each row is a sample; columns are dimensions
PCA::PCA(float** vbls, int nDimensionsIn, int nSamplesIn) :
	nDimensions(nDimensionsIn), nSamples(nSamplesIn)
{
	//                                   ROWS:        COLS:
	MatrixXd DataPoints = MatrixXd::Zero(nDimensions, nSamples);
	for (int d=0 ; d<nDimensions ; d++)
	{
		for (int s=0 ; s<nSamples ; s++)
		{
			DataPoints(d,s) = vbls[s][d];
		}
	}
	finishConstruction(DataPoints);
}

// The following is only inteded for subclasses that will call
// finishConstruction themselves.
PCA::PCA(int nDimensionsIn, int nSamplesIn) :
    nDimensions(nDimensionsIn), nSamples(nSamplesIn)
{
}

void PCA::finishConstruction(MatrixXd& DataPoints)
{
	double mean;
	VectorXd meanVector;

	// for each point
	//   center the point with the mean among all the coordinates
	for (int i = 0; i < DataPoints.rows(); i++)
	{
		mean = (DataPoints.row(i).sum())/nSamples; //compute mean in this dimension
		if (debug)
			std::cout << "Mean in dimension " << i << " is " << mean << '\n';
		meanVector  = VectorXd::Constant(nSamples,mean); // create a vector with constant value = mean
		DataPoints.row(i) -= meanVector;
	}

	// get the covariance matrix
	MatrixXd Covariance = MatrixXd::Zero(nDimensions, nDimensions);
	Covariance = (1 / (double) nSamples) * DataPoints * DataPoints.transpose();
	if (debug)
		std::cout << "Covariance matrix:\n" << Covariance;	

	// compute the eigenvalues of the Cov Matrix
	EigenSolver<MatrixXd> m_solve(Covariance);
	VectorXd eigenvalues = VectorXd::Zero(nDimensions);
	eigenvalues = m_solve.eigenvalues().real();

	eigenVectors = MatrixXd::Zero(nSamples, nDimensions);  // matrix (n x m) (points, dims)
	eigenVectors = m_solve.eigenvectors().real();	

	// sort and get the permutation indices
	for (int i = 0 ; i < nDimensions; i++)
		pi.push_back(std::make_pair(eigenvalues(i), i));

	if (debug)
	{
		unsigned int largest = pi.size()-1;
		unsigned int k = 4;
		std::cout << "\nBEFORE SORT\n";
		for (unsigned int i = 0; i < k; i++)
		{
			std::cout << i << "-eigenvector has eigenvalue: "
			          << pi[i].first << ", "
			          << eigenVectors.col(i) << "\n";
		}

		for (unsigned int i = 0; i < nDimensions ; i++)
			std::cout << "eigen=" << pi[i].first << " pi=" << pi[i].second << std::endl;
	}

	sort(pi.begin(), pi.end());

	if (debug)
	{
		std::cout << "\nAFTER SORT\n";

		// consider the subspace corresponding to the top-k eigenvectors
		unsigned int largest = pi.size()-1;
		unsigned int k = 4;
		for (unsigned int i = 0; i < k; i++)
		{
			int piLoc = largest - i;
			std::cout << pi[piLoc].second << "-eigenvector has eigenvalue: "
			          << pi[piLoc].first << ", "
			          << eigenVectors.col(pi[piLoc].second) << "\n";
		}

		for (unsigned int i = 0; i < nDimensions ; i++)
			std::cout << "eigen=" << pi[i].first << " pi=" << pi[i].second << std::endl;
	}
}

void PCA::getIthLargestEigenValueEigenVector(int i, float& eigenValue, float* eigenVector) const
{
	if ((i < 0) || (i >= nDimensions))
		return;
	int piLoc = pi.size()-1-i;
	eigenValue = pi[piLoc].first;
	for (int i=0 ; i<nDimensions ; i++)
		eigenVector[i] = eigenVectors.col(pi[piLoc].second)(i);
}

PCA::~PCA()
{
}
