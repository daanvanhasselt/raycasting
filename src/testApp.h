#pragma once

#include "ofMain.h"
#include "ofxUI.h"

class testApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    void exit();
    
    void vertex(float x, float y, float z);
    void drawQuads(float x, float y, float z);
    
    ofShader shader;
    unsigned char *tex3d;
    GLuint tex3dHandle;
    
    ofFbo backfaceRender;
    ofFbo raycastRender;
    
    int texWidth, texHeight, texDepth;
    int renderWidth, renderHeight;
    
    int zTexOffset;
    
    ofxUICanvas *gui;
    void guiEvent(ofxUIEventArgs &e);
    float quality;
    float density;
    float threshold;
    bool animate;
    bool antialias;
};
