//
//  ofxFlexibleVideoPlayer.h
//  emptyExample
//
//  Created by Will Gallia on 04/02/2016.
//
//

#pragma once

#include "ofMain.h"
//#include "ofxMaxim.h"

class ofxFlexibleSilentVideoPlayer {
public:
    ofxFlexibleSilentVideoPlayer();
    
    virtual void load(string framesFolder, float frameRate=25);
	//bool load(string framesFolder) { load(framesFolder, 25); return true; };
	
    virtual void update();
    virtual void draw();
	
	virtual void play() { mIsPlaying = true; }
	virtual void stop() { mIsPlaying = false; }
	
	float getWidth() const { return 0; }
	float getHeight() const {return 0; }
	
	bool isPaused() const { return !mIsPlaying; }
	bool isLoaded() const { return true; }
	bool isPlaying() const { return !mIsPlaying; }
	
    virtual void setPositionTime(float time);
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
	
    float mFrameRate, mFrameTime;
    float mContentLength; // in seconds
    float mPlayhead; // playhead in seconds
    float mSpeed;
	bool mIsPlaying;
	
protected:
    float mLastUpdateTime;
    
    LoopType mLoop;
    float mCuePoint;
	
    ofShader blendShader;
};

class ofxFlexibleVideoPlayer : public ofxFlexibleSilentVideoPlayer {
public:

	void load(string framesFolder, string audioFile, float frameRate=25);
	void audioOut(ofSoundBuffer &buffer);
	
	void update();
	
	void setPositionTime(float time);
	
protected:
//	ofxMaxiSample mSoundtrackSample;
	vector<float> mAudioData;
	unsigned long mAudioPlayhead, mLastAudioPlayhead;
	float mAudioStep;
	
	ofMutex audioMutex;

};