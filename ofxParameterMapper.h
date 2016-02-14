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
    
    struct Limits : public BaseMapper {
        ofParameter<float> inputMin, inputMax, outputMin, outputMax;
        Limits(ofxBaseGui &guiElem, string commandString);
    };
    
    void save();
    void load();
    
    static ofMutex addSourceMutex;
    
private:
    ofxParameterMapper() {}
    void setup();
    
    map<string, unique_ptr<Sources>> mSourceMap; // (path, sources)
    map<pair<string, ofxBaseGui*>, unique_ptr<Limits>> mLimitsMap;
    map<string, vector<ofxBaseGui*>> mParamMap; // (oscCommand, list of params)
    
};


template<typename T>
void ofxParameterMapper::modifyParams(const vector<ofxBaseGui*> &baseGuis, T v, float min, float max, string &commandString) {
    

    for (auto *bg : baseGuis) {
    
        float outputMin, outputMax;
        bool limitsSet = false;
        auto key = make_pair(commandString, bg);
        if (mLimitsMap.count(key) > 0) {
            min = mLimitsMap[key]->inputMin;
            max = mLimitsMap[key]->inputMax;
            outputMax = mLimitsMap[key]->outputMax;
            outputMin = mLimitsMap[key]->outputMin;
            limitsSet = true;
        }
    
        auto *param = &bg->getParameter();
        auto type = param->type();
        
        if (type == "11ofParameterIiE") {
            auto &intParam = param->cast<int>();
            if (!limitsSet) {
                intParam.set(ofMap(v, min, max, intParam.getMin(), intParam.getMax()));
            }
            else {
                intParam.set(ofMap(v, min, max, outputMin, outputMax));
            }
        }
        else if (type == "11ofParameterIfE") {
            auto &floatParam = param->cast<float>();
            if (!limitsSet) {
                floatParam.set(ofMap(v, min, max, floatParam.getMin(), floatParam.getMax()));
            }
            else {
                floatParam.set(ofMap(v, min, max, outputMin, outputMax));
            }
        }
        else if (type == "11ofParameterIhE") {
            auto &ucharParam = param->cast<unsigned char>();
            if (!limitsSet) {
                ucharParam.set(ofMap(v, min, max, ucharParam.getMin(), ucharParam.getMax()));
            }
            else {
                ucharParam.set(ofMap(v, min, max, outputMin, outputMax));
            }
        }
        
    }
}