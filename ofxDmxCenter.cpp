//
//  ofxDmxCenter.cpp
//  desperados studio test
//
//  Created by Will Gallia on 14/02/2016.
//
//

#include "ofxDmxCenter.h"

#include "Fixture.h"

shared_ptr<ofxDmxCenter> dmxCenter = nullptr;


ofxDmxCenter& ofxDmxCenter::get() {
    if (dmxCenter == nullptr) {
        dmxCenter = shared_ptr<ofxDmxCenter>(new ofxDmxCenter);
        dmxCenter->setup();
        dmxCenter->startThread();
    }
    return *dmxCenter.get();
}

void ofxDmxCenter::setup() {

    ofSerial serial;
    auto devices = serial.getDeviceList();
    for (ofSerialDeviceInfo device : devices) {
        string name = device.getDeviceName();
        cout << name << endl;
        if (name.find("tty.usb") != string::npos) {
            auto parts = ofSplitString(name, "-");
            string extension = parts[parts.size()-1];
            
            mDevices[extension].connect(name, 512);
        }
    }
    
}

void ofxDmxCenter::threadedFunction() {
    while(isThreadRunning()) {
        
        mutex.lock();
        for (auto &fixture : mFixtures) {
            
            auto &dmxDevice = mDevices[fixture->getDmxUniverse()];
            const auto &parameters = fixture->getParameters();
            int startAddress = fixture->getDmxStartAddress();
            
            for (auto &paramPair : parameters) {
                int address = paramPair.first + startAddress;
                auto param = paramPair.second;
                int value = 0;
                
                if (param->type() == typeid(ofParameter<int>).name()) {
                    dmxDevice.setLevel(address, param->cast<int>());
                }
                else if (param->type() == typeid(ofParameter<unsigned char>).name()) {
                    dmxDevice.setLevel(address, param->cast<unsigned char>());
                }
                else if (param->type() == typeid(ofParameter<ofColor>).name()) {
                    auto &color = param->cast<ofColor>().get();
                    
                    dmxDevice.setLevel(address, color.r);
                    dmxDevice.setLevel(address+1, color.g);
                    dmxDevice.setLevel(address+2, color.b);
                }
                
            }
        }
        
        mutex.unlock();
        
        for (auto &dmxPair : mDevices) {
            dmxPair.second.update();
        }
        
        sleep(10);
    }
}

void ofxDmxCenter::addFixture(shared_ptr<Fixture> fixture) {
    ofScopedLock lock(mutex);
    mFixtures.push_back(fixture);
    
}

void ofxDmxCenter::assignAddresses() {
    
    ofScopedLock lock(mutex);
    
    auto deviceIterator = mDevices.begin();
    int currentAddress = 0;
    
    for (auto &fixture : mFixtures) {
        if (currentAddress + fixture->getNumChannels() >= 512) {
            deviceIterator++;
            
            if (deviceIterator == mDevices.end()) {
                ofLogError("ofxDmxCenter") << "Exceeded DMX universes size.";
                ofExit();
                return;
            }
        }
    
        fixture->setDmxStartAddress(currentAddress);
        fixture->setDmxUniverse(deviceIterator->first);
        currentAddress+= fixture->getNumChannels();
        
    }
}