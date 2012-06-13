#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
//    ofSetVerticalSync(true);
    
    texWidth        = 256;
    texHeight       = 256;
    texDepth        = 256;
    
    renderWidth     = texWidth;
    renderHeight    = texHeight;
    
    zTexOffset      = 0;
    animate         = false;
    
    shader.load("shaders/raycast");
    backfaceRender.allocate(renderWidth, renderHeight, GL_RGBA);
    raycastRender.allocate(renderWidth, renderHeight, GL_RGBA);
    
    tex3d = new unsigned char[texWidth * texHeight * texDepth * 4];
    for(int x=0; x<texWidth; x++) {
        for(int y=0; y<texHeight; y++) {
            for(int z=0;z<texDepth;z++) {
                int index = (x+y*texWidth+z*texWidth*texHeight)*4;
                tex3d[index] = ofNoise(x*0.02,y*0.02,z*0.02)*255;
                tex3d[index+1] = ofNoise(y*0.02,z*0.02,x*0.02)*255;
                tex3d[index+2] = ofNoise(z*0.02,y*0.02,x*0.02)*255;
                tex3d[index+3] = ofNoise(x*0.02,z*0.02,y*0.02)*196;
            }
        }
    }
    
    //setup 3d texture
    glGenTextures(1, &tex3dHandle);
    glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, tex3dHandle);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    
    // upload volume to graphics card
    glTexImage3D(GL_TEXTURE_3D,         // target
                 0,                     // level
                 GL_RGBA,               // internal format
                 texWidth,              // width
                 texHeight,             // height
                 texDepth,              // depth
                 0,                     // border
                 GL_RGBA,               // format
                 GL_UNSIGNED_BYTE,      // type
                 tex3d);                // data

}

//--------------------------------------------------------------
void testApp::update(){
    // cycles through the original volume and uploads one slice at a time to the GPU
    // tests how fast this function is on your GFX card, press any key to turn on animation.
    if(animate) {
        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, zTexOffset, texWidth,texHeight, 1, GL_RGBA, GL_UNSIGNED_BYTE, tex3d + (zTexOffset * texWidth * texHeight * 4));
        zTexOffset = (zTexOffset + 1) % texDepth;
    }
    
    smoothedFPS = ofGetFrameRate() * 0.01 + smoothedFPS * 0.99;
    ofSetWindowTitle(ofToString(smoothedFPS));
}

//--------------------------------------------------------------
void testApp::draw(){

    float size = renderHeight;
    float angle = sin(ofGetFrameNum()*0.001)*360.0f;
    
    // render the backface
    backfaceRender.begin();
    ofClear(0);
        glPushMatrix();
        glTranslatef(renderWidth/2, renderHeight/2, -100);
        glRotatef(angle,0.3,1,0.6);
        glTranslatef(-size/2,-size/2, -size/2);
        glScalef(size,size,size);
        glActiveTexture(GL_TEXTURE0);
        
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        drawQuads(0.99,0.99,0.99);
        glDisable(GL_CULL_FACE);
        glPopMatrix();
    backfaceRender.end();
    
//    backfaceRender.draw(0, 0);
    
    // cast rays
    raycastRender.begin();
    ofClear(0);
        shader.begin();

            // bind textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(backfaceRender.getTextureReference().texData.textureTarget, (GLuint)backfaceRender.getTextureReference().texData.textureID);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_3D, tex3dHandle);
            
            // pass variables to the shader
            shader.setUniform1i("backface", 0);         // bg tex reference
            shader.setUniform1i("volume_tex", 1);       // volume tex reference
            shader.setUniform3f("vol_d", (float)texWidth, (float)texHeight, float(texDepth));   // dimensions of volume tex
            shader.setUniform2f("bg_d", (float)renderWidth, (float)renderHeight);               // dimensions of bg tex
            shader.setUniform1f("zoffset", (float)zTexOffset);          // used for animation so that we dont have to upload the entire volume every time    
            shader.setUniform1f("quality", 0.5f); // 0 ... 1
            shader.setUniform1f("density", 0.01f); // 0 ... 1
            shader.setUniform1f("threshold", 1.0f/255.0f);//(float)mouseX/(float)ofGetWidth());
            
            glPushMatrix();
            glTranslatef(renderWidth/2, renderHeight/2, -100);
            glRotatef(angle,0.3,1,0.6);
            glTranslatef(-size/2,-size/2, -size/2);
            glScalef(size,size,size);
            glActiveTexture(GL_TEXTURE0);
            
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            drawQuads(0.99,0.99,0.99);
            glDisable(GL_CULL_FACE);
            glPopMatrix();

        shader.end();    
    raycastRender.end();
    
    ofSetColor(255);
    raycastRender.draw(0, 0, 512, 512);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    animate = !animate;
}

//--------------------------------------------------------------
void testApp::vertex(float x, float y, float z)
{
	glColor4f(x,y,z,1);
	glMultiTexCoord3f(GL_TEXTURE0, x, y, z);
	glVertex3f(x,y,z);
}

//--------------------------------------------------------------
// this method is used to draw the front and backside of the volume
//--------------------------------------------------------------
void testApp::drawQuads(float x, float y, float z)
{
    
	glBegin(GL_QUADS);
	/* Back side */
	glNormal3f(0.0, 0.0, -1.0);
	vertex(0.0, 0.0, 0.0);
	vertex(0.0, y, 0.0);
	vertex(x, y, 0.0);
	vertex(x, 0.0, 0.0);
    
	/* Front side */
	glNormal3f(0.0, 0.0, 1.0);
	vertex(0.0, 0.0, z);
	vertex(x, 0.0, z);
	vertex(x, y, z);
	vertex(0.0, y, z);
    
	/* Top side */
	glNormal3f(0.0, 1.0, 0.0);
	vertex(0.0, y, 0.0);
	vertex(0.0, y, z);
    vertex(x, y, z);
	vertex(x, y, 0.0);
    
	/* Bottom side */
	glNormal3f(0.0, -1.0, 0.0);
	vertex(0.0, 0.0, 0.0);
	vertex(x, 0.0, 0.0);
	vertex(x, 0.0, z);
	vertex(0.0, 0.0, z);
    
	/* Left side */
	glNormal3f(-1.0, 0.0, 0.0);
	vertex(0.0, 0.0, 0.0);
	vertex(0.0, 0.0, z);
	vertex(0.0, y, z);
	vertex(0.0, y, 0.0);
    
	/* Right side */
	glNormal3f(1.0, 0.0, 0.0);
	vertex(x, 0.0, 0.0);
	vertex(x, y, 0.0);
	vertex(x, y, z);
	vertex(x, 0.0, z);
	glEnd();
    
}