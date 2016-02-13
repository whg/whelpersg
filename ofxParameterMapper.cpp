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
    ofAddListener(ofxOscCenter::newMessageEvent, this, &ofxParameterMapper::newOscMessage);
}

void ofxParameterMapper::guiSelected(ofxGuiSelectedArgs &args) {
    if (args.type == OF_MOUSE_BUTTON_RIGHT) {
        auto path = getGuiPath(args.baseGui);
        
        auto &guiElem = ofxPanelManager::get().getGuiElem(path);
        
        mSourceMap.emplace(path, unique_ptr<Sources>(new Sources(guiElem, path)));
        
        ofxPanelManager::get().addPanel(mSourceMap[path]->getPanel());
        
    }
}

/// Look through all the sources to find the bool that matches the callback
void ofxParameterMapper::updateOscSourceMapping(bool &b) {
    
    for (auto &pair : mSourceMap) {
        
        const auto &sources = pair.second;
        for (auto &oscSource : sources->mOscSources) {
            if (&oscSource.get() == &b) {
                ofxOscCenter::Command command { oscSource.getFirstParent().getName(), oscSource.getName() };
                if (b) {
                    mParamMap[command.toString()] = &sources->guiElem.getParameter();

                }
                else {
                    mParamMap.erase(command.toString());
                }
            }
        }
    }
}



void ofxParameterMapper::newOscMessage(ofxOscCenterNewMessageArgs &args) {
    
    auto commandString = args.command.toString();
    
    if (mParamMap.count(commandString) != 0) {
        
        if (args.command.address.find("/MIDI/note") != string::npos) {
            
            int velocity = args.message.getArgAsInt(2);
            
            auto *param = mParamMap[commandString];
            auto type = param->type();
            
            if (type == "11ofParameterIiE") {
                auto &intParam = param->cast<int>();
                intParam.set(ofMap(velocity, 0, 127, intParam.getMin(), intParam.getMax()));
            }
            else if (type == "11ofParameterIfE") {
                auto &floatParam = param->cast<float>();
                floatParam.set(ofMap(velocity, 0, 127, floatParam.getMin(), floatParam.getMax()));
            }
            else if (type == "11ofParameterIhE") {
                auto &ucharParam = param->cast<unsigned char>();
                ucharParam.set(ofMap(velocity, 0, 127, ucharParam.getMin(), ucharParam.getMax()));
            }
            
            
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////
/// Sources

ofxParameterMapper::Sources::Sources(ofxBaseGui &guiElem, string path): guiElem(guiElem), panel(nullptr), path(path) {
    midiChannel.set("midi channel", 10, 0, 127);
    
    // Osc
    auto oscCommands = ofxOscCenter::get().getReceivedCommands();
    for (auto &command : oscCommands) {
        addParameter(command);

    }
    
    mOscGroup.setup("Osc sources");

    for (auto &pair : mTracks) {
        mOscGroup.add(&pair.second);

    }
    

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
    
    addParameter(args.command);
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

