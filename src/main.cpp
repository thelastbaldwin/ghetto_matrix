#include "ofAppNoWindow.h"
#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
    ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);
	ofSetupOpenGL(400,400,OF_WINDOW);			// <-------- setup the GL context

	//ofAppNoWindow window;
	//ofSetupOpenGL(&window, 1024, 768, OF_WINDOW);

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());
}
