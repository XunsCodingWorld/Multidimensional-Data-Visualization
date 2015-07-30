
#ifndef VARIABLE_H
#define VARIABLE_H

#include <string>

class Variable
{
public:
	//Variable();
	//virtual ~Variable();

	int count;	
	std::string name;
	float minValue, maxValue, cardinality;
	float mean;
	float alpha, beta;
	float *value;
};

#endif
