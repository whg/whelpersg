#include "ofxMidiMapper.h"


bool ofxMidiMapper::learning = false;
ofxBaseGui* ofxMidiMapper::selectedGui = nullptr;


shared_ptr<ofxMidiMapper> midiMapper = nullptr;
const int DEVICE_POLLING_INTERVAL = 5; // seconds

ofxMidiMapper& ofxMidiMapper::get() {
    if (midiMapper == nullptr) {
        midiMapper = shared_ptr<ofxMidiMapper>(new ofxMidiMapper);
    }
    return *midiMapper.get();
}