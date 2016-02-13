#pragma once

#include "ofxGui.h"

#include "ofxOscCenter.h"

class ofxParameterMapper {
public:
    
    static shared_ptr<ofxParameterMapper> get();
    
    void guiSelected(ofxGuiSelectedArgs &args);
    
    void updateOscSourceMapping(bool& b);
    
    void subscribeParameterToOsc(ofAbstractParameter &param, string source);
    
    struct Sources {
        
        Sources(ofxBaseGui &guiElem, string path);

        void addParameter(const ofxOscCenter::Command &command);

        string path;
        shared_ptr<ofxPanel> getPanel();
        
        shared_ptr<ofxPanel> panel;
        ofxBaseGui &guiElem;
        ofParameter<int> midiChannel;
        ofxGuiGroup mOscGroup;
        
        vector<ofParameter<bool>> mOscSources;
        map<string, ofxGuiGroup> mTracks;
        
        void updateOscList(ofxOscCenterCommandArgs &args);
        void something(bool& p) {}
    };
    
private:
    ofxParameterMapper() {}
    void setup();
    
    map<string, unique_ptr<Sources>> sourceMap;
};
