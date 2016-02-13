 //
//  ofxParameterMapper.cpp
//  desperados studio test
//
//  Created by Will Gallia on 11/02/2016.
//
//

#include "ofxParameterMapper.h"

#include "ofxPanelManager.h"
#include "ofxOscCenter.h"

shared_ptr<ofxParameterMapper> parameterMapper = nullptr;

shared_ptr<ofxParameterMapper> ofxParameterMapper::get() {
    if (parameterMapper == nullptr) {
        parameterMapper = shared_ptr<ofxParameterMapper>(new ofxParameterMapper);
        parameterMapper->setup();
    }
    return parameterMapper;
}

void ofxParameterMapper::setup() {
    ofAddListener(ofxBaseGui::guiSelectedEvent, this, &ofxParameterMapper::guiSelected);
}

void ofxParameterMapper::guiSelected(ofxGuiSelectedArgs &args) {
    if (args.type == OF_MOUSE_BUTTON_RIGHT) {
        auto path = getGuiPath(args.baseGui);
        
        auto &guiElem = ofxPanelManager::get().getGuiElem(path);
        
        sourceMap.emplace(path, unique_ptr<Sources>(new Sources(guiElem, path)));
        
        ofxPanelManager::get().addPanel(sourceMap[path]->getPanel());
        
    }
}

void ofxParameterMapper::updateOscSourceMapping(bool &b) {
    
    for (auto &pair : sourceMap) {
        
        const auto &sources = pair.second;
        for (auto &oscSource : sources->mOscSources) {
            if (&oscSource.get() == &b) {
                cout << "gotcha: " << oscSource.getName() << " : " << oscSource.getFirstParent().getName() << endl;
                subscribeParameterToOsc(sources->guiElem.getParameter(), oscSource.getName());
            }
        }
    }
}

void ofxParameterMapper::subscribeParameterToOsc(ofAbstractParameter &param, string source) {
    
    
}

////////////////////////////////////////////////////////////////////////////////////
/// Sources

ofxParameterMapper::Sources::Sources(ofxBaseGui &guiElem, string path): guiElem(guiElem), panel(nullptr), path(path) {
    midiChannel.set("midi channel", 10, 0, 127);
    
    // Osc
    
    auto oscCommands = ofxOscCenter::get().getReceivedCommands();
    for (auto &command : oscCommands) {
        addParameter(command);
//        if (mTracks.count(command.track) == 0) {
//            mTracks[command.track].setup(command.track);
//        }
//        mOscSources.push_back(ofParameter<bool>(command.address, false));
//
//        mTracks[command.track].add(mOscSources.back());

    }
    
    mOscGroup.setup("Osc sources");

    for (auto &pair : mTracks) {
        mOscGroup.add(&pair.second);

    }
    
//    auto oscTracks = ofxOscCenter::get().getReceivedTracks();
//
//    auto oscSources = ofxOscCenter::get().getReceivedAddresses();
////    vector<string> oscSources = { "hi there", "something", "otehr thigs" };
//
//    mOscGroup.setName("Osc sources");
//    for (auto &track : oscTracks) {
//        
//
//        
//        for (auto &source : oscSources) {
//            
//            mOscSources.push_back(ofParameter<bool>(source, false));
//            auto &param = mOscSources.back();
//            
//            param.addListener(parameterMapper.get(), &ofxParameterMapper::updateOscSourceMapping);
//            mOscGroup.add(param);
//        }
//        
//    }
    ofAddListener(ofxOscCenter::newCommandEvent, this, &ofxParameterMapper::Sources::updateOscList);
}

//void ofxParameterMapper::Sources::something(ofParameter<bool> &param) {
//    
//}


shared_ptr<ofxPanel> ofxParameterMapper::Sources::getPanel() {
    if (panel == nullptr) {
        panel = make_shared<ofxPanel>();
        panel->setup(guiElem.getName() + " sources");
    }
    panel->clear();
    panel->setPosition(guiElem.getPosition() + ofPoint(guiElem.getWidth(), 0));
    panel->add(midiChannel);
    
    panel->add(&mOscGroup);
    return panel;
}


void ofxParameterMapper::Sources::updateOscList(ofxOscCenterCommandArgs &args) {
    
//    auto command = args.command;
    addParameter(args.command);

//    mOscSources.push_back(ofParameter<bool>(command.address, false));
////    mOscGroup.add(mOscSources.back());
//
//    if (mTracks.count(command.track) == 0) {
//        mTracks[command.track].setup(command.track);
//        mOscGroup.add(&mTracks[command.track]);
//    }
//    
//    mTracks[args.command.track].add(mOscSources.back());

//    mOscGroup.maximizeAll();
//    mTracks[args.command.track].minimize();
//    mTracks[args.command.track].maximizeAll();
    
//    if (mTracks.count(args.command.track) == 0) {
//        auto &group = mTracks[args.command.track];
//        
//    }

    getPanel();
}

void ofxParameterMapper::Sources::addParameter(const ofxOscCenter::Command &command) {
    
    mOscSources.push_back(ofParameter<bool>(command.address, false));
    
    if (mTracks.count(command.track) == 0) {
        mTracks[command.track].setup(command.track);
        mOscGroup.add(&mTracks[command.track]);
    }
    
    auto &param = mOscSources.back();
    param.addListener(parameterMapper.get(), &ofxParameterMapper::updateOscSourceMapping);

    mTracks[command.track].add(param);

}

