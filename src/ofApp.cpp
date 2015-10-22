#include "ofApp.h"

//TODO: load these from XML settings
const int ofApp::SEND_PORT = 12346;
const int ofApp::RECEIVE_PORT = 12345;
const int ofApp::NUM_MATRIX_FRAMES = 10;
const std::string ofApp::HOST = "localhost";
const std::string ofApp::OUTPUT_PATH = "output/";


//--------------------------------------------------------------
void ofApp::setup(){
    ofSetWindowTitle("Ghetto Matrix");

    sender.setup(HOST, SEND_PORT);
    receiver.setup(RECEIVE_PORT);

    settings.load("settings.xml");
    fbo.allocate(ofGetWidth(), ofGetHeight());

    std::vector<int> cam1Ids = getCameraIds(settings.getValue("camthread1"));
    hCamThread1 = new CamThread(cam1Ids, 320, 240);
    hCamThread1->startThread();

    std::vector<int> cam2Ids = getCameraIds(settings.getValue("camthread2"));
    hCamThread2 = new CamThread(cam2Ids, 320, 240);
    hCamThread2->startThread();

    std::vector<int> mainCameraIds = getCameraIds(settings.getValue("maincam"));
    hMainCameraThread = new CamThread(mainCameraIds, 1024, 768);
    hMainCameraThread->startThread();

    gifEncoder.setup(320, 240, 0.1);
    gifEncoder.start();

    ofAddListener(ofxGifEncoder::OFX_GIF_SAVE_FINISHED, this, &ofApp::onGifSaved);
}

//--------------------------------------------------------------
void ofApp::update(){
    while(receiver.hasWaitingMessages()){
        // get the next message
		ofxOscMessage m;
		receiver.getNextMessage(&m);

        if(m.getAddress() == "/take/picture"){
            std::string type = m.getArgAsString(0);
            std::string id = m.getArgAsString(1);


            if(type == "matrix"){
                cout << "matrix requested" << endl;
                takeMatrixPhoto("matrix-" + ofGetTimestampString("%m%d%Y-%H%M%s") + "_" + id + ".gif", id);
            }else if(type == "traditional"){
                //take a picture every time we get this message
                //while we have less than 4 images, keep waiting
                //once we have all the images, composite it and send it
                //using sendMessage
                cout << "traditional requested" << endl;
                std::string fileName = takeTraditionalPhoto("traditional-" + ofGetTimestampString("%m%d%Y-%H%M%s") + ".jpg", id);
                sendPhoto(fileName, id);
            }

		}else{
            std::cout << m.getAddress();
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){

}

 std::vector<int> ofApp::getCameraIds(std::string idString){
    std::vector<string> cam1Addresses = ofSplitString(idString, " ");
    std::vector<int> camIds;
    std::stringstream ss;
    int fromString;

    std::for_each(cam1Addresses.begin(), cam1Addresses.end(), [&ss, &fromString, &camIds](string s){
        ss << s;
        ss >> fromString;
        camIds.push_back(fromString);
        ss.str("");
        ss.clear();
    });

    return camIds;
 }


std::string ofApp::takeTraditionalPhoto(const string& fileName, const string& id){
    //this method takes 4 pictures, each after ~3 second day
    //and composes them into a single 4-up image
    ofFbo fbo;
    fbo.allocate(1024 * 2, 768 * 2, GL_RGB);
    ofImage currentFrame;
    currentFrame.allocate(1024, 768, OF_IMAGE_COLOR);

    fbo.begin();
    //repeat 4 times
    for(int i = 0; i < 4; ++i){
        //wait 3 seconds
        sendCountDown(3, id);
        //should send the countdown

        //takes 1 picture
        hMainCameraThread->lock();
        currentFrame.getPixelsRef() = hMainCameraThread->pixels[0];
        hMainCameraThread->unlock();

        currentFrame.reloadTexture();
        //draw to fbo in correct quadrant
        switch(i){
            case 0:
                currentFrame.draw(0, 0);
                break;
            case 1:
                currentFrame.draw(1024, 0);
                break;
            case 2:
                currentFrame.draw(0, 768);
                break;
            case 3:
                currentFrame.draw(1024, 768);
                break;
        }
    }
    fbo.end();

    //save the image in the fbo
    ofImage finalImage;
    finalImage.allocate(1024 * 2, 768 * 2, OF_IMAGE_COLOR);
    fbo.readToPixels(finalImage.getPixelsRef());
    finalImage.reloadTexture();
    finalImage.saveImage(OUTPUT_PATH + fileName);

    return fileName;
}

std::string ofApp::takeMatrixPhoto(const string& fileName, const string&){
    gifEncoder.reset();
    ofImage tmpImage;
    tmpImage.allocate(320, 240, OF_IMAGE_COLOR);

    //collect the camera images and insert them into cameraPixels
    hCamThread1->lock();
    hCamThread2->lock();


    //forward
    std::for_each(hCamThread1->pixels.begin(), hCamThread1->pixels.end(), [&](const ofPixels& pixels){
        tmpImage.getPixelsRef() = pixels;
        tmpImage.reloadTexture();
        gifEncoder.addFrame(tmpImage);
    });


    std::for_each(hCamThread2->pixels.begin(), hCamThread2->pixels.end(), [&](const ofPixels& pixels){
        tmpImage.getPixelsRef() = pixels;
        tmpImage.reloadTexture();
        gifEncoder.addFrame(tmpImage);
    });

    //backward
    std::for_each(hCamThread2->pixels.rbegin(), hCamThread2->pixels.rend(), [&](const ofPixels& pixels){
        tmpImage.getPixelsRef() = pixels;
        tmpImage.reloadTexture();
        gifEncoder.addFrame(tmpImage);
    });

    std::for_each(hCamThread1->pixels.rbegin(), hCamThread1->pixels.rend(), [&](const ofPixels& pixels){
        tmpImage.getPixelsRef() = pixels;
        tmpImage.reloadTexture();
        gifEncoder.addFrame(tmpImage);
    });

    hCamThread1->unlock();
    hCamThread2->unlock();

    gifEncoder.save(OUTPUT_PATH + fileName);

    return fileName;
}

void ofApp::onGifSaved(string &fileName) {
    //extract id out of filename
    int underscorePos = fileName.find("_");
    int dotPos = fileName.find(".gif");
    int slashPos = fileName.find("/");
    std::string parsedFilename = fileName.substr(slashPos + 1, fileName.size() - 1);
    std::string id = fileName.substr(underscorePos + 1, dotPos - underscorePos - 1);
    sendPhoto(parsedFilename, id);
    cout << "gif sent to" << id << endl;
    cout << "gif saved as " << fileName << endl;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    //just for testing, but might be useful for MANUAL OVERRIDE
    cout << key << endl;
    if(key == 't'){
        cout << "space pressed" << endl;
        takeTraditionalPhoto("traditional - " + ofGetTimestampString("%m%d%Y-%H%M%s") + ".jpg", NULL);
    }
    if(key == 'm'){
        cout << "m pressed" << endl;
        takeMatrixPhoto("matrix - " + ofGetTimestampString("%m%d%Y-%H%M%s") + ".gif", NULL);
    }
}


//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

void ofApp::sendPhoto(const string& filename, const string& id){
    cout << "trying to send " << filename << " to " << id << endl;
    /*ofxOscMessage m;
	m.setAddress("/transmit/photo");
	m.addStringArg(filename);
    m.addStringArg(id);
	sender.sendMessage(m);*/
	sendMessage(filename, "/transmit/photo", id);
}

void ofApp::sendCountDown(const int& seconds, const string& id){
    for(int i = seconds; i > 0; --i){
        sendMessage(ofToString(i), "/photo/countdown", id);
        ofSleepMillis(1000);
    }
}

void ofApp::sendMessage(const string& message, const string& address, const string& id){
    cout << "message: " << message << ", address: " << address << ", id: " << id << endl;
    ofxOscMessage m;
    m.setAddress(address);
    m.addStringArg(message);
    m.addStringArg(id);
    sender.sendMessage(m);
}


void ofApp::exit(){
    hMainCameraThread->stopThread();
    delete hMainCameraThread;

    hCamThread1->stopThread();
    delete hCamThread1;

    hCamThread2->stopThread();
    delete hCamThread2;

    gifEncoder.exit();
}
