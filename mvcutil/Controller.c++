// Controller.c++: a basic Controller (in Model-View-Controller sense)

#include <GL/gl.h>
#include <GL/freeglut.h>

#include "Controller.h"
#include "ModelView.h"
#include "ProjectionType.h"

Controller* Controller::curController = NULL;

static const char NO_CHAR = '\0';
static const char MULTIDIGIT_NUMERIC_COMMAND_PARAMETER_START = '#';
static const char MULTIDIGIT_NUMERIC_COMMAND_PARAMETER_END   = '$';
static const char SINGLEDIGIT_NUMERIC_COMMAND_PARAMETER_FLAG   = '@';

Controller::Controller(const std::string& name, int glutRCFlags) :
	scaleFraction(1.1), scaleIncrement(0.1),
	mouseMotionIsRotate(false), mouseMotionIsTranslate(false),
	// Viewport
	vpWidth(-1), vpHeight(1), doubleBuffering(false), glClearFlags(GL_COLOR_BUFFER_BIT),
	commandChar(NO_CHAR), lastNonNumericKeyboardChar(NO_CHAR),
	parsingMultiDigitCommandParameter(false), parsingSingleDigitCommandParameter(false),
	commandParameter(0)
{
	curController = this;
	
    // indicate we do not yet have any models by setting min to +1 and max to -1:
    overallMCBoundingBox[0] = overallMCBoundingBox[2] = overallMCBoundingBox[4] = 1.0;
    overallMCBoundingBox[1] = overallMCBoundingBox[3] = overallMCBoundingBox[5] = -1.0;

	// First create the window and its Rendering Context (RC)
	int windowID = createWindow(name, glutRCFlags);

	// Then initialize the newly created OpenGL RC
	establishInitialCallbacksForRC(); // the callbacks for this RC
}

Controller::~Controller()
{
	if (this == curController)
		curController = NULL;
}

void Controller::addModel(ModelView* m)
{
	if (m == NULL)
		return;
	models.push_back(m);
	visible.push_back(true);
	updateMCBoundingBox(m);
}

bool Controller::checkForErrors(std::ostream& os, const std::string& context)
	// CLASS METHOD
{
	bool hadError = false;
	GLenum e = glGetError();
	while (e != GL_NO_ERROR)
	{
		os << "CheckForErrors (context: " <<  context
		   << "): " << (char*)gluErrorString(e) << std::endl;
		e = glGetError();
		hadError = true;
	}
	return hadError;
}

void Controller::checkForPick(int x, int y)
{
	// convert pixel coordinates to LDS
	double ldsX = 2.0 * static_cast<double>(x) / static_cast<double>(vpWidth) - 1.0;
	double ldsY = 2.0 * static_cast<double>(vpHeight - y) / static_cast<double>(vpHeight) - 1.0;

	int which = 0;
	for (std::vector<ModelView*>::iterator it=models.begin() ; it<models.end() ; it++)
	{
		if (visible[which++])
			if ((*it)->picked(ldsX, ldsY))
				return;
	}
}

int Controller::createWindow(const std::string& windowTitle, int glutRCFlags) // CLASS METHOD
{
	// The following calls enforce use of only non-deprecated functionality.
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	// Now create the window and its Rendering Context.
	if (glutRCFlags != 0)
	{
		glutInitDisplayMode(glutRCFlags);
		doubleBuffering = (glutRCFlags & GLUT_DOUBLE) != 0;
		if ((glutRCFlags & GLUT_DEPTH) != 0)
			glClearFlags |= GL_DEPTH_BUFFER_BIT;
	}
	int windowID = glutCreateWindow(windowTitle.c_str());
	if ((glutRCFlags & GLUT_DEPTH) != 0)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	return windowID;
}

void Controller::displayCB() // CLASS METHOD
{
	if (curController != NULL)
		curController->handleDisplay();
}

void Controller::establishInitialCallbacksForRC()
{
	glutDisplayFunc(displayCB);
	glutReshapeFunc(reshapeCB);
	glutKeyboardFunc(keyboardCB);
	glutMouseFunc(mouseFuncCB);
	glutMotionFunc(mouseMotionCB);
	glutPassiveMotionFunc(mousePassiveMotionCB);
	glutSpecialFunc(specialKeyCB);
}

Controller* Controller::getCurrentController()
{
	return curController;
}

ModelView* Controller::getModel(int which) const
{
	if ((which >= 0) && (which < models.size()))
		return models[which];
	return NULL;
}

void Controller::getOverallMCBoundingBox(double* xyzLimits) const
{
    for (int i=0 ; i<6 ; i++)
        xyzLimits[i] = overallMCBoundingBox[i];
}

double Controller::getViewportAspectRatio() const
{
    return static_cast<double>(vpHeight) / static_cast<double>(vpWidth);
}

// Depending on user interactions and requests, the current "region of interest"
// may be a subset (or possibly a superset) of the overall bounding box.For now,
// we will just assume the current "region of interest" is just the whole model.
void Controller::getMCRegionOfInterest(double* xyzLimits) const
{
    for (int i=0 ; i<6 ; i++)
        xyzLimits[i] = overallMCBoundingBox[i];
}

void Controller::handleDisplay()
{
	// clear the frame buffer
	glClear(glClearFlags);

	// draw the collection of models
	int which = 0;
	for (std::vector<ModelView*>::iterator it=models.begin() ; it<models.end() ; it++)
		if (visible[which++])
			(*it)->render();

	if (doubleBuffering)
		glutSwapBuffers(); // does an implicit glFlush()
	else
		glFlush(); // force execution of queued OpenGL commands

	checkForErrors(std::cout, "Controller::handleDisplay");
}

void Controller::handleKeyboard(unsigned char key, int x, int y)
{
	const unsigned char ESC = 27;
	if (key == ESC)
		exit(0);

	if (key == '?')
	{
		printKeyboardKeyList();
		return;
	}

	bool passToModelView = false, haveCommandParameter = false;
	if (key == '+')
		ModelView::addToGlobalZoom(scaleIncrement);
	else if (key == '-')
		ModelView::addToGlobalZoom(-scaleIncrement);
	else if (key == 'O')
		ModelView::setProjection(ORTHOGONAL);
	else if (key == 'P')
		ModelView::setProjection(PERSPECTIVE);
	else if (key == 'Q')
		ModelView::setProjection(OBLIQUE);
	else if ((key == MULTIDIGIT_NUMERIC_COMMAND_PARAMETER_END) && parsingMultiDigitCommandParameter)
	{
		if (commandChar == 'D')
		{
			ModelView* m = removeModel(commandParameter);
			delete m;
		}
		else if (commandChar == 'V')
			toggleVisibility(commandParameter);
		else
			passToModelView = haveCommandParameter = true;
		parsingMultiDigitCommandParameter = false;
	}
	else if (key == MULTIDIGIT_NUMERIC_COMMAND_PARAMETER_START)
	{
		commandChar = lastNonNumericKeyboardChar;
		commandParameter = 0;
		parsingMultiDigitCommandParameter = true;
		// in case this isn't intended to be part of a numeric command sequence:
		passToModelView = true;
	}
	else if (key == SINGLEDIGIT_NUMERIC_COMMAND_PARAMETER_FLAG)
	{
		commandChar = lastNonNumericKeyboardChar;
		parsingSingleDigitCommandParameter = true;
		// in case this isn't intended to be part of a numeric command sequence:
		passToModelView = true;
	}
	else
		passToModelView = true;

	bool numericKey = (key >= '0') && (key <= '9');
	if (numericKey)
	{
		if (parsingMultiDigitCommandParameter)
			commandParameter = (10 * commandParameter) +
				(static_cast<int>(key) - static_cast<int>('0'));
		else if (parsingSingleDigitCommandParameter)
		{
			commandParameter = static_cast<int>(key) - static_cast<int>('0');
			if (commandChar == 'D')
			{
				ModelView* m = removeModel(commandParameter);
				delete m;
			}
			else if (commandChar == 'V')
				toggleVisibility(commandParameter);
			else
				passToModelView = haveCommandParameter = true;
			parsingSingleDigitCommandParameter = false;
		}
	}
	else if ((key != MULTIDIGIT_NUMERIC_COMMAND_PARAMETER_START) && (key != SINGLEDIGIT_NUMERIC_COMMAND_PARAMETER_FLAG))
		parsingMultiDigitCommandParameter = parsingSingleDigitCommandParameter = false;

	if (passToModelView)
	{
		double ldsX, ldsY; // only coord system known to both Controller and ModelView
		screenXYToLDS(x, y, ldsX, ldsY);

		for (std::vector<ModelView*>::iterator it=models.begin() ; it<models.end() ; it++)
		{
			// we always pass each individual key stroke
			(*it)->handleCommand(key, ldsX, ldsY);
			if (haveCommandParameter)
			{
				(*it)->handleCommand(commandChar, commandParameter, ldsX, ldsY);
			}
		}		
	}

	if (key == MULTIDIGIT_NUMERIC_COMMAND_PARAMETER_END)
		commandChar = lastNonNumericKeyboardChar = NO_CHAR;
	else if ((key != MULTIDIGIT_NUMERIC_COMMAND_PARAMETER_START) &&
	         (key != SINGLEDIGIT_NUMERIC_COMMAND_PARAMETER_FLAG) && !numericKey)
		lastNonNumericKeyboardChar = key;

	glutPostRedisplay();
}

void Controller::handleMouseFunc(int button, int state, int x, int y)
{
	const int SCROLL_WHEEL_UP = 3;
	const int SCROLL_WHEEL_DOWN = 4;
	if (button == SCROLL_WHEEL_UP)
	{
		// each wheel click generates a state==DOWN and state==UP event; use only one
		if (state == GLUT_DOWN)
		{
			ModelView::scaleGlobalZoom(scaleFraction);
			glutPostRedisplay();
		}
	}
	else if (button == SCROLL_WHEEL_DOWN)
	{
		// each wheel click generates a state==DOWN and state==UP event; use only one
		if (state == GLUT_DOWN)
		{
			ModelView::scaleGlobalZoom(1.0/scaleFraction);
			glutPostRedisplay();
		}
	}
	else if (button == GLUT_LEFT_BUTTON)
	{
		mouseMotionIsTranslate = ((glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0);
		mouseMotionIsRotate = !mouseMotionIsTranslate;
		if (state == GLUT_DOWN)
		{
			screenBaseX = x; screenBaseY = y;
		}
		else
			mouseMotionIsTranslate = mouseMotionIsRotate = false;
	}
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP))
		checkForPick(x,y);
}

void Controller::handleMouseMotion(int x, int y)
{
	// compute (dx,dy), the offset in pixel space from last event
	int dx = (x - screenBaseX);
	int dy = (y - screenBaseY);
	// update so that next call will get the offset from the location of this event
	screenBaseX = x;
	screenBaseY = y;
	if (mouseMotionIsTranslate)
	{
		// convert (dx, dy) into LDS (-1..+1) range
		double ldsPerPixelX = 2.0/vpWidth;
		double ldsPerPixelY = 2.0/vpHeight;
		ModelView::addToGlobalPan(dx*ldsPerPixelX, -dy*ldsPerPixelY, 0.0);
	}
	else if (mouseMotionIsRotate)
	{
		// convert (dx,dy) into rotation angles about y and x axis, respectively
		const double pixelsToDegrees = 360.0 / 500.0;
		double screenRotY = pixelsToDegrees * dx;
		double screenRotX = pixelsToDegrees * dy;
		ModelView::addToGlobalRotationDegrees(screenRotX, screenRotY, 0.0);
	}
	glutPostRedisplay();
}

void Controller::handleMousePassiveMotion(int x, int y)
{
}

void Controller::handleReshape()
{
	glViewport(0, 0, vpWidth, vpHeight);
}

void Controller::handleSpecialKey(int key, int x, int y)
{
	const double degreesPerKey = 2.0;
	bool shiftDown = ((glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0);
	if (shiftDown)
	{
		double ldsX, ldsY; // only coord system known to both Controller and ModelView
		screenXYToLDS(x, y, ldsX, ldsY);
		for (std::vector<ModelView*>::iterator it=models.begin() ; it<models.end() ; it++)
			(*it)->handleSpecialKey(key, ldsX, ldsY);
	}
	else
	{
		switch (key)
		{
			case GLUT_KEY_HOME:
#ifdef GLUT_KEY_ALT_R
			case GLUT_KEY_ALT_R:
#endif
				ModelView::resetGlobalDynamic();
				ModelView::resetGlobalZoom();
				break;
			case GLUT_KEY_LEFT:
				ModelView::addToGlobalRotationDegrees(0.0, degreesPerKey, 0.0);
				break;
			case GLUT_KEY_RIGHT:
				ModelView::addToGlobalRotationDegrees(0.0, -degreesPerKey, 0.0);
				break;
			case GLUT_KEY_UP:
				ModelView::addToGlobalRotationDegrees(-degreesPerKey, 0.0, 0.0);
				break;
			case GLUT_KEY_DOWN:
				ModelView::addToGlobalRotationDegrees(degreesPerKey, 0.0, 0.0);
				break;
			case 112: //GLUT_KEY_SHIFT_LEFT:
			case 113: //GLUT_KEY_SHIFT_RIGHT:
				break; // ignore
			default:
				std::cout << "special key: " << key << '\n';
		}
	}
	glutPostRedisplay();
}

void Controller::keyboardCB(unsigned char key, int x, int y)
{
	if (Controller::curController != NULL)
		Controller::curController->handleKeyboard(key, x, y);
}

void Controller::mouseFuncCB(int button, int state, int x, int y)
{
	if (Controller::curController != NULL)
		Controller::curController->handleMouseFunc(button, state, x, y);
}

void Controller::mouseMotionCB(int x, int y)
{
	if (Controller::curController != NULL)
		Controller::curController->handleMouseMotion(x, y);
}

void Controller::printKeyboardKeyList()
{
	std::cout << "Controller:\n";
	std::cout << "? - produce this list\n";
	std::cout << "\tO, P, Q: set Orthogonal, Perspective, obliQue\n";
	std::cout << "\tDelete (D) and Visibility (V): D" << SINGLEDIGIT_NUMERIC_COMMAND_PARAMETER_FLAG
	          << "i -OR- D" << MULTIDIGIT_NUMERIC_COMMAND_PARAMETER_START << "iii"
	          << MULTIDIGIT_NUMERIC_COMMAND_PARAMETER_END << '\n';
	bool firstCall = true;
	for (std::vector<ModelView*>::iterator it=models.begin() ; it<models.end() ; it++)
	{
		(*it)->printKeyboardKeyList(firstCall);
		firstCall = false;
	}
}

void Controller::mousePassiveMotionCB(int x, int y)
{
	if (Controller::curController != NULL)
		Controller::curController->handleMousePassiveMotion(x, y);
}

void Controller::removeAllModels(bool do_delete)
{
	std::vector<ModelView*>::iterator it = models.begin();
	while (it < models.end())
	{
		if (do_delete)
			delete *it;
		models.erase(it);
		it = models.begin();
	}
	visible.clear();
}

ModelView* Controller::removeModel(int which)
{
	if ((which >= 0) && (which < models.size()))
	{
		ModelView* m = models[which];
		models.erase(models.begin()+which);
		visible.erase(visible.begin()+which);
		return m;
	}
	return NULL;
}

void Controller::removeModel(ModelView* m)
{
	int which = 0;
	for (std::vector<ModelView*>::iterator it=models.begin() ; it<models.end() ; it++)
	{
		if (*it == m)
		{
			models.erase(it);
			visible.erase(visible.begin() + which);
			break;
		}
		which++;
	}
}

void Controller::reportVersions(std::ostream& os)
{
	const char* glVer = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	const char* glslVer = reinterpret_cast<const char*>
			(glGetString(GL_SHADING_LANGUAGE_VERSION));
	// glGetString can return NULL if no rendering context has been created
	os << "VERSIONS: GL: ";
	if (glVer == NULL)
		os << "NULL (has RC been created?)\n";
	else
		os << glVer << '\n';
	os << "        GLSL: ";
	if (glslVer == NULL)
		os << "NULL (has RC been created?)\n";
	else
		os << glslVer << '\n';
	if ((glVer == NULL) || (glslVer == NULL))
		return;
	int glutVer = glutGet(GLUT_VERSION);
	int glutVerMajor = glutVer/10000;
	int glutVerMinor = (glutVer % 10000) / 100;
	int glutVerPatch = glutVer % 100;
	os << "        GLUT: " << glutVerMajor << '.';
	if (glutVerMinor < 10)
		os << '0';
	os << glutVerMinor << '.';
	if (glutVerPatch < 10)
		os << '0';
	os << glutVerPatch << " (" << glutVer << ")\n";
#ifdef __GLEW_H__
	os << "        GLEW: " << glewGetString(GLEW_VERSION) << '\n';
#endif
}

void Controller::reshapeCB(int width, int height)
{
	Controller::curController->vpWidth = width;
	Controller::curController->vpHeight = height;
	Controller::curController->handleReshape();
}

void Controller::screenXYToLDS(int x, int y, double& ldsX, double& ldsY)
{
	ldsX = 2.0 * static_cast<double>(x) / static_cast<double>(vpWidth) - 1.0;
	// The glut reports pixel coordinates assuming y=0 is at the top of the
	// window. The main OpenGL API assumes y=0 is at the bottom, so note we
	// account for that here.
	ldsY = 2.0 * static_cast<double>(vpHeight - y) / static_cast<double>(vpHeight) - 1.0;
}

void Controller::specialKeyCB(int key, int x, int y)
{
	if (Controller::curController != NULL)
		Controller::curController->handleSpecialKey(key, x, y);
}

void Controller::toggleVisibility(int which)
{
	if ((which >= 0) && (which < models.size()))
	{
		visible[which] = !visible[which];
		glutPostRedisplay();
	}
}

void Controller::updateMCBoundingBox(ModelView* m)
{
	if (m == NULL)
		return;
	if (overallMCBoundingBox[0] <= overallMCBoundingBox[1])
	{
        // bounding box already initialized; just update it:
        double xyz[6];
		m->getMCBoundingBox(xyz);
		for (int i=0 ; i<5 ; i+=2)
		{
			if (xyz[i] < overallMCBoundingBox[i])
				overallMCBoundingBox[i] = xyz[i];
			if (xyz[i+1] > overallMCBoundingBox[i+1])
				overallMCBoundingBox[i+1] = xyz[i+1];
		}
	}
	else // use this model to initialize the bounding box
		m->getMCBoundingBox(overallMCBoundingBox);
}
