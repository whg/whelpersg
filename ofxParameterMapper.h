#pragma once

#include "ofxGui.h"

#include "ofxOscCenter.h"

class ofxParameterMapper {
public:
    
    static shared_ptr<ofxParameterMapper> get();
    
    void guiSelected(ofxGuiSelectedArgs &args);
    
    void updateOscSourceMapping(bool& b);
    void updateLimitMapping(float& v);

    
    void newOscMessage(ofxOscCenterNewMessageArgs &args);
    
    template<typename T>
    void modifyParams(const vector<ofAbstractParameter*> &params, T v, T min, T max);
    
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
        ofAbstractParameter *paramToEffect;
        string inputCommand;
        
        Limits(ofxBaseGui &guiElem, string path);
    };
    
private:
    ofxParameterMapper() {}
    void setup();
    
    map<string, unique_ptr<Sources>> mSourceMap;
    map<ofAbstractParameter*, unique_ptr<Limits>> mLimitsMap;
    map<string, vector<ofAbstractParameter*>> mParamMap;
    
};


template<typename T>
void ofxParameterMapper::modifyParams(const vector<ofAbstractParameter*> &params, T v, T min, T max) {
    for (auto *param : params) {
        
        auto type = param->type();
        
        if (type == "11ofParameterIiE") {
            auto &intParam = param->cast<int>();
            intParam.set(ofMap(v, min, max, intParam.getMin(), intParam.getMax()));
        }
        else if (type == "11ofParameterIfE") {
            auto &floatParam = param->cast<float>();
            floatParam.set(ofMap(v, min, max, floatParam.getMin(), floatParam.getMax()));
        }
        else if (type == "11ofParameterIhE") {
            auto &ucharParam = param->cast<unsigned char>();
            ucharParam.set(ofMap(v, min, max, ucharParam.getMin(), ucharParam.getMax()));
        }
        
    }
}