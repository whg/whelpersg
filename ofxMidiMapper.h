
#pragma once

#include "ofxMidi.h"
#include "ofxGui.h"

class ofxMidiMapper : public ofxMidiListener {

public:
    
    void setup(ofxMidiIn &midiIn) {
        midiIn.addListener(this);
        ofAddListener(ofxBaseGui::guiSelectedEvent, this, &ofxMidiMapper::guiSelected);
        ofRegisterKeyEvents(this);
    }
    
    void guiSelected(ofxGuiSelectedArgs &args) {
        ofxMidiMapper::selectedGui = args.baseGui;
    }

    void newMidiMessage(ofxMidiMessage& msg) {
        if (ofxMidiMapper::learning) {
            if (msg.status == MIDI_CONTROL_CHANGE) {
                if (ofxMidiMapper::selectedGui != nullptr) {
                    mParameterMap[msg.control] = &selectedGui->getParameter();
                    ofxMidiMapper::selectedGui = nullptr;
                }
            }
        }
        
        else {
            if (msg.status == MIDI_CONTROL_CHANGE) {
                if (mParameterMap.count(msg.control)) {
                    auto *param = mParameterMap[msg.control];
                    auto type = param->type();

                    if (type == "11ofParameterIiE") {
                        auto intParam = param->cast<int>();
                        intParam.set(ofMap(msg.value, 0, 127, intParam.getMin(), intParam.getMax()));
                    }
                    else if (type == "11ofParameterIfE") {
                        auto floatParam = param->cast<float>();
                        floatParam.set(ofMap(msg.value, 0, 127, floatParam.getMin(), floatParam.getMax()));
                    }
                    
                }
            }
        }
    }
    
    void keyPressed(ofKeyEventArgs &args) {
        if (args.key == 'l' || args.key == 'L') {
            if (ofGetKeyPressed(OF_KEY_COMMAND)) {
                ofxMidiMapper::learning ^= true;
                ofLogNotice() << "Midi learning = " << ofxMidiMapper::learning;
            }
        }
    }
    void keyReleased(ofKeyEventArgs &args) {}
    
    static bool learning;
    static ofxBaseGui *selectedGui;
    
protected:
    map<int, ofAbstractParameter*> mParameterMap;
    
};