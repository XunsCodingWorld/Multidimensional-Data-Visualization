// PNGImageReader.h -- subclass of ImageReader for reading png image files
// This software was developed by James R. Miller (jrmiller@ku.edu) and is OPEN SOURCE
//  Documentation on use:
//      http://people.eecs.ku.edu/~miller/Courses/ToolDoc/ImageReader.html
//

#ifndef PNGIMAGEREADER_H
#define PNGIMAGEREADER_H

#include "ImageReader.h"

class PNGImageReader : public ImageReader
{
public:
	PNGImageReader(std::string fileName);

protected:
	PNGImageReader(const PNGImageReader& s); // cannot use the copy constructor

	virtual bool read();
private:
	int numChannelsFromColorType(int cType);
};

#endif
