//
//  ofxFlexibleVideoPlayer.h
//  emptyExample
//
//  Created by Will Gallia on 04/02/2016.
//
//

#pragma once

#include "ofMain.h"
#include "ofxMaxim.h"

class ofxFlexibleVideoPlayer {
public:
    ofxFlexibleVideoPlayer();
    
    void load(string framesFolder, string audioFile, float frameRate=25);
    
    void update();
    void draw();
    
    void audioOut(ofSoundBuffer& buffer);
    
    void setPositionTime(float time);
    void setFrame(int frame);
    void setPosition(float absolutePoint);
    
    float getPositionTime() { return mPlayhead; }
    int getFrame() { return int(mPlayhead / mFrameTime); }
    float getPosition() { return mPlayhead / mContentLength; }
    
    void setSpeed(float speed) { mSpeed = speed; }
    float getSpeed() { return mSpeed; }
    
    void setCuePoint(float time) { mCuePoint = time; }
    void setCueFrame(int frame) { mCuePoint = frame * mFrameTime; }
    
    
    size_t getNumFrames() { return mTextures.size(); }
    int getNumFramesFromTime(float time) { return int(ceil(time / mFrameTime)); }
    
    enum class LoopType { WHOLE, END, NONE };
    LoopType getLoopMode() { return mLoop; }
    void setLoopMode(LoopType t) { mLoop = t; }
    
protected:
    vector<ofTexture> mTextures;
    ofxMaxiSample mSoundtrackSample;
    vector<float> mAudioData;
    unsigned long mAudioPlayhead, mLastAudioPlayhead;
    float mAudioStep;
    
    float mFrameRate, mFrameTime;
    float mContentLength; // in seconds
    float mPlayhead; // playhead in seconds
    float mSpeed;
protected:
    float mLastUpdateTime;
    
    LoopType mLoop;
    float mCuePoint;
    
    ofMutex audioMutex;
    
    ofShader blendShader;
};