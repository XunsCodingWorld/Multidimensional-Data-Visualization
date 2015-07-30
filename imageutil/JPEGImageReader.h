// JPEGImageReader.h -- subclass of ImageReader for reading jpeg image files
// This software was developed by James R. Miller (jrmiller@ku.edu) and is OPEN SOURCE
//  Documentation on use:
//      http://people.eecs.ku.edu/~miller/Courses/ToolDoc/ImageReader.html
//

#ifndef JPEGIMAGEREADER_H
#define JPEGIMAGEREADER_H

#include "ImageReader.h"

class JPEGImageReader : public ImageReader
{
public:
	JPEGImageReader(std::string fileName);

protected:
	JPEGImageReader(const JPEGImageReader& s); // cannot use the copy constructor

	virtual bool read();
};

#endif
