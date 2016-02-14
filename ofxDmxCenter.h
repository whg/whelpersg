#pragma once

#include "ofThread.h"
#include "ofxDmx.h"

class Fixture;

class ofxDmxCenter : public ofThread {
    
public:
    static ofxDmxCenter& get();

    void addFixture(shared_ptr<Fixture> fixture);
    void assignAddresses();
    
protected:
    
    void threadedFunction();


    map<string, ofxDmx> mDevices;
    vector<shared_ptr<Fixture>> mFixtures;

private:
    void setup();
    ofMutex mutex;
};