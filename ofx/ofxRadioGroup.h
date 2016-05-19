#pragma once

#include "ofMain.h"

class ofxRadioGroupEventArgs : public ofEventArgs {
public:
    string name;
};

class ofxRadioGroup : public ofParameterGroup {
    // TODO: add callbacks on selection
public:
    ofxRadioGroup(): selectedOne(false), currentOn("") {
        ofAddListener(this->parameterChangedE(), this, &ofxRadioGroup::previewGroupChange);
    }
    
    void addChoice(string name, bool value=false) {
        
        if (!selectedOne) {
            value = true;
            selectedOne = true;
        }
        
        choices[name].set(name, value);
        add(choices[name]);
        
        if (value) {
            currentOn = name;
        }
    }
    
    string getCurrentChoice() {
        return currentOn;
    }
    
    
    void previewGroupChange(ofAbstractParameter &param) {
        
        if (currentOn != "" && currentOn != param.getName()) {
            auto &current = this->getBool(currentOn);
            current.set(false);
        }
        
        auto &toggle = this->getBool(param.getName());
        if (toggle.get()) {
            currentOn = toggle.getName();
        }
        else {
            currentOn = "";
        }
        
        if (currentOn != "") {
            ofxRadioGroupEventArgs e;
            e.name = currentOn;
            ofNotifyEvent(changeEvent, e);
        }
    }
    
    ofEvent<ofxRadioGroupEventArgs> changeEvent;
    
protected:
    map<string, ofParameter<bool>> choices;
    string currentOn;
    bool selectedOne;
    
};