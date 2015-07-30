// main.c++
#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/gl.h>
#include <GL/freeglut.h>

#include "Variable.h"
#include "PCA.h"
#include "Controller.h"
#include "AxesMV.h"
#include "PointsMV.h"

void initializeViewingInformation(Controller& c)
{
	// determine the center of the scene:
	double xyz[6];
	c.getOverallMCBoundingBox(xyz);
	double xmid = 0.5 * (xyz[0] + xyz[1]);
	double ymid = 0.5 * (xyz[2] + xyz[3]);
	double zmid = 0.5 * (xyz[4] + xyz[5]);

	// a heuristic: arrange the eye and center points so that the distance
	// between them is (2*max scene dimension)
	double maxDelta = xyz[1] - xyz[0];
	double delta = xyz[3] - xyz[2];
	if (delta > maxDelta)
		maxDelta = delta;
	delta = xyz[5] - xyz[4];
	if (delta > maxDelta)
		maxDelta = delta;
	double distEyeCenter = 2.0 * maxDelta;

	cryph::AffPoint center(xmid, ymid, zmid);
	cryph::AffPoint eye(xmid, ymid, zmid + distEyeCenter);
	cryph::AffVector up = cryph::AffVector::yu;

	ModelView::setEyeCenterUp(eye, center, up);

	// Place the projection plane roughly at the front of scene.
	ModelView::setProjectionPlaneZ(-(distEyeCenter - 0.5*maxDelta));
	ModelView::setAspectRatioPreservationEnabled(false);
	ModelView::setProjection(ORTHOGONAL);
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << std::endl;
		return -1;
	}

	std::ifstream infile(argv[1]);
	
	if (!infile.good())
	{
		std::cerr << "Could not open" << argv[1] << "for reading." << std::endl;
		return -1;
	}
		
	int N; //the number of variables
	int R; // the number of data points
	int nLine(0); // the number of lines already read from the file
	
	std::string line; //line string read from the file

	int i(0),j(0);

	Variable *mylist;
	
	if (infile.is_open())
	{
		while (std::getline(infile, line))
		{
			nLine++;
			if (nLine == 1)
			{
				std::stringstream iss(line);
				if (iss >> N >> R)//process initialization
				{
					mylist = new Variable[N];
					for(i=0; i < N; i++)
					{
						mylist[i].value = new float[R];
					}
					continue;
				}
				else
				{
					std::cout << "Format Error!" << std::endl;
				}
			}
			else if (nLine <= N+1) //read from line 2 to line N+1
			{
				mylist[nLine-2].count = nLine - 1;
				mylist[nLine-2].name = line;

			}
			else if (nLine <= 2*N + 1) //read from line N+2 to line 2N+1
			{
				std::stringstream iss(line);
				iss >> mylist[nLine-N-2].minValue >> mylist[nLine-N-2].maxValue >> mylist[nLine-N-2].cardinality;
				mylist[nLine-N-2].alpha = 1/(mylist[nLine-N-2].maxValue - mylist[nLine-N-2].minValue);
				mylist[nLine-N-2].beta = - mylist[nLine-N-2].minValue/(mylist[nLine-N-2].maxValue - mylist[nLine-N-2].minValue);
			}
			else //read from line 2N+2 to line 2N+R+1
			{
				std::stringstream iss(line);
				for(i=0;i < N;i++)				
				{
					iss >> mylist[i].value[j];	
				}
				j++;
			}
		}
	}
	
	for (i = 0; i < N; i++)//calculate the mean of each variable
	{
		float sum = 0;
		for(j = 0; j < R; j++)
		{
			sum = sum + mylist[i].value[j];
		}
		mylist[i].mean = sum/R;
	}

/*	for (i = 0; i < N; i++) //test output
	{
		std::cout << mylist[i].name << std::endl;
		std::cout << mylist[i].minValue << "\t" << mylist[i].maxValue << "\t" << mylist[i].cardinality << std::endl;
		std::cout << mylist[i].alpha << "\t" << mylist[i].beta << "\t" << std::endl;
		std::cout << mylist[i].mean << std::endl;
		for (j = 0; j < R; j++)
		{
			std::cout << mylist[i].value[j] << " ";
		}
		std::cout << "\n";
	}*/

	int* varInclude = NULL; 
	int varCount;
	std::cout << "How many variables of original data set you want to use (prefer all of them):";
	std::cin >> varCount;
	varInclude = new int[varCount];
	std::cout << "\n";
	std::cout << "Please include the variables' serial number you want to use (1 - Max), separate by space, then press enter." << std::endl;
	std::cout << "include:";
	for(i = 0; i < varCount; i++)
	{
		std::cin >> varInclude[i];
	}

	Variable* newlist = new Variable[varCount];
	for(i = 0; i < varCount; i++)
	{	
		for(int w = 0; w < N; w++)
		{
			if(mylist[w].count == varInclude[i])
			{
				newlist[i].value = new float[R];
				newlist[i].alpha = mylist[w].alpha;
				newlist[i].beta = mylist[w].beta;
				for(j = 0; j < R; j++)
				{
					newlist[i].value[j] = mylist[w].value[j];
				}
			}
		}
	}

	//create a 2-dimension array to rearrange the data
	float** vals = new float*[R];
	for (j = 0 ; j < R ; j++)
		vals[j] = new float[varCount];
	
	for (i = 0; i < varCount; i++)//fetch the data from mylist to array vlas
	{
		for (j = 0; j < R; j++)
		{
			vals[j][i] = newlist[i].alpha * newlist[i].value[j] + newlist[i].beta;
		}
	}

/*	for (j = 0; j < R; j++)//to test the array vals
	{
		for (i = 0; i < N; i++)
		{
			std::cout << vals[j][i] << " ";
		}
		std::cout <<"\n";
	}
*/

	cryph::AffPoint pts[R];
	float sps[R], sz[R], crs[R];//array used to store value
	
	PCA pca(vals, N, R);
	
	float eigenValue;
	float* eigenVector = new float[N];
	
	//print eigenValues and eigenVectors
	for (int i = 0 ; i < 6 ; i++)
	{
		pca.getIthLargestEigenValueEigenVector(i, eigenValue, eigenVector);
		std::cout << "eigenValue[" << i << "] = " << eigenValue << "; eigenVector:";
		for (int j=0 ; j<N ; j++)
			std::cout << " " << eigenVector[j];
		std::cout << '\n';
	}

	//get the x, y, z values of each sample from the file 
	float minXValue, maxXValue, minYValue, maxYValue, minZValue, maxZValue;
	float minShape, maxShape, minColor, maxColor;
	for (int i = 0; i < R; i++)
	{
		float xValue(0),yValue(0),zValue(0);
		float shape(0), size(0), color(0);

		for (int v = 0; v < 6; v++)
		{
			pca.getIthLargestEigenValueEigenVector(v, eigenValue, eigenVector);
			if(v == 0)
			{
				for (int j = 0 ; j < N ; j++)
					xValue = vals[i][j]*eigenVector[j]+xValue;
			}
			if(v == 1)
			{
				for (int j = 0 ; j < N ; j++)
					yValue = vals[i][j]*eigenVector[j]+yValue;
			}
			if(v == 2)
			{
				for (int j = 0 ; j < N ; j++)
					zValue = vals[i][j]*eigenVector[j]+zValue;
			}
			if(v == 3)
			{
				for (int j = 0 ; j < N ; j++)
					shape = vals[i][j]*eigenVector[j]+shape;
			}
			if(v == 4)
			{
				for (int j = 0 ; j < N ; j++)
					size = vals[i][j]*eigenVector[j]+size;
			}
			else
			{
				for (int j = 0 ; j < N ; j++)
					color = vals[i][j]*eigenVector[j]+color;
			}
		}
		pts[i] = cryph::AffPoint(xValue, yValue, zValue);
		sps[i] = shape;
		sz[i] = size;
		crs[i] = color;
	}

	maxShape = minShape = sps[0];
	maxColor = minColor = crs[0];	
	for (int i = 0; i<R; i++)
	{
/*		if(pts[i][0] > maxXValue) maxXValue = pts[i][0];
		if(pts[i][0] < minXValue) minXValue = pts[i][0];
		if(pts[i][1] > maxYValue) maxYValue = pts[i][1];
		if(pts[i][1] < minYValue) minYValue = pts[i][1];
		if(pts[i][2] > maxZValue) maxZValue = pts[i][2];
		if(pts[i][2] < minZValue) minZValue = pts[i][2];
*/
		if(sps[i] > maxShape) maxShape = sps[i];
		if(sps[i] < minShape) minShape = sps[i];
		if(crs[i] > maxColor) maxColor = crs[i];
		if(crs[i] < minColor) minColor = crs[i];
		std::cout << sps[i] << "\t" << sz[i] << "\t" << crs[i] << std::endl;
	}
	std::cout << "\n";
	std::cout << "\n";


	// One-time initialization of the glut
	glutInit(&argc, argv);

	Controller c("Scatter Plot", GLUT_DOUBLE|GLUT_DEPTH);

/*	AxesMV* axes = new AxesMV(minXValue, maxXValue, 0.2, 0.5,
				  minYValue, maxYValue, 0.2, 0.5,
				  minZValue, maxZValue, 0.2, 0.5);
*/
	AxesMV* axes = new AxesMV(-2.0, 2.0, 0.2, 0.5,
				  -2.0, 2.0, 0.2, 0.5,
				  -2.0, 2.0, 0.2, 0.5);

	c.addModel(axes);
	
	PointsMV* ptsmv = new PointsMV(pts, sps, sz, crs, R, GL_POINTS);	
	std::cout << "The above data are the values for attributes shape, size and color respectively" << std::endl;
	std::cout << "First colume (shape) | Second colume (size) | Third colue (color)" << std::endl;
	std::cout << "\n";
	std::cout << "Shape(Min):" << minShape << "\t" << "Shape(Max):" << maxShape << std::endl;
	std::cout << "According to Shape(Min) and Shape(Max) value, please input three proper cut-points for shapes(increasing), sperate by space, then press Enter:" << std::endl;
	std::cin >> ptsmv->cutForCross >> ptsmv->cutForCircle >> ptsmv->cutForHourglass;
	std::cout << "\n";
	std::cout << "Please input sizeFactor (prefer 0.1):" << std::endl;
	std::cin >> ptsmv->sizeFactor;
	std::cout << "\n";
	std::cout << "Color(Min):" << minColor << "\t" << "Color(Max):" << maxColor << std::endl;
	std::cout << "According to Color(Min) and Color(Max) value, please input two proper cut-points for colors(increasing), sperate by space, then press Enter:" << std::endl;
	std::cin >> ptsmv->cutForRed >> ptsmv->cutForGreen;
	std::cout << "\n";
	std::cout << "Please input three locations for vec4 pvaSet for attribute shape, size and color (must be 0 - 7):";
	do{
		std::cin >> ptsmv->useForShape >> ptsmv->useForSize >> ptsmv->useForColor;
		if(ptsmv->useForShape > 7 || ptsmv->useForSize > 7 || ptsmv->useForColor > 7)
		{
			std::cout << "Locations for pvaSets can not exceed 7, please input again:";
			continue;
		}
		else break;
	}while(1);

	c.addModel(ptsmv);

	initializeViewingInformation(c);
	glClearColor(1.0, 1.0, 1.0, 1.0);

	std::cout << "\n";
	std::cout << "Program runs successfully. Congratulations!" << std::endl;
	std::cout << "Hit ^C and follow the same steps if you want to change the cutpoints or test another data set." << std::endl;
	// Off to the glut event handling loop:
	glutMainLoop();

	return 0;
}
