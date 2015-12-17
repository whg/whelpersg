//
//  ImageMask.h
//  emptyExample
//
//  Created by Will Gallia on 05/05/2015.
//
//

#pragma once

#include "ofMain.h"
#include "ofxBaseHasCanvas.h"
#include "ofxTools.h"


#define EDIT_THRESHOLD 20

class ofxImageMask : public ofPolyline, public BaseHasCanvas {
protected:
    float width, height;
    
    bool editing;
    ofVec3f *editPoint;
    ofMesh mesh;
    ofFbo fbo, maskFbo;
    ofShader shader;
    
public:
    ofxImageMask() {}
    
    void setup(float w=ofGetWidth(), float h=ofGetHeight()) {
        width = w;
        height = h;
        BaseHasCanvas::setup(w, h);

        setClosed(true);
        setEdit(true);
        
        fbo.allocate(w, h);
        maskFbo.allocate(w, h);
        
        
        /////////////////////////////////////////////////////////
        // vertex shader
        
        string vert = GLSL120(
                        varying vec2 texc;
                         
                         void main() {
                             
                             gl_Position = ftransform();
                             
                             texc = vec2(gl_TextureMatrix[0] * gl_MultiTexCoord0);
                             
                         }
                         );
        
        shader.setupShaderFromSource(GL_VERTEX_SHADER, vert);

        /////////////////////////////////////////////////////////
        // fragment shader

        string frag = GLSL120(
                         varying vec2 texc;
                         uniform sampler2DRect tex0;
                         uniform sampler2DRect mask;
                         
                         uniform float scale_factor;
                         uniform vec2 inputSize;
                         
                         void main() {
                             
                             vec4 col = texture2DRect(tex0, texc);
                             vec4 mask_c = texture2DRect(mask, texc);
                             
                             gl_FragColor = col * mask_c;
                             
                         }
                         );
        
        
        
        shader.setupShaderFromSource(GL_FRAGMENT_SHADER, frag);
        shader.linkProgram();
        
        
    }
    
    void setEdit(bool b) {
        editing = b;
        if (editing) ofRegisterMouseEvents(this);
        else ofUnregisterMouseEvents(this);
    }
    
    void begin() {
        fbo.begin();
    }
    
    void end() {
        fbo.end();
        
        maskFbo.begin();
        ofClear(0);
        draw();
        maskFbo.end();
        
        
        ofTexture &tex = fbo.getTexture();
        tex.bind();
        
        shader.begin();
        shader.setUniformTexture("tex0", tex, 0);
        shader.setUniformTexture("mask", maskFbo.getTexture(), 1);
        
        canvas.draw();

        shader.end();
        
        tex.unbind();
        
    }
    
    void draw(ofPolyRenderMode fill=OF_MESH_FILL) {
    
        if (havePointsChanged()) {
            updateMesh();
        }
    
        ofPushStyle();
        
        ofSetColor(ofColor::white);
        
        mesh.draw(fill);
        
        ofPopStyle();
        
    }
    
    void setNumberPoints(int n) {
        clear();
        
        float theta = TWO_PI / n;
        float mag = MIN(width, height) / 4;
        ofVec2f center(width/2, height/2);
        for (int i = 0; i < n; i++) {
            float x = cos(theta *  i) * mag + center.x;
            float y = sin(theta *  i) * mag + center.y;
            addVertex(x, y);
        }
        setClosed(true);
        
        updateMesh();
    }
    
    
#pragma mark -
#pragma mark Events
    
    void mouseDragged(ofMouseEventArgs &args) {
        if (editPoint) {
            editPoint->x = args.x;
            editPoint->y = args.y;
            
        }
    }
    
    void mousePressed(ofMouseEventArgs &args) {
        auto &pts = getVertices();
        ofVec3f mouse(args);
        editPoint = NULL;
        
        for (ofVec3f &p : pts) {
            if (p.distance(mouse) < EDIT_THRESHOLD) {
                editPoint = &p;
                return;
            }
        }
    }
    
    void mouseReleased(ofMouseEventArgs &args) {}
    void mouseMoved(ofMouseEventArgs &args) {}
    void mouseScrolled(ofMouseEventArgs &args) {}
    void mouseEntered(ofMouseEventArgs &args) {}
    void mouseExited(ofMouseEventArgs &args) {}
    
    
#pragma mark -
#pragma mark Saving/Loading

    void save(string filename="imagemask.xml") {
        
        ofXml xml;
        
        xml.addChild("imagemask");
        xml.setTo("imagemask");
        xml.setAttribute("version", "0.2");
        
        xml.addChild("points");
        xml.setTo("points");
        xml.setAttribute("closed", isClosed() ? "true" : "false");
        
        for (ofVec3f p : getVertices()) {
            ofXml point;
            point.addChild("point");
            point.setTo("point");
            point.addValue("x", p.x);
            point.addValue("y", p.y);
            xml.addXml(point);
        }
        xml.save(filename);
        
        ofLogNotice() << "saved image mask to " << filename;
    }
    
    void load(string filename="imagemask.xml") {
        
        ofXml xml;
        
        bool success = xml.load(filename);
        
        if (!success) {
            ofLogError() << "can't load image mask from " << filename;
            return;
        }
        
        clear();
        
        xml.setTo("imagemask");
        float version = ofToFloat(xml.getAttribute("version"));
        
        if (version >= 0.2) {
            xml.setTo("//points");
            setClosed(xml.getAttribute("closed") == "true");
        }
        
        if (version >= 0.1) {
            xml.setTo("//points");
            int n = xml.getNumChildren();
            
            for (int i = 0; i < n; i++) {
                xml.setTo("//points");
                xml.setToChild(i);
                float x = xml.getFloatValue("x");
                float y = xml.getFloatValue("y");
                addVertex(x, y);
                cout << x << ", " << y << endl;
            }
        }
        else {
            ofLogError() << "can't decode image mask file version " << version;
            return;
        }
        
        ofLogNotice() << "loaded from " << filename;
        
    }
    
    
#pragma mark -
#pragma mark Protected
protected:

    /// Create a mesh from the polyline, by triangulating from the center.
    /// There are a quite a few degenerate shapes using this method.
    void updateMesh() {
        
        ofVec3f center = getCentroid2D();
        auto &points = getVertices();
        int n = points.size();
        
        mesh.clear();
        
        for (int j = 0, i = n-1; j < n; i = j++) {
            mesh.addVertex(points[i]);
            mesh.addVertex(points[j]);
            mesh.addVertex(center);
        }
        
    }
    
    /// Find the sum of point data by summing the byte values of points vector
    ///
    bool havePointsChanged() {
    
        static long lastPointsSum = 0;
        
        auto &points = getVertices();
		if (points.size() == 0) return false;
        
        int nbytes = points.size() * sizeof(points);
        unsigned char *bytes = (unsigned char*) &points[0];
        
        long sum = 0;
        for (int i = 0; i < nbytes; i++) {
            sum+= bytes[i];
        }
        
        if (sum != lastPointsSum) {
            lastPointsSum = sum;
            ofLogVerbose() << "points changed";
            return true;
        }
        
        return false;
    }
};