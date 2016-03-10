#pragma once

#include "ofThread.h"
#include "ofxDmx.h"
#include "ofxGui.h"

class Fixture;

class ofxDmxCenter : public ofThread {
    
public:
    static ofxDmxCenter& get();

    void addFixture(shared_ptr<Fixture> fixture);
    void assignAddresses();
    
    void openFixturesGui();
    void openFixtureGui(bool &b);
    
protected:
    
    void threadedFunction();
    
    void checkForNewDevices();
    float mLastDeviceCheckTime;
    
    map<string, ofxDmx> mDevices;
    vector<shared_ptr<Fixture>> mFixtures;
    
    vector<shared_ptr<ofxButton>> mFixtureButtons;
    shared_ptr<ofxPanel> mPanel;
    
private:
    void setup();
    ofMutex mutex;
};