// TGAImageReader.h -- subclass of ImageReader for reading targa (TGA) image files
// This software was developed by James R. Miller (jrmiller@ku.edu) and is OPEN SOURCE
//  Documentation on use:
//      http://people.eecs.ku.edu/~miller/Courses/ToolDoc/ImageReader.html
//

#ifndef TGAIMAGEREADER_H
#define TGAIMAGEREADER_H

#include "ImageReader.h"

class TGAImageReader : public ImageReader
{
public:
	TGAImageReader(std::string fileName);

protected:
	TGAImageReader(const TGAImageReader& s); // cannot use the copy constructor

	virtual bool read();
};

#endif
