#include "ofxDmxCenter.h"

#include "Fixture.h"

#include "ofxPanelManager.h"

shared_ptr<ofxDmxCenter> dmxCenter = nullptr;
const int DEVICE_POLLING_INTERVAL = 5; // seconds

ofxDmxCenter& ofxDmxCenter::get() {
    if (dmxCenter == nullptr) {
        dmxCenter = shared_ptr<ofxDmxCenter>(new ofxDmxCenter);
        dmxCenter->setup();
    }
    return *dmxCenter.get();
}

void ofxDmxCenter::setup() {

    checkForNewDevices();
    
    mPanel = make_shared<ofxPanel>();
    mPanel->setup("DMX Center");

    dmxCenter->startThread();
}

void ofxDmxCenter::checkForNewDevices() {
    
    ofSerial serial;
    auto devices = serial.getDeviceList();
    mLastDeviceCheckTime = ofGetElapsedTimef();
    
    ofScopedLock lock(mutex);
    
    for (ofSerialDeviceInfo device : devices) {
        string name = device.getDeviceName();
        if (name.find("tty.usb") != string::npos) {
            auto parts = ofSplitString(name, "-");
            string extension = parts[parts.size()-1];
            
            if (mDevices.count(extension) == 0) {
            
                mDevices[extension].connect(name, 512);
                ofLogNotice("ofxDmxCenter") << "connected " << name << endl;
            }
        }
    }
}

void ofxDmxCenter::threadedFunction() {
    while(isThreadRunning()) {
        
        mutex.lock();
        for (auto &fixture : mFixtures) {
            
            
            try {
                auto &dmxDevice = mDevices.at(fixture->getDmxUniverse());
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
                
                

            } catch (const out_of_range &e) {}
            
        }
  
        
        mutex.unlock();
        
        for (auto &dmxPair : mDevices) {
            if (dmxPair.first != "-") {
                dmxPair.second.update();
            }
        }
        
        if (ofGetElapsedTimef() - mLastDeviceCheckTime > DEVICE_POLLING_INTERVAL) {
            checkForNewDevices();
        }
        
        sleep(10);
    }
}

void ofxDmxCenter::addFixture(shared_ptr<Fixture> fixture) {
    ofScopedLock lock(mutex);
    mFixtures.push_back(fixture);
    
    auto button = make_shared<ofxButton>();
    button->setup(fixture->getName());
    mFixtureButtons.push_back(button);
    button->addListener(this, &ofxDmxCenter::openFixtureGui);
    mPanel->add(button.get());
}

void ofxDmxCenter::assignAddresses() {
    
    ofScopedLock lock(mutex);
    
    auto deviceIterator = mDevices.begin();
    
    if (deviceIterator == mDevices.end()) {
        ofLogError("ofxDmxCenter") << "no DMX boxes connected";
    }
    
    int currentAddress = 0;
    
    map<string, set<int>> usedSlots;
    for (auto &fixture : mFixtures) {
        auto startAddress = fixture->getDmxStartAddress();
        if (startAddress != 0) {
            auto numChannels = fixture->getNumChannels();
            if (usedSlots[deviceIterator->first].count(startAddress) ||
                usedSlots[deviceIterator->first].count(startAddress + numChannels)) {
                deviceIterator++;
                
                if (deviceIterator == mDevices.end()) {
                    ofLogError("ofxDmxCenter") << "Can't assign DMX addresses with current manual config";
                    return;
                }
            }
        
            for (int i = 0; i < numChannels; i++) {
                usedSlots[deviceIterator->first].insert(startAddress + i);
                fixture->setDmxUniverse(deviceIterator->first);
            }
            
        }
    }
    
    for (auto &fixture : mFixtures) {
        
        auto lastAddress = currentAddress + fixture->getNumChannels();
        
        auto &usedSlotsForDevice = usedSlots[deviceIterator->first];
        if (usedSlotsForDevice.count(currentAddress) != 0 ||
            usedSlotsForDevice.count(lastAddress) != 0) {
            int addressToStart = 0;
            for (int i = lastAddress + 1; i < 512; i++) {
                if (usedSlotsForDevice.count(i) == 0) {
                    addressToStart = i;
                }
            }
            if (addressToStart == 0) {
                deviceIterator++;
            }
            else {
                currentAddress = addressToStart;
            }
        }

    
        if (lastAddress >= 512) {
            deviceIterator++;
            currentAddress = 0;
            
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

void ofxDmxCenter::openFixturesGui() {
    
    ofxPanelManager::get().addPanel(mPanel);
}

void ofxDmxCenter::openFixtureGui(bool &b) {
    int i = 0;
    for (auto &fixture : mFixtureButtons) {
        if (&b == &fixture->getParameter().cast<bool>().get()) {
            ofxPanelManager::get().addPanel(mFixtures[i]->getPanel());
        }
        i++;
    }
}