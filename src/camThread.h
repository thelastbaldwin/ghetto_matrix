//
//  camThread.h
//  fullscreenCamThreaded
//
//  Created by Minor, Steve on 6/23/14.
//
//
#pragma once

#include "ofApp.h"

class CamThread: public ofThread{
public:
    std::vector<ofVideoGrabber> vidGrabbers;
    std::vector<ofPixels> pixels;

    CamThread(std::vector<int> & videoIds, int width, int height){
        //make sure that we only hold as many pixels as we have cameras
        pixels.resize(videoIds.size());

        for (int i = 0; i < videoIds.size(); ++i){
            ofVideoGrabber grabber;
            grabber.setDeviceID(videoIds[i]);
            //at 30fps, will use significantly less bandwidth
            //at least on the mac, can't set framerate
            grabber.setDesiredFrameRate(30);
            //the false tells the grabber not to use GL textures
            grabber.initGrabber(width, height, false);
            vidGrabbers.push_back(grabber);

        }
    }
private:
    void threadedFunction(){
        while( isThreadRunning()){
            for (auto i = 0; i < vidGrabbers.size(); ++i){
                vidGrabbers[i].update();
                if(vidGrabbers[i].isFrameNew()){
                    lock();
                    pixels[i] = vidGrabbers[i].getPixelsRef();
                    unlock();
                }
            }
        }
    }
};

