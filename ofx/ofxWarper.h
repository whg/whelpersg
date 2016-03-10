// this is a wrapper for whg's fork of ofxGLWarper

#pragma once

#include <array>

#include "ofMain.h"
#include "opencv2/opencv.hpp"
#include "ofxTools.h"
#include "json/json.hpp"

class ofxWarper {
    
    
public:
    ofxWarper() {
        outputFilename = "ofxWarper.config.json";
        calibrated = false;
        editing = false;
        active = false;
        reset();
    }

    void reset() {
        for (int i = 0; i < 16; i++) {
            transformMatrix[i] = 0.0f;
        }
        transformMatrix[10] = 1.0f; // z scale, other things will be changed
        transformMatrix[5] = transformMatrix[0] = transformMatrix[15] = 1.0f;
    }

    void setActive(bool a) {
        if (a != active) {
            active = a;
            if (a) ofRegisterKeyEvents(this);
            else ofUnregisterKeyEvents(this);
        }
    }

    void init() {
        int bwidth = 400;
        int bx = ofGetWidth() / 2 - bwidth / 2;
        int by = ofGetHeight() / 2 - bwidth / 2;
        initToSquare(bx, by, bwidth, bwidth);
    }

    void initToSquare(int x, int y, int w, int h) {
        sourcePoints.push_back(ofVec2f(x, y));
        sourcePoints.push_back(ofVec2f(x+w, y));
        sourcePoints.push_back(ofVec2f(x+w, y+h));
        sourcePoints.push_back(ofVec2f(x, y+h));
    }

    void addSourcePoints(const vector<ofVec2f> &points) {
        sourcePoints = points;
    }

    void addDestinationPoints(const vector<ofVec2f> &points) {
        destinationPoints = points;
    }
        
    void draw() {
        int n = 0;
        for (auto &p : sourcePoints) {
            ofDrawBitmapStringHighlight(ofToString(n + 1), p, ofColor::white, ofColor::black);
            n++;
        }

        n = 0;
        for (auto &p : destinationPoints) {
            ofDrawBitmapStringHighlight(ofToString(n + 1), p, ofColor::black, ofColor::white);
            n++;
        }
    }
    
    void begin() {
        ofPushMatrix();
        glMultMatrixf(transformMatrix);
    }
    
    void end() {
        ofPopMatrix();
    }

    vector<ofVec2f> sourcePoints, destinationPoints;
    GLfloat transformMatrix[16];
    
    void calc() {

        assert(sourcePoints.size() == destinationPoints.size());

        //cv::Point2f and ofVec2f are the same in data
        vector<cv::Point2f> src(sourcePoints.size());
        memcpy(&src[0], &sourcePoints[0], sizeof(ofVec2f) * sourcePoints.size());
        
        vector<cv::Point2f> dest(destinationPoints.size());
        memcpy(&dest[0], &destinationPoints[0], sizeof(ofVec2f) * destinationPoints.size());


        cv::Mat transform = cv::findHomography(src, dest);

        static std::array<int, 9> mapIndex = { 0, 4, 12, 1, 5, 13, 3, 7, 15 };
        for (int i = 0; i < mapIndex.size(); i++) {
            transformMatrix[mapIndex[i]] = transform.at<double>(i / 3, i % 3);
        }
        
        ofLogNotice("ofxWarper") << "calculated projection transformation matrix";
    }

    void keyPressed(ofKeyEventArgs &args) {
        
        int key = args.key;
            
        ofBaseApp *app = ofGetAppPtr();
        ofVec2f p(app->mouseX, app->mouseY);

        // keys 1 - 9 for source
        // keys q, w, e, r, t, y, u, i, o for dest

        if (key >= '1' && key <= '9') {
            int n = key - '1';
            if (n >= sourcePoints.size()) {
                sourcePoints.resize(n + 1);
            }
            sourcePoints[n] = p;
        }

        static std::array<char, 9> destinationKeys = { 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o' };
        int n = 0;
        for (auto k : destinationKeys) {
            if (key == k) {
                if (n >= destinationPoints.size()) {
                    destinationPoints.resize(n + 1);
                }
                destinationPoints[n] = p;
            }
            n++;
        }

        KEY('c', calc());
        KEY('s', save());
        KEY('l', load());
        KEY('r', reset());
    }

    void keyReleased(ofKeyEventArgs &args) {}

    void save() {
        using json = nlohmann::json;
        json output;

        json src;
        for (auto &p : sourcePoints) {
            src.push_back({ { "x", p.x }, { "y", p.y } });
        }

        output["sourcePoints"] = src;

        json dst;
        for (auto &p : destinationPoints) {
            dst.push_back({ { "x", p.x },{ "y", p.y } });
        }

        output["destinationPoints"] = dst;

        ofstream outfile;
        outfile.open(ofToDataPath(outputFilename));
        outfile << output.dump(2);
        outfile.close();
        
        ofLogNotice() << "saved file to " << outputFilename;
    }

    void load() {
        using json = nlohmann::json;

        std::ifstream file(ofToDataPath(outputFilename));
        std::stringstream buffer;
        buffer << file.rdbuf();

        json input = json::parse(buffer.str());

        sourcePoints.clear();
        destinationPoints.clear();

        for (auto &p : input["sourcePoints"]) {
            sourcePoints.push_back(ofVec2f(p["x"], p["y"]));
        }

        for (auto &p : input["destinationPoints"]) {
            destinationPoints.push_back(ofVec2f(p["x"], p["y"]));
        }

    }

    bool editing;
    bool calibrated;
    bool active;
    std::string outputFilename;
};
