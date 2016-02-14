#include "ofxParameterMapper.h"

#include "ofxPanelManager.h"
#include "ofxOscCenter.h"


#define SOURCE_PREFIX "~"

shared_ptr<ofxParameterMapper> parameterMapper = nullptr;
ofMutex ofxParameterMapper::addSourceMutex;


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

    auto path = getGuiPath(args.baseGui);
    auto &guiElem = ofxPanelManager::get().getGuiElem(path);

    if (args.type == OF_MOUSE_BUTTON_RIGHT) {
        guiElem.path = path;
        
        if (path.find(SOURCE_PREFIX) != string::npos) {
            
            auto toEffectPath = getGuiRoot(&guiElem)->effectingPath;
            auto &toEffectElem = ofxPanelManager::get().getGuiElem(toEffectPath);

            
            auto parts = ofSplitString(path, PATH_DELIMITER);
            ofxOscCenter::Command command { parts[parts.size()-2], parts[parts.size()-1] };
            
            auto key = make_pair(command.toString(), &toEffectElem);
            if (mLimitsMap.count(key) == 0) {
                mLimitsMap.emplace(key, unique_ptr<InputLimits>(new InputLimits(guiElem, command.toString())));
            }
            
            ofxPanelManager::get().addPanel(mLimitsMap[key]->getPanel());

        }
        else {
            if (mSourceMap.count(path) == 0) {
                mSourceMap.emplace(path, unique_ptr<Sources>(new Sources(guiElem, path)));
            }
            ofxPanelManager::get().addPanel(mSourceMap[path]->getPanel());

        }
        
    }
    else if (args.type == OF_MOUSE_BUTTON_MIDDLE) {
        if (mParameterLimits.count(path) == 0) {
            mParameterLimits.emplace(path, unique_ptr<OutputLimits>(new OutputLimits(guiElem, path)));
        }
        ofxPanelManager::get().addPanel(mParameterLimits[path]->getPanel());

    }
}

/// Look through all the sources to find the bool that matches the callback
void ofxParameterMapper::updateOscSourceMapping(bool &b) {
    
    ofScopedLock lock(addSourceMutex);
    
    for (auto &pair : mSourceMap) {
        
        if (Sources *sources = dynamic_cast<Sources*>(pair.second.get())) {
            for (auto &oscSource : sources->mOscSources) {
                if (&oscSource.get() == &b) {
                    ofxOscCenter::Command command { oscSource.getFirstParent().getName(), oscSource.getName() };

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

                    }
                }
            }
        }
    }
    
    save();
}


void ofxParameterMapper::newOscMessage(ofxOscCenterNewMessageArgs &args) {
    
    auto commandString = args.command.toString();
    
    if (mParamMap.count(commandString) != 0) {
        
        if (args.command.address.find("/MIDI/") != string::npos) {
            // handle MIDI cc too, which has the value byte in postion 2 too
            
            int velocity = args.message.getArgAsInt(2);
            modifyParams(mParamMap[commandString], velocity, 0, 127, commandString);
            
        }
        else if (args.command.address.find("/PC") != string::npos) {
            
            float value = args.message.getArgAsFloat(1);
            modifyParams(mParamMap[commandString], value, 0.0f, 1.0f, commandString);
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

    xml.setToParent();
    xml.addChild("limits-maps");
    xml.setTo("limits-maps");

    
    for (auto &pair : mLimitsMap) {
        
        ofXml map;
        map.addChild("map");
        map.setTo("map");
        map.addValue("key", pair.first.first + "^" + getGuiPath(pair.first.second));
        map.addValue("inputMin", pair.second->inputMin);
        map.addValue("inputMax", pair.second->inputMax);
        map.addValue("path", getGuiPath(&pair.second->guiElem));
        xml.addXml(map);
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
    
    int n;
    if (version >= 0.1) {
        xml.setTo("//param-maps");
        n = xml.getNumChildren();
        
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
            mSourceMap[path]->addParameter(ofxOscCenter::Command::fromString(commandString), true);
            
            ofxPanelManager::get().addPanel(mSourceMap[path]->getPanel(), true);

            
            mParamMap[commandString].push_back(&guiElem);
        }
        

        xml.setTo("//limits-maps");
        n = xml.getNumChildren();
        for (int i = 0; i < n; i++) {
            xml.setTo("//limits-maps");
            xml.setToChild(i);
            
            string keyString = xml.getValue("key");
            auto parts = ofSplitString(keyString, "^");
            auto &effectingGuiElem = ofxPanelManager::get().getGuiElem(parts[1]);
            auto commandString = parts[0];
            
            auto key = make_pair(commandString, &effectingGuiElem);
            auto &guiElem = ofxPanelManager::get().getGuiElem(xml.getValue("path"));
            
            mLimitsMap.emplace(key, unique_ptr<InputLimits>(new InputLimits(guiElem, commandString)));
            
            mLimitsMap[key]->inputMin = xml.getFloatValue("inputMin");
            mLimitsMap[key]->inputMax = xml.getFloatValue("inputMax");

            
        }
    }
    else {
        ofLogError() << "can't decode file version " << version;
        return;
    }
    
    ofLogNotice() << "loaded from " << filename;

}


void ofxParameterMapper::setOutputLimitMin(float &v) {
    for (auto &pair : mParameterLimits) {
        if (&pair.second->outputMin.get() == &v) {
            auto &param = pair.second->guiElem.getParameter();
            
            if (param.type() == typeid(ofParameter<float>).name()) {
                param.cast<float>().setMin(v);
            }
            else if (param.type() == typeid(ofParameter<int>).name()) {
                param.cast<int>().setMin(int(v));
            }
            else if (param.type() == typeid(ofParameter<unsigned char>).name()) {
                param.cast<unsigned char>().setMin(int(v));
            }
            
            pair.second->guiElem.setNeedsRedraw();
            return;
        }
    }
}

void ofxParameterMapper::setOutputLimitMax(float &v) {
    for (auto &pair : mParameterLimits) {
        if (&pair.second->outputMax.get() == &v) {
            auto &param = pair.second->guiElem.getParameter();
            
            if (param.type() == typeid(ofParameter<float>).name()) {
                param.cast<float>().setMax(v);
            }
            else if (param.type() == typeid(ofParameter<int>).name()) {
                param.cast<int>().setMax(int(v));
            }
            else if (param.type() == typeid(ofParameter<unsigned char>).name()) {
                param.cast<unsigned char>().setMax(int(v));
            }
            
            pair.second->guiElem.setNeedsRedraw();
            return;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
/// Sources

ofxParameterMapper::Sources::Sources(ofxBaseGui &guiElem, string path): ofxParameterMapper::BaseMapper(guiElem, path) {
    midiChannel.set("midi channel", 10, 0, 127);
    titlePrefix = getGuiRoot(&guiElem)->getName() + SOURCE_PREFIX;
    // Osc
    auto oscCommands = ofxOscCenter::get().getReceivedCommands();
    for (auto &command : oscCommands) {
        addParameter(command);

    }
    
    ofAddListener(ofxOscCenter::newCommandEvent, this, &ofxParameterMapper::Sources::updateOscList);
    
    cout << "added with path: " << path << endl;
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

void ofxParameterMapper::Sources::addParameter(const ofxOscCenter::Command &command, bool value) {
    
    ofScopedLock lock(ofxParameterMapper::addSourceMutex);

    if (mTracks.count(command.track) == 0) {
        mTracks[command.track].setup(command.track);
//        mOscGroup.add(&mTracks[command.track]);
        getPanel()->add(&mTracks[command.track]);
    }
    else {
        if (mTracks[command.track].getControl(command.address) != nullptr) {
            return;
        }
    }
    
    mOscSources.push_back(ofParameter<bool>(command.address, value));
    
    
    auto &param = mOscSources.back();
    param.addListener(parameterMapper.get(), &ofxParameterMapper::updateOscSourceMapping);

    mTracks[command.track].add(param);

    mOscGroup.minimize();
    mOscGroup.maximize();
    
    for (auto &pair : mTracks) {
        pair.second.minimize();
        pair.second.maximize();
    }
    
    cout << "added " << command.toString() << " to " << path << endl;
}


ofxParameterMapper::OutputLimits::OutputLimits(ofxBaseGui &guiElem, string path): ofxParameterMapper::BaseMapper(guiElem, path) {
    
    getPanel();
    outputMin.set("output min", 0, 0, 255);
    outputMax.set("output max", 127, 0, 255);
    
    outputMin.addListener(parameterMapper.get(), &ofxParameterMapper::setOutputLimitMin);
    outputMax.addListener(parameterMapper.get(), &ofxParameterMapper::setOutputLimitMax);
    
    panel->add(outputMin);
    panel->add(outputMax);
    
    cout << "added base limit " << endl;
}

ofxParameterMapper::InputLimits::InputLimits(ofxBaseGui &guiElem, string commandString): ofxParameterMapper::BaseMapper(guiElem, commandString) {

    getPanel();
    panel->add(inputMin.set("input min", 0, 0, 127));
    panel->add(inputMax.set("input max", 127, 0, 127));
    
    cout << "added limit " << commandString << endl;
}
