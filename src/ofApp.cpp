#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(60);

	scene = bind(&ofApp::renderScene, this);

	gui.setup();

	ofDisableArbTex();
	pbr.setup(1024);
	ofxPBRFiles::getInstance()->setup("ofxPBRAssets");
	pbrHelper.setup(&pbr, ofxPBRFiles::getInstance()->getPath() + "/settings", true);
	pbrHelper.addLight(&pbrLight1, "light1");
	pbrHelper.addMaterial(&jointMaterial, "joint");
	pbrHelper.addMaterial(&material1, "material1");
	pbrHelper.addMaterial(&floorMaterial, "floor");
	pbrHelper.addCubeMap(&cubemap, "cubeMap1");

	particlesShader.load("shaders/particles");
	postEffect.load("shaders/tonemap");

	resizeFbos();

	cam.setupPerspective(false, 60, 1, 12000);

	float * position = new float[numParticles * 3];
	for (int i = 0; i < numParticles; i++) {
		position[i * 3 + 0] = ofRandom(-1, 1);
		position[i * 3 + 1] = ofRandom(-1, 1);
		position[i * 3 + 2] = ofRandom(-1, 1);
	}

	float * velocity = new float[numParticles * 3];
	for (int i = 0; i < numParticles; i++) {
		velocity[i * 3 + 0] = ofRandom(-1, 1);
		velocity[i * 3 + 1] = ofRandom(-1, 1);
		velocity[i * 3 + 2] = ofRandom(-1, 1);
	}

	float * age = new float[numParticles];
	for (int i = 0; i < numParticles; i++) {
		age[i] = 0;
	}

	float * lifetime = new float[numParticles];
	for (int i = 0; i < numParticles; i++) {
		lifetime[i] = 1;
	}

	tf.setup(numParticles, "shaders/tf.vert");
	tf.addBufferObject("inPosition", "outPosition", 3, GL_RGB32F, position);
	tf.addBufferObject("inVelocity", "outVelocity", 3, GL_RGB32F, velocity);
	tf.addBufferObject("inAge", "outAge", 1, GL_R32F, age);
	tf.addBufferObject("inLifetime", "outLifetime", 1, GL_R32F, lifetime);
	tf.generate();

	fileName = "05_05.bvh";
	bvh.load(fileName);
	bvh.setLoop(true);
	bvh.play();

	ofBoxPrimitive box;
	box.set(1, 1, 1, 1, 1, 1);
	mesh = box.getMesh();

	setupJoints();
}

//--------------------------------------------------------------
void ofApp::setupJoints() {
	joints.assign(bvh.getNumJoints(), ofVec3f());
	prevJoints.assign(bvh.getNumJoints(), ofVec3f());

	joinsBuffer.allocate();
	joinsBuffer.bind(GL_TEXTURE_BUFFER);
	joinsBuffer.setData(joints, GL_STREAM_DRAW);

	prevJointsBuffer.allocate();
	prevJointsBuffer.bind(GL_TEXTURE_BUFFER);
	prevJointsBuffer.setData(prevJoints, GL_STREAM_DRAW);

	jointsTexture.allocateAsBufferTexture(joinsBuffer, GL_RGB32F);
	prevJointsTexture.allocateAsBufferTexture(prevJointsBuffer, GL_RGB32F);

	tf.getShader()->begin();
	tf.getShader()->setUniformTexture("joints", jointsTexture, 0);
	tf.getShader()->setUniformTexture("prevJoints", prevJointsTexture, 1);
	tf.getShader()->end();
}

//--------------------------------------------------------------
void ofApp::update() {
	bvh.update();
	if (bvh.isFrameNew()) {
		for (int i = 0; i < bvh.getNumJoints(); i++) {
			prevJoints[i] = joints[i];
			joints[i] = bvh.getJoint(i)->getPosition() * 10;
		}
		joinsBuffer.updateData(0, joints);
		prevJointsBuffer.updateData(0, prevJoints);
	}

	tf.begin();
	tf.getShader()->setUniform1f("time", ofGetElapsedTimef());
	tf.getShader()->setUniform1f("timestep", timestep);
	tf.getShader()->setUniform1f("scale", scale);
	tf.getShader()->setUniform1f("maxLifetime", lifetime);
	tf.end();
}

//--------------------------------------------------------------
void ofApp::draw() {
	pbr.makeDepthMap(scene);

	firstPass.begin();
	ofClear(0);
	cam.begin();
	scene();
	cam.end();
	firstPass.end();

	postEffect.begin();
	postEffect.setUniform1f("gamma", 2.2);
	postEffect.setUniform1f("exposure", 1.0);
	firstPass.draw(0, 0);
	postEffect.end();

	gui.begin();
	{
		ImGui::Text(ofToString(ofGetFrameRate()).c_str());
		if (ImGui::Button("load bvh file")) {
			ofFileDialogResult openFileResult = ofSystemLoadDialog("Select a image");
			if (openFileResult.bSuccess) {
				bvhFile.open(openFileResult.getPath());
				if (bvhFile.getExtension() == "bvh") {
					bvh = ofxBvh();
					bvh.load(bvhFile.getAbsolutePath());
					bvh.setLoop(true);
					bvh.play();
					fileName = bvhFile.getBaseName() + "." + bvhFile.getExtension();
					setupJoints();
				}
			}
		}
		ImGui::Text(fileName.c_str());
		ImGui::DragFloat("timestep", &timestep, 0.001, 0.0, 1.0);
		ImGui::DragFloat("scale", &scale, 0.001, 0.0, 1.0);
		ImGui::DragFloat("maxLifetime", &lifetime, 1.0, 0.0, 500.0);
		ImGui::DragFloat3("box size", &boxSize[0], 1.0, 1.0, 100.0);
		pbrHelper.drawGui();
	}
	gui.end();
}

//--------------------------------------------------------------
void ofApp::resizeFbos() {
	ofFbo::Settings firstPassSettings;

	firstPassSettings.textureTarget = GL_TEXTURE_2D;
	firstPassSettings.useDepth = true;
	firstPassSettings.depthStencilAsTexture = true;
	firstPassSettings.useStencil = true;
	firstPassSettings.minFilter = GL_LINEAR;
	firstPassSettings.maxFilter = GL_LINEAR;
	firstPassSettings.wrapModeHorizontal = GL_CLAMP_TO_EDGE;
	firstPassSettings.wrapModeVertical = GL_CLAMP_TO_EDGE;

	firstPassSettings.width = ofGetWidth();
	firstPassSettings.height = ofGetHeight();
	firstPassSettings.internalformat = GL_RGBA32F;
	firstPassSettings.colorFormats.push_back(GL_RGBA32F);

	firstPass.allocate(firstPassSettings);
}

//--------------------------------------------------------------
void ofApp::renderScene() {
	ofEnableDepthTest();

	pbr.begin(&cam, &particlesShader);
	pbr.getShader()->setUniformTexture("posTex", *tf.getTexture(0), 11);
	pbr.getShader()->setUniformTexture("velTex", *tf.getTexture(1), 12);
	pbr.getShader()->setUniformTexture("ageTex", *tf.getTexture(2), 13);
	pbr.getShader()->setUniformTexture("lifetimeTex", *tf.getTexture(3), 14);
	pbr.getShader()->setUniform3f("maxSize", boxSize);
	material1.begin(&pbr);
	mesh.drawInstanced(OF_MESH_FILL, numParticles);
	material1.end();
	pbr.end();

	pbr.begin(&cam);
	jointMaterial.begin(&pbr);
	for (int i = 0; i < bvh.getNumJoints(); i++) {
		ofMatrix4x4 m = bvh.getJoint(i)->getMatrix();
		ofPushMatrix();
		float angle;
		ofVec3f axis;
		m.getRotate().getRotate(angle, axis);
		ofTranslate(bvh.getJoint(i)->getPosition() * 10);
		ofRotate(angle, axis.x, axis.y, axis.z);
		ofDrawSphere(0, 0, 0, 10);
		ofPopMatrix();
	}
	jointMaterial.end();
	floorMaterial.begin(&pbr);
	ofDrawBox(0, -25, 0, 5000, 1, 5000);
	floorMaterial.end();
	pbr.end();

	ofDisableDepthTest();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
	resizeFbos();
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
