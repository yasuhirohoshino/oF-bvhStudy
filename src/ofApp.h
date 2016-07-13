#pragma once

#include "ofMain.h"

#include "ofxImGui.h"
#include "ofxPBR.h"
#include "ofxPBRHelper.h"
#include "ofxBvh.h"
#include "ofxTF.h"

class ofApp : public ofBaseApp {

public:
	void setup();
	void setupJoints();
	void update();
	void draw();
	void resizeFbos();
	void renderScene();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	function<void()> scene;

	ofxPBR pbr;
	ofxPBRHelper pbrHelper;
	ofxImGui gui;
	ofxPBRCubeMap cubemap;
	ofxPBRMaterial material1, jointMaterial, floorMaterial;
	ofxPBRLight pbrLight1, pbrLight2;

	ofFbo firstPass;

	ofShader renderShader, particlesShader, postEffect;
	ofEasyCam cam;
	ofxBvh bvh;
	int prevFrame = 0;

	ofxTF tf;

	const int numParticles = 50000;

	ofVboMesh mesh;
	vector<ofVec3f> joints, prevJoints;
	ofFile bvhFile;
	string fileName;

	float timestep = 0.1;
	float scale = 0.05;
	float lifetime = 50.0;
	ofVec3f boxSize = ofVec3f(1.0, 1.0, 25.0);

	ofBufferObject joinsBuffer, prevJointsBuffer;
	ofTexture jointsTexture, prevJointsTexture;
};
