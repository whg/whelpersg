#pragma once

#include "ofxGui.h"

#include "ofxOscCenter.h"

class ofxParameterMapper {
public:
    
    static shared_ptr<ofxParameterMapper> get();
    
    void guiSelected(ofxGuiSelectedArgs &args);
    
    void updateOscSourceMapping(bool& b);
    
    void newOscMessage(ofxOscCenterNewMessageArgs &args);
        
    template<typename T>
    void modifyParams(const vector<ofxBaseGui*> &baseGuis, T v, float min, float max, string &commandString);
    
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

        void addParameter(const ofxOscCenter::Command &command, bool value=false);

        ofParameter<int> midiChannel;
        ofxGuiGroup mOscGroup;
        
        vector<ofParameter<bool>> mOscSources;
        map<string, ofxGuiGroup> mTracks;
        
        void updateOscList(ofxOscCenterCommandArgs &args);
        
    };
    
    struct OutputLimits : public BaseMapper {
        ofParameter<float> outputMin, outputMax;
        OutputLimits(ofxBaseGui &guiElem, string path);
    };

    
    struct InputLimits : public BaseMapper {
        ofParameter<float> inputMin, inputMax;
        InputLimits(ofxBaseGui &guiElem, string path, string commandString);
    };
    
    
    void save();
    void load();
    
    void setOutputLimitMin(float &v);
    void setOutputLimitMax(float &v);
    
    static ofMutex addSourceMutex;
    
private:
    ofxParameterMapper() {}
    void setup();
    
    map<string, unique_ptr<Sources>> mSourceMap; // (path, sources)
    map<pair<string, ofxBaseGui*>, unique_ptr<InputLimits>> mLimitsMap;
    map<string, vector<ofxBaseGui*>> mParamMap; // (oscCommand, list of params)
    
    map<string, unique_ptr<OutputLimits>> mParameterLimits;
    
};


template<typename T>
void ofxParameterMapper::modifyParams(const vector<ofxBaseGui*> &baseGuis, T v, float min, float max, string &commandString) {

    for (auto *bg : baseGuis) {
    
        float outputMin, outputMax;
        auto key = make_pair(commandString, bg);
        if (mLimitsMap.count(key) > 0) {
            min = mLimitsMap[key]->inputMin;
            max = mLimitsMap[key]->inputMax;
        }
    
        auto *param = &bg->getParameter();
        auto type = param->type();
        
        if (type == typeid(ofParameter<int>).name()) {
            auto &intParam = param->cast<int>();
            intParam.set(ofMap(v, min, max, intParam.getMin(), intParam.getMax(), true));
        }
        else if (type == typeid(ofParameter<float>).name()) {
            auto &floatParam = param->cast<float>();
            floatParam.set(ofMap(v, min, max, floatParam.getMin(), floatParam.getMax(), true));
        }
        else if (type == typeid(ofParameter<unsigned char>).name()) {
            auto &ucharParam = param->cast<unsigned char>();
            ucharParam.set(ofMap(v, min, max, ucharParam.getMin(), ucharParam.getMax(), true));
        }
        
    }
}