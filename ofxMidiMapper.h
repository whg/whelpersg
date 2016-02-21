
#pragma once

#include "ofxMidi.h"
#include "ofxGui.h"

class ofxMidiMapper : public ofxMidiListener {

public:
    
    static ofxMidiMapper& get();
    
    void setup(ofxMidiIn &midiIn) {
        midiIn.addListener(this);
        ofAddListener(ofxBaseGui::guiSelectedEvent, this, &ofxMidiMapper::guiSelected);
        ofRegisterKeyEvents(this);
    }
    
    void guiSelected(ofxGuiSelectedArgs &args) {
        ofxMidiMapper::selectedGui = args.baseGui;
    }

    void newMidiMessage(ofxMidiMessage& msg) {
        if (ofxMidiMapper::learning && ofxMidiMapper::selectedGui != nullptr) {
            auto *abstractParam = &selectedGui->getParameter();
            if (msg.status == MIDI_CONTROL_CHANGE) {
                mParameterControlMap[msg.control].push_back(abstractParam);
                ofxMidiMapper::selectedGui = nullptr;
            }
            else if (msg.status == MIDI_NOTE_ON) {
                mParameterNoteMap[msg.pitch].push_back(abstractParam);
                
                if (mInvertedNoteIndex.count(abstractParam)) {
                    int note = mInvertedNoteIndex[abstractParam];
                    auto &params = mParameterNoteMap[note];
                    for (auto it = params.begin(); it != params.end(); ++it) {
                        if (*it == abstractParam) {
                            params.erase(it);
                            break;
                        }
                    }
                }
                
                mInvertedNoteIndex[abstractParam] = msg.pitch;
                
                ofxMidiMapper::selectedGui = nullptr;
            }
//            
//                auto &param = selectedGui->getParameter();
//                auto type = param.type();
//                
//                if (type == typeid(ofParameter<int>).name()) {
//                    auto intParam = param.cast<int>();
//                    intParam.set(msg.);
//                }
//                else if (type == typeid(ofParameter<float>).name()) {
//                    auto floatParam = param.cast<float>();
//                    floatParam.set(msg.pitch);
//                }
//                else if (type == typeid(ofParameter<unsigned char>).name()) {
//                    auto ucharParam = param.cast<unsigned char>();
//                    ucharParam.set(ofMap(msg.value, 0, 127, ucharParam.getMin(), ucharParam.getMax()));
//                }
//                ofxMidiMapper::selectedGui = nullptr;
//
//            }
        }
        
        else {
            if (msg.status == MIDI_CONTROL_CHANGE) {
                if (mParameterControlMap.count(msg.control)) {
                
                    for (auto *param : mParameterControlMap[msg.control]) {
                    
                        auto type = param->type();

                        if (type == typeid(ofParameter<int>).name()) {
                            auto intParam = param->cast<int>();
                            intParam.set(ofMap(msg.value, 0, 127, intParam.getMin(), intParam.getMax()));
                        }
                        else if (type == typeid(ofParameter<float>).name()) {
                            auto floatParam = param->cast<float>();
                            floatParam.set(ofMap(msg.value, 0, 127, floatParam.getMin(), floatParam.getMax()));
                        }
                        else if (type == typeid(ofParameter<unsigned char>).name()) {
                            auto ucharParam = param->cast<unsigned char>();
                            ucharParam.set(ofMap(msg.value, 0, 127, ucharParam.getMin(), ucharParam.getMax()));
                        }
                    }
                    
                }
            }
            else if (msg.status == MIDI_NOTE_ON || msg.status == MIDI_NOTE_OFF || msg.status == MIDI_POLY_AFTERTOUCH) {
                if (mParameterNoteMap.count(msg.pitch)) {
                    if (msg.status == MIDI_POLY_AFTERTOUCH) {
                        msg.velocity = msg.value;
                    }
                    
                    for (auto *param : mParameterNoteMap[msg.pitch]) {
                        
                        auto type = param->type();
    
                        if (type == typeid(ofParameter<int>).name()) {
                            auto intParam = param->cast<int>();
                            intParam.set(ofMap(msg.velocity, 0, 127, intParam.getMin(), intParam.getMax()));
                        }
                        else if (type == typeid(ofParameter<float>).name()) {
                            auto floatParam = param->cast<float>();
                            floatParam.set(ofMap(msg.velocity, 0, 127, floatParam.getMin(), floatParam.getMax()));
                        }
                        else if (type == typeid(ofParameter<unsigned char>).name()) {
                            auto ucharParam = param->cast<unsigned char>();
                            ucharParam.set(ofMap(msg.velocity, 0, 127, ucharParam.getMin(), ucharParam.getMax()));
                        }
                    }
                    
                }
            }
        }
    }
    
    void keyPressed(ofKeyEventArgs &args) {
        if (args.key == 'm' || args.key == 'M') {
//            if (ofGetKeyPressed(OF_KEY_COMMAND)) {
                ofxMidiMapper::learning ^= true;
                ofLogNotice() << "Midi learning = " << ofxMidiMapper::learning;
//            }
        }
    }
    void keyReleased(ofKeyEventArgs &args) {}
    
    static bool learning;
    static ofxBaseGui *selectedGui;
    
protected:
    map<int, vector<ofAbstractParameter*>> mParameterControlMap;
    map<int, vector<ofAbstractParameter*>> mParameterNoteMap;
    
    map<ofAbstractParameter*, int> mInvertedNoteIndex;
    map<ofAbstractParameter*, int> mInvertedControlIndex;
};