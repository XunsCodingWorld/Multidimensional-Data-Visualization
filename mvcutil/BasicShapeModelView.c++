// BasicShapeModelView.c++

#include "BasicShapeModelView.h"

BasicShapeModelView::BasicShapeModelView(BasicShape* shapeIn,
	float R, float G, float B, float A,
	float fa, float fd, float fs, float shininess) :
		theShape(shapeIn), vertexBuffer(0), normalBuffer(0),
		textureCoordBuffer(0),
		texImage(NULL), texID(0)
{
	if (theShape == NULL)
		return;
	defineGeometry();
	theShape->getMCBoundingBox(xyzMinMax);

	// ambient, diffuse, and specular reflectances along with specular exponent
	matl.set(R, G, B, fa, fd, fs, shininess, A);
}

BasicShapeModelView::~BasicShapeModelView()
{
	if (vertexBuffer > 0)
		glDeleteBuffers(1, &vertexBuffer);
	if (normalBuffer > 0)
		glDeleteBuffers(1, &normalBuffer);
	if (textureCoordBuffer > 0)
		glDeleteBuffers(1, &textureCoordBuffer);
	if (texID > 0)
		glDeleteTextures(1, &texID);
	glDeleteVertexArrays(1, &vao);
	if (texImage != NULL)
		delete texImage;
	if (theShape != NULL)
		delete theShape;
}

void BasicShapeModelView::defineGeometry()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// There is always a vertex buffer:
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	const float* pts = theShape->getPointCoords();
	int nPoints = theShape->getNumPoints();
	glBufferData(GL_ARRAY_BUFFER, nPoints*3*sizeof(float), pts, GL_STATIC_DRAW);
	glEnableVertexAttribArray(pvaLoc_mcPosition);
	glVertexAttribPointer(pvaLoc_mcPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Create the normal buffer, if appropriate
	const float* normals = theShape->getNormals();
	if (normals == NULL)
		glDisableVertexAttribArray(pvaLoc_mcNormal);
	else
	{
		glGenBuffers(1, &normalBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glBufferData(GL_ARRAY_BUFFER, nPoints*3*sizeof(float), normals, GL_STATIC_DRAW);
		glEnableVertexAttribArray(pvaLoc_mcNormal);
		glVertexAttribPointer(pvaLoc_mcNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	// there may or may not be texture coordinates
	const float* texCoords = theShape->getTextureCoords();
	if (texCoords == NULL)
		glDisableVertexAttribArray(pvaLoc_texCoords);
	else
	{
		glGenBuffers(1, &textureCoordBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, textureCoordBuffer);
		glBufferData(GL_ARRAY_BUFFER, nPoints*2*sizeof(float), texCoords, GL_STATIC_DRAW);
		glEnableVertexAttribArray(pvaLoc_texCoords);
		glVertexAttribPointer(pvaLoc_texCoords, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}
}

void BasicShapeModelView::defineTexture()
{
	if (texImage == NULL)
	{
		std::cerr << "The texImage was NULL.\n";
		return;
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	float white[] = { 1.0, 1.0, 1.0, 1.0 };
//	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, white);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); //GL_CLAMP_TO_BORDER);
	GLint level = 0;
	int pw = texImage->getWidth(), ph = texImage->getHeight();
	GLint iFormat = texImage->getInternalFormat();
	GLenum format = texImage->getFormat();
	GLenum type = texImage->getType();
	const GLint border = 0; // must be zero (only present for backwards compatibility)
	const void* pixelData = texImage->getTexture();
	glTexImage2D(GL_TEXTURE_2D, level, iFormat, pw, ph, border, format, type, pixelData);
}

void BasicShapeModelView::drawShape()
{
	// set the shape's material properties:
	glUniform3fv(loc_ka, 1, matl.ka);
	glUniform3fv(loc_kd, 1, matl.kd);
	glUniform3fv(loc_ks, 1, matl.ks);
	glUniform1f (loc_shininess, matl.shininess);
	glUniform1f (loc_alpha, matl.alpha);

	glBindVertexArray(vao);
	
	glUniform1i(ppuLoc_useTextureColorMultiplier, 0);

	// Rendering BasicShape instances may involve 1 or more glDrawArrays calls:
	int nDrawArraysCalls = theShape->getNumDrawArraysCalls();
	for (int i=0 ; i<nDrawArraysCalls ; i++)
	{
		// get data for i-th glDrawArrays call:
		GLenum mode;
		int offset;
		bool canUseTexCoordArray, canUseNormalArray;
		cryph::AffVector fixedN;
		float normal[3];
		int nPoints = theShape->getDrawArraysData(i, mode, offset,
			canUseTexCoordArray, canUseNormalArray, fixedN);
		// process the i-th call
		if (nPoints > 0)
		{
			// "> 0" in the following means some fragment-shader based coloring (e.g., texture mapping)
			if (canUseTexCoordArray && (colorGenerationMode > 0))
			{
				glUniform1i(ppuLoc_colorGenerationMode, colorGenerationMode);
				glUniform1i(ppuLoc_textureSource, textureSource);
				// first see if there is a texture to be applied
				if ((textureSource == 0) && (texImage == NULL))
					glUniform1i(ppuLoc_textureSource, -1); // signal missing texture map
				else if (textureSource == 0)
				{
					glBindTexture(GL_TEXTURE_2D, texID);
					glUniform1i(ppuLoc_textureMap, 0); // refers to '0' in glActiveTexture(GL_TEXTURE0);
				}
			}
			else
				glUniform1i(ppuLoc_colorGenerationMode, JUST_colorToFS);
			if (canUseNormalArray)
				glEnableVertexAttribArray(pvaLoc_mcNormal);
			else
			{
				glDisableVertexAttribArray(pvaLoc_mcNormal);
				glVertexAttrib3fv(pvaLoc_mcNormal, fixedN.vComponents(normal));
			}
			glDrawArrays(mode, offset, nPoints);
		}
	}

	// Rendering BasicShape instances may also involve 1 or more glDrawElements calls.
	// (For example, caps, if present, are drawn with index lists. So are 3 of the faces
	// of blocks.)
	int nIndexLists = theShape->getNumIndexLists();
	for (int i=0 ; i<nIndexLists ; i++)
	{
		GLenum mode;
		GLsizei nInList;
		GLenum type;
		bool canUseTexCoordArray, canUseNormalArray;
		cryph::AffVector fixedN;
		float normal[3];
		const void* indices = theShape->getIndexList(i, mode, nInList, type,
									canUseTexCoordArray, canUseNormalArray, fixedN);
		if (canUseTexCoordArray && (colorGenerationMode > 0))
		{
			glUniform1i(ppuLoc_colorGenerationMode, colorGenerationMode);
			glUniform1i(ppuLoc_textureSource, textureSource);
			// first see if there is a texture to be applied
			if ((textureSource == 0) && (texImage == NULL))
				glUniform1i(ppuLoc_textureSource, -1); // signal missing texture map
			else if (textureSource == 0)
			{
				glBindTexture(GL_TEXTURE_2D, texID);
				glUniform1i(ppuLoc_textureMap, 0); // refers to '0' in glActiveTexture(GL_TEXTURE0);
			}
		}
		else
			glUniform1i(ppuLoc_colorGenerationMode, JUST_colorToFS);
		if (canUseNormalArray)
			glEnableVertexAttribArray(pvaLoc_mcNormal);
		else
		{
			glDisableVertexAttribArray(pvaLoc_mcNormal);
			glVertexAttrib3fv(pvaLoc_mcNormal, fixedN.vComponents(normal));
		}
		glDrawElements(mode, nInList, type, indices);
	}
}

void BasicShapeModelView::getMCBoundingBox(double* xyzLimits) const
{
	// Just pass back what the BasicShape instance gave to us
	for (int i=0 ; i<6 ; i++)
		xyzLimits[i] = xyzMinMax[i];
}

void BasicShapeModelView::render()
{
	if (theShape == NULL)
		return;

	// save the current and set the new shader program
	int savedPgm;
	glGetIntegerv(GL_CURRENT_PROGRAM, &savedPgm);
	glUseProgram(shaderProgram);

	// establish the lighting environment
	defineLightingEnvironment();
	glEnable(GL_DEPTH_TEST);

	// establish global matrices
	cryph::Matrix4x4 mc_ec, ec_lds;
	getMatrices(mc_ec, ec_lds);
	float m[16];
	glUniformMatrix4fv(ppuLoc_mc_ec, 1, false, mc_ec.extractColMajor(m));
	glUniformMatrix4fv(ppuLoc_ec_lds, 1, false, ec_lds.extractColMajor(m));

	drawShape();

	// restore the saved program
	glUseProgram(savedPgm);
}

void BasicShapeModelView::setTextureImage(const char* imgFileName)
{
	if ((theShape == NULL) || (imgFileName == NULL))
	{
		std::cerr << "theShape was NULL.\n";
		return;
	}
	texImage = ImageReader::create(imgFileName);
	defineTexture();
}
