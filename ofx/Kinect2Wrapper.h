#pragma once

#include "ofMain.h"
#include "ofxKinectForWindows2.h"

class Kinectv2 : public ofxKinectForWindows2::Device {

	ofPixels calibPix;
	ofFloatPixels worldMap;
	const float scale = 4.0f;
	bool gotColor, gotDepth;
public:
	Kinectv2() :gotColor(false), gotDepth(false) {

	}

	void update() {
		ofxKinectForWindows2::Device::update();
		gotColor = false;
		gotDepth = false;
	}

	ofPixels& getCalibratedColorPixelsRef() {
		if (this->isFrameNew() || !gotColor) {
			calibPix = this->getColorSource()->getPixels();
			calibPix.setImageType(OF_IMAGE_COLOR);
			gotColor = true;
		}
		return calibPix;
	}

	ofVec3f mapDepthPointToWorldPoint(ofVec2f p) {

		auto &worldMap = this->getDepthSource()->getDepthToWorldMap();
		auto depthColour = worldMap.getColor(p.x, p.y);
		return ofVec3f(depthColour.r, depthColour.g, depthColour.b);
	}

	ofVec3f imageToWorld(ofVec2f p) {
		return mapDepthPointToWorldPoint(p);
	}
};