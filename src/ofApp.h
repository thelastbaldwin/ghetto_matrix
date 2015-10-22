#pragma once

#include <algorithm>
#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxGifEncoder.h"
#include "camThread.h"
//#include "ofxXmlSettings.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);

		void gotMessage(ofMessage msg);
        void sendPhoto(const string& filename, const string& id);
        void sendCountDown(const int& seconds, const string& id);
        void sendMessage(const string& message, const string& address,const string& id);
        std::string takeTraditionalPhoto(const string& fileName, const string& id);
        std::string takeMatrixPhoto(const string& fileName, const string& id);

        std::vector<int> getCameraIds(std::string idString);

        void exit();

        ofxGifEncoder gifEncoder;
        void onGifSaved(string & fileName);

        static const int SEND_PORT;
        static const int RECEIVE_PORT;
        static const std::string HOST;
        static const std::string OUTPUT_PATH;
        static const int NUM_MATRIX_FRAMES;

		ofFbo fbo;

        ofxOscReceiver receiver;
        ofxOscSender sender;

        CamThread* hMainCameraThread;
        CamThread* hCamThread1;
        CamThread* hCamThread2;

		ofXml settings;
};
