// ShaderIF.h: Basic interface to read, compile, and link GLSL Shader programs

#ifndef SHADERIF_H
#define SHADERIF_H

#include <GL/gl.h>

#include <string>

class ShaderIF
{
public:
	struct ShaderSpec
	{
		std::string fName;
		GLenum sType;
	};
	// There are two ways to create and manage shader programs:
	// 1. PREFERRED SCHEME: create an instance with one of the following
	//    two constructors and use the getShaderPgmID() method to get the
	//    shader program ID like:
	//       ShaderIF* sIF = new ShaderIF(vShaderFileName, fShaderFileName);
	//       int pgmId = sIF->getShaderPgmID();
	//    This approach is preferred whenever it is necessary for a shader
	//    program and its associated resources to be deleted during program
	//    execution. In addition to "normal" situations in which this might
	//    occur, note that the Rendering Context is routinely destroyed when
	//    running on android devices (e.g., whenever the device is rotated,
	//    the "home" key pressed, etc.) and when this happens, you must be
	//    prepared to recognize that the shader program must be recreated.
	//    A typical client approach is to create one shader program for a
	//    class of objects, using a reference counting scheme to know when the
	//    shader program and its resources need to be deleted.
	ShaderIF(const std::string& vShader, const std::string& fShader);
	ShaderIF(const ShaderSpec* shaders, int nShaders);
	virtual ~ShaderIF();
	int getShaderPgmID() const { return shaderPgm; }

	// 2. A simpler scheme: Use one of the following two class methods. This
	//    simpler scheme will be satisfactory as long as you know you will never
	//    need to delete and/or recreate the program and its resources before
	//    your program ends.
	//       int pgmId = ShaderIF::initShader(vShaderFileName, fShaderFileName);
	static int initShader(const std::string& vShader, const std::string& fShader);
	static int initShader(const ShaderSpec* shaders, int nShaders);

	// If it is important to be able to delete shader programs before the end
	// of the program, then the following method must be used. Note that this
	// method is NOT called from the destructor as you might expect in case the
	// client has used one of the initShader class methods. In that case, we need
	// the compiled and linked shader programs to live after those methods have
	// exited, deleting the local ShaderIF object created internally.
	void destroy();
private:
	struct Shader
	{
		std::string fName;
		GLenum sType;
		std::string source;
		int pgmID;

		Shader(): fName(""), sType(0), source(""), pgmID(0) {}
	};
	void initShaders();

	static bool readShaderSource(Shader& shader);

	int shaderPgm;
	int numShaderComponents;
	Shader* shaders;
};

#endif
