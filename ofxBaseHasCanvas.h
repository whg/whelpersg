//
//  BaseHasCanvas.h
//  mixer
//
//  Created by Will Gallia on 26/07/2012.
//  Copyright (c) 2012. All rights reserved.
//

#pragma once

#include "ofMain.h"

class BaseHasCanvas {
public:
    
    BaseHasCanvas() {}
    void setup(int w, int h) {
        canvas.setMode(OF_PRIMITIVE_TRIANGLES);
        
        vector<ofVec3f> vertices;
        vertices.push_back(ofVec3f(0, 0, 0));
        vertices.push_back(ofVec3f(w, 0, 0));
        vertices.push_back(ofVec3f(w, h, 0));
        vertices.push_back(ofVec3f(w, h, 0));
        vertices.push_back(ofVec3f(0,h, 0));
        vertices.push_back(ofVec3f(0, 0, 0));
        canvas.addVertices(vertices);
        
        canvas.enableTextures();
        vector<ofVec2f> texcoords;
        texcoords.push_back(ofVec2f(0, 0));
        texcoords.push_back(ofVec2f(w, 0));
        texcoords.push_back(ofVec2f(w, h));
        texcoords.push_back(ofVec2f(w, h));
        texcoords.push_back(ofVec2f(0, h));
        texcoords.push_back(ofVec2f(0, 0));
        canvas.addTexCoords(texcoords);
        
        texture.allocate(w, h, GL_RGBA);
    }
protected:
    ofMesh canvas;
    ofTexture texture;
};