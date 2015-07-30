// Controller.h - a basic Controller (in Model-View-Controller sense)

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <iostream>
#include <string>
#include <vector>

class ModelView;

class Controller
{
public:
	Controller(const std::string& name, int glutRCFlags = 0);
	virtual ~Controller();

	// 1. MANAGING THE LIST OF MODELS
	void addModel(ModelView* m);
	int getNumModels() const { return models.size(); }
	ModelView* getModel(int which) const;
	// The following method returns the ModelView removed from the
	// list (e.g., so the caller can issue a "delete" on it)
	ModelView* removeModel(int which);
	void removeModel(ModelView* m);
	// if (do_delete) each model will have "delete" applied to it as
	// it is removed from the list
	void removeAllModels(bool do_delete);
	void toggleVisibility(int which);

	// 2. OTHER METHODS
   	double getViewportAspectRatio() const; // height/width
	void getOverallMCBoundingBox(double* xyzLimits) const;
	virtual void getMCRegionOfInterest(double* xyzLimits) const;
	virtual void printKeyboardKeyList();

	// 3. CLASS METHODS
	static bool checkForErrors(std::ostream& os, const std::string& context);
	static Controller* getCurrentController();
	static void reportVersions(std::ostream& os);

protected:
	Controller(const Controller& c) {} // do not allow copies, including pass-by-value
	std::vector<ModelView*> models;
	std::vector<bool> visible;

	void checkForPick(int x, int y);
	virtual void establishInitialCallbacksForRC();
	virtual void handleDisplay();
	virtual void handleKeyboard(unsigned char key, int x, int y);
	virtual void handleMouseFunc(int button, int state, int x, int y);
	virtual void handleMouseMotion(int x, int y);
	virtual void handleMousePassiveMotion(int x, int y);
	virtual void handleReshape();
	virtual void handleSpecialKey(int key, int x, int y);

	int createWindow(const std::string& windowTitle, int glutRCFlags);
	void screenXYToLDS(int x, int y, double& ldsX, double& ldsY);

    	// Data the Controller uses to track the overall MC box bounding all models.
    	double overallMCBoundingBox[6];
	int glClearFlags;

private:
    	// Width and Height of current viewport (window manager window)
    	int vpWidth, vpHeight;
	bool doubleBuffering;
	double scaleFraction, scaleIncrement;

	// mouse state
	bool mouseMotionIsRotate, mouseMotionIsTranslate;
	int screenBaseX, screenBaseY;

	// for handling keyboard commands with numeric operands
	unsigned char commandChar, lastNonNumericKeyboardChar;
	bool parsingMultiDigitCommandParameter;
	bool parsingSingleDigitCommandParameter;
	int commandParameter;

	void updateMCBoundingBox(ModelView* m);

	static Controller* curController;

	static void displayCB();
	static void keyboardCB(unsigned char key, int x, int y);
	static void mouseFuncCB(int button, int state, int x, int y);
	static void mouseMotionCB(int x, int y);
	static void mousePassiveMotionCB(int x, int y);
	static void reshapeCB(int width, int height);
	static void specialKeyCB(int key, int x, int y);
};

#endif
