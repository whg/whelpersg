#pragma once

#include "ofxGui.h"

#include "ofxOscCenter.h"

class ofxParameterMapper {
public:
    
    static shared_ptr<ofxParameterMapper> get();
    
    void guiSelected(ofxGuiSelectedArgs &args);
    
    void updateOscSourceMapping(bool& b);
        
    void newOscMessage(ofxOscCenterNewMessageArgs &args);
    
    struct BaseMapper {
        
        BaseMapper(ofxBaseGui &guiElem, string path) : guiElem(guiElem), path(path), panel(nullptr) {}
        
        ofxBaseGui &guiElem;
        string path;
        shared_ptr<ofxPanel> panel;
        string titlePrefix;
        
        virtual shared_ptr<ofxPanel> getPanel();
        
    };

    struct Sources : public BaseMapper {
        
        Sources(ofxBaseGui &guiElem, string path);

        void addParameter(const ofxOscCenter::Command &command);

        ofParameter<int> midiChannel;
        ofxGuiGroup mOscGroup;
        
        vector<ofParameter<bool>> mOscSources;
        map<string, ofxGuiGroup> mTracks;
        
        void updateOscList(ofxOscCenterCommandArgs &args);
        
    };
    
    struct Limits : public BaseMapper {
        ofParameter<float> inputMin, inputMax, outputMin, outputMax;
        
        Limits(ofxBaseGui &guiElem, string path);
    };
    
private:
    ofxParameterMapper() {}
    void setup();
    
    map<string, unique_ptr<BaseMapper>> mSourceMap;
    map<string, vector<ofAbstractParameter*>> mParamMap;
};
