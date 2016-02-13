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
    
    load();
}

void ofxParameterMapper::guiSelected(ofxGuiSelectedArgs &args) {
    if (args.type == OF_MOUSE_BUTTON_RIGHT) {
        auto path = getGuiPath(args.baseGui);
        auto &guiElem = ofxPanelManager::get().getGuiElem(path);
        guiElem.path = path;
        cout << guiElem.getName() << endl;
        
        if (path.find(SOURCE_PREFIX) != string::npos) {
            
            auto toEffect = getGuiRoot(&guiElem)->effectingPath;
            auto &toEffectElem = ofxPanelManager::get().getGuiElem(toEffect);
            auto *key = &toEffectElem.getParameter();
            mLimitsMap.emplace(key, unique_ptr<Limits>(new Limits(toEffectElem, path)));
            ofxPanelManager::get().addPanel(mLimitsMap[key]->getPanel());

        }
        else {
            mSourceMap.emplace(path, unique_ptr<Sources>(new Sources(guiElem, path)));
            ofxPanelManager::get().addPanel(mSourceMap[path]->getPanel());

        }
        
    }
}

/// Look through all the sources to find the bool that matches the callback
void ofxParameterMapper::updateOscSourceMapping(bool &b) {
    
    for (auto &pair : mSourceMap) {
        
        if (Sources *sources = dynamic_cast<Sources*>(pair.second.get())) {
            for (auto &oscSource : sources->mOscSources) {
                if (&oscSource.get() == &b) {
                    ofxOscCenter::Command command { oscSource.getFirstParent().getName(), oscSource.getName() };
//                    auto *param = &sources->guiElem.getParameter();
                    auto *bg = &sources->guiElem;
                    if (b) {
                        mParamMap[command.toString()].push_back(bg);
                    }
                    else {
                        auto &params = mParamMap[command.toString()];
                        for (auto it = params.begin(); it != params.end(); ++it) {
                            if (*it == bg) {
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
    
    save();
}

void ofxParameterMapper::updateLimitMapping(float& v) {
    for (auto &pair : mLimitsMap) {
        
    }
}


void ofxParameterMapper::newOscMessage(ofxOscCenterNewMessageArgs &args) {
    
    auto commandString = args.command.toString();
    
    if (mParamMap.count(commandString) != 0) {
        
        if (args.command.address.find("/MIDI/note") != string::npos) {
            
            int velocity = args.message.getArgAsInt(2);
            modifyParams(mParamMap[commandString], velocity, 0, 127);
            
        }
        else if (args.command.address.find("/PC") != string::npos) {
            
            float value = args.message.getArgAsFloat(1);
            modifyParams(mParamMap[commandString], value, 0.0f, 1.0f);
        }
    }
}

void ofxParameterMapper::save() {

    ofXml xml;
    xml.addChild("ofxParameterMapper");
    xml.setTo("ofxParameterMapper");
    xml.setAttribute("version", "0.1");
    
    xml.addChild("param-maps");
    xml.setTo("param-maps");
    
    for (auto &pair : mParamMap) {
        auto command  = pair.first;

        for (auto &bg : pair.second) {
            ofXml map;
            map.addChild("map");
            map.setTo("map");
            map.addValue("command", command);
            map.addValue("param-path", getGuiPath(bg));
            xml.addXml(map);
        }
    }
    
    
    xml.save("parameter-map.xml");

}

void ofxParameterMapper::load() {
    
    ofXml xml;
    
    string filename = "parameter-map.xml";
    bool success = xml.load(filename);
    
    if (!success) {
        ofLogError() << "can't load from " << filename;
        return;
    }
    
    xml.setTo("ofxParameterMapper");
    float version = ofToFloat(xml.getAttribute("version"));
    
    if (version >= 0.1) {
        xml.setTo("//param-maps");
        int n = xml.getNumChildren();
        
        for (int i = 0; i < n; i++) {
            xml.setTo("//param-maps");
            xml.setToChild(i);
            string commandString = xml.getValue("command");
            string path = xml.getValue("param-path");
            auto &guiElem = ofxPanelManager::get().getGuiElem(path);
            
            if (mSourceMap.count(path) == 0) {
                mSourceMap.emplace(path, unique_ptr<Sources>(new Sources(guiElem, path)));
            }
            
            // do we need to make sure this isn't added twice?
            mSourceMap[path]->addParameter(ofxOscCenter::Command::fromString(commandString));
            
            mParamMap[commandString].push_back(&guiElem);
        }
    }
    else {
        ofLogError() << "can't decode file version " << version;
        return;
    }
    
    ofLogNotice() << "loaded from " << filename;

}


////////////////////////////////////////////////////////////////////////////////////
/// Sources

ofxParameterMapper::Sources::Sources(ofxBaseGui &guiElem, string path): ofxParameterMapper::BaseMapper(guiElem, path) {
    midiChannel.set("midi channel", 10, 0, 127);
    titlePrefix = SOURCE_PREFIX;
    // Osc
    auto oscCommands = ofxOscCenter::get().getReceivedCommands();
    for (auto &command : oscCommands) {
        addParameter(command);

    }
    
    mOscGroup.setup("Osc sources");

    for (auto &pair : mTracks) {
        mOscGroup.add(&pair.second);

    }
    
    getPanel()->add(&mOscGroup);
    ofAddListener(ofxOscCenter::newCommandEvent, this, &ofxParameterMapper::Sources::updateOscList);
}



shared_ptr<ofxPanel> ofxParameterMapper::BaseMapper::getPanel() {
    if (panel == nullptr) {
        panel = make_shared<ofxPanel>();
        panel->setup(titlePrefix + ofSplitString(guiElem.getName(), " ")[0]);
    }

    panel->setPosition(guiElem.getPosition() + ofPoint(guiElem.getWidth(), 0));
    panel->effectingPath = guiElem.path;

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

    getPanel();
    panel->add(inputMin.set("input min", 0, 0, 127));
    panel->add(inputMax.set("input max", 127, 0, 127));
    panel->add(outputMin.set("output min", 0, 0, 255));
    panel->add(outputMax.set("output max", 127, 0, 255));
    
    inputCommand = path;
    paramToEffect = &guiElem.getParameter();
}
