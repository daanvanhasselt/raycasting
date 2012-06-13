#pragma once

#include "ofMain.h"

class testApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    
    void vertex(float x, float y, float z);
    void drawQuads(float x, float y, float z);
    
    ofShader shader;
    unsigned char *tex3d;
    GLuint tex3dHandle;
    
    ofFbo backfaceRender;
    ofFbo raycastRender;
    
    int texWidth, texHeight, texDepth;
    int renderWidth, renderHeight;
    
    bool animate;
    int zTexOffset;
    
    float smoothedFPS;
};
