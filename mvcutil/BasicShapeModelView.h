// BasicShapeModelView.h

#ifndef BASICSHAPEMODELVIEW_H
#define BASICSHAPEMODELVIEW_H

#include <GL/gl.h>
#include <GL/freeglut.h>

#include <string>

#include "ModelViewWithLighting.h"
#include "BasicShape.h"
#include "ImageReader.h"

class BasicShapeModelView : public ModelViewWithLighting
{
public:
	BasicShapeModelView(BasicShape* shapeIn,
		float R = 1.0, float G = 193.0/255.0, float B = 37.0/255.0, float A = 1.0,
		float fa = 0.6, float fd = 0.6, float fs = 0.9, float shininess=50.0);
	virtual ~BasicShapeModelView();

	void getMCBoundingBox(double* xyzLimits) const;
	void render();
	void setColorGenerationMode(int m) { colorGenerationMode = m; }
	void setTextureImage(const char* imgFileName);
	void setTextureSource(int s) { textureSource = s; }

private:
	void defineGeometry();
	void defineTexture();
	void drawShape();

	BasicShape* theShape;
	GLuint vao, vertexBuffer, normalBuffer, textureCoordBuffer;
	double xyzMinMax[6];
	ImageReader* texImage;
	GLuint texID;
};

#endif
