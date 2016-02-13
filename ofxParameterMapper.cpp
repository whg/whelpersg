#include "ofxParameterMapper.h"

#include "ofxPanelManager.h"
#include "ofxOscCenter.h"

#define SOURCE_PREFIX "~"

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
                
        if (path.find(SOURCE_PREFIX) != string::npos) {
            mSourceMap.emplace(path, unique_ptr<Limits>(new Limits(guiElem, path)));
        }
        else {
            mSourceMap.emplace(path, unique_ptr<Sources>(new Sources(guiElem, path)));
        }
        cout << path << endl;
        
        ofxPanelManager::get().addPanel(mSourceMap[path]->getPanel());
        
    }
}

/// Look through all the sources to find the bool that matches the callback
void ofxParameterMapper::updateOscSourceMapping(bool &b) {
    
    for (auto &pair : mSourceMap) {
        
        if (Sources *sources = dynamic_cast<Sources*>(pair.second.get())) {
            for (auto &oscSource : sources->mOscSources) {
                if (&oscSource.get() == &b) {
                    ofxOscCenter::Command command { oscSource.getFirstParent().getName(), oscSource.getName() };
                    auto *param = &sources->guiElem.getParameter();
                    
                    if (b) {
                        mParamMap[command.toString()].push_back(param);
                    }
                    else {
                        auto &params = mParamMap[command.toString()];
                        for (auto it = params.begin(); it != params.end(); ++it) {
                            if (*it == param) {
                                params.erase(it);
                                break;
                            }
                        }
//                        auto it = mParamMap[command.toString()].
//                        mParamMap.erase(command.toString());
                    }
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
            
            for (auto *param : mParamMap[commandString]) {
            
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
}


////////////////////////////////////////////////////////////////////////////////////
/// Sources

ofxParameterMapper::Sources::Sources(ofxBaseGui &guiElem, string path): ofxParameterMapper::BaseMapper(guiElem, path) {
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
    
    titlePrefix = SOURCE_PREFIX;
    getPanel()->add(&mOscGroup);
    ofAddListener(ofxOscCenter::newCommandEvent, this, &ofxParameterMapper::Sources::updateOscList);
}

//void ofxParameterMapper::Sources::something(ofParameter<bool> &param) {
//    
//}


shared_ptr<ofxPanel> ofxParameterMapper::BaseMapper::getPanel() {
    if (panel == nullptr) {
        panel = make_shared<ofxPanel>();
        panel->setup(titlePrefix + ofSplitString(guiElem.getName(), " ")[0]);
    }
//    panel->clear();
    panel->setPosition(guiElem.getPosition() + ofPoint(guiElem.getWidth(), 0));
//    panel->add(midiChannel);
    
//    panel->add(&mOscGroup);
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


ofxParameterMapper::Limits::Limits(ofxBaseGui &guiElem, string path): ofxParameterMapper::BaseMapper(guiElem, path) {
//    midiChannel.set("midi channel", 10, 0, 127);
//
    getPanel();
    panel->add(inputMin.set("input min", 0, 0, 127));
    panel->add(inputMax.set("input max", 127, 0, 127));
    panel->add(outputMin.set("output min", 0, 0, 127));
    panel->add(outputMax.set("output max", 127, 0, 127));
//    inputMin, inputMax, outputMin, outputMax
//    
//    // Osc
//    auto oscCommands = ofxOscCenter::get().getReceivedCommands();
//    for (auto &command : oscCommands) {
//        addParameter(command);
//        
//    }
//    
//    mOscGroup.setup("Osc sources");
//    
//    for (auto &pair : mTracks) {
//        mOscGroup.add(&pair.second);
//        
//    }
//    
//    titlePrefix = SOURCE_PREFIX;
//    getPanel()->add(&mOscGroup);
//    ofAddListener(ofxOscCenter::newCommandEvent, this, &ofxParameterMapper::Sources::updateOscList);
}
