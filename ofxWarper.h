// this is a wrapper for whg's fork of ofxGLWarper

#pragma once

#include "ofMain.h"
#include "ofxGLWarper.h"
#include "ofxTools.h"

class ofxWarper {
    
    
public:
    ofxWarper() {}
    
    void setup() {
        
        ofRegisterKeyEvents(this);
        
        calibrated = false;
        editing false;
    
        int bwidth = 400;
        int bx = ofGetWidth() / 2 - bwidth/2;
        int by = ofGetHeight() / 2 - bwidth/2;
        warper.setup(bx, by, bwidth, bwidth);
        warper.activate();
    }
    
    void update() {}
    
    void draw() {
        
        warper.setActive(editing);
        warper.draw();
        
    }
    
    void begin() {
        warper.begin(calibrated);
    }
    
    void end() {
        warper.end();
    }
    
protected:
    // don't call these
    void keyPressed(ofKeyEventArgs &args) {
        
            int key = args.key;
            
            ofApp *app = (ofApp*) ofGetAppPtr();
            ofVec3f p(app->mouseX, app->mouseY);
            KEY('1', warper.setCorner(ofxGLWarper::TOP_LEFT, p))
            KEY('2', warper.setCorner(ofxGLWarper::TOP_RIGHT, p))
            KEY('3', warper.setCorner(ofxGLWarper::BOTTOM_LEFT, p))
            KEY('4', warper.setCorner(ofxGLWarper::BOTTOM_RIGHT, p))
            
            KEY('c', calibrated ^= true)
            KEY('e', editing ^= true)
            
            KEY('s', warper.save())
            KEY('l', warper.load())
        
        }
    }
    void keyReleased(ofKeyEventArgs &args) {}


    bool editing;

    
    ofxGLWarper warper;
    bool calibrated;
    
    void setupWarper() {
        
    }

    

};
