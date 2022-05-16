
//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//  Octree Test - startup scene
// 
//
//  Student Name:   < Your Name goes Here >
//  Date: <date of last version>


#include "ofApp.h"
#include "Utilities/Util.h"


//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){
	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;
	started = false;
	gameover = false;
//	ofSetWindowShape(1024, 768);
	mainCam.setDistance(50);
	mainCam.setNearClip(.1);
	mainCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	mainCam.setPosition(mainCam.getPosition() + glm::vec3(0, 100, 0));
	ofSetVerticalSync(true);
	ofEnableSmoothing();
	ofEnableDepthTest();

	//Setup keys
	keysPressed["space"] = false;
	keysPressed["shift"] = false;
	keysPressed["up"] = false;
	keysPressed["down"] = false;
	keysPressed["left"] = false;
	keysPressed["right"] = false;

	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	//Set Background
	background.load("assets/jeremy-perkins-uhjiu8FjnsQ-unsplash.jpg");

	//Load in the terrain Model
	mars.loadModel("geo/moon-houdini.obj");
	mars.setScaleNormalization(false);

	// create sliders for testing
	//
	gui.setup();
	gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	bHide = false;

	//  Create Octree for testing.
	//
	octree.create(mars.getMesh(0), 20);
	cout << "Number of Verts: " << mars.getMesh(0).getNumVertices() << endl;

	//Load in the Lander
	lander = Lander();
	lander.load("geo/lander.obj");
	lander.model.setScaleNormalization(false);
	lander.position = glm::vec3(1, 100, 0);
	bLanderLoaded = true;
	for (int i = 0; i < lander.model.getMeshCount(); i++) {
		bboxList.push_back(Octree::meshBounds(lander.model.getMesh(i)));
	}

	GravityForce *f = new GravityForce(glm::vec3(0, -1.625, 0));
	f->applyOnce = false;
	lander.addForce(f);
	currentAltitude = getAltitude();
	last_check = ofGetElapsedTimeMillis();

	// Load Fonts
	font.load("fonts/OpenSans-Semibold.ttf", 50);

	//Add Additional Cameras
	sideCam.setPosition(-169.841, -1.1000, 176.855);
	sideCam.lookAt(lander.position);
	landerCam.setNearClip(.1);
	glm::vec3 landerCamPosition = lander.position - glm::vec3(0, 10, 0);
	landerCam.setPosition(landerCamPosition);
	landerCam.lookAt(landerCamPosition - glm::vec3(0, 1, 0), glm::vec3(0, 1, 0)); //Look right below camera point

	theCam = &landerCam;

	//Create Landing Zone
	landingZone = glm::vec3(-169.841, -1.0000, 176.855);
	glm::vec3 landingMin = landingZone + glm::vec3(-10);
	glm::vec3 landingMax = landingZone + glm::vec3(10);
	landingZoneBox = Box(Vector3(landingMin.x, landingMin.y, landingMin.z), Vector3(landingMax.x, landingMax.y, landingMax.z));

	//Set the emitter
	emitter.setPosition(lander.position);
	emitter.setEmitterType(DirectionalEmitter);
	emitter.sys->addForce(new DownwardForce(10));
	emitter.setVelocity(ofVec3f(0, 0, 0));
	emitter.setOneShot(true);
	emitter.setGroupSize(50);
	emitter.setRandomLife(true);
	emitter.setLifespanRange(ofVec2f(0.5, 1));

	//Load Sounds
	if(backgroundSound.load("sounds/Hardmoon_-_Deep_space.mp3")) {
	}
	else {
	cout << "Can't open sound file" << endl;
	ofExit();
	}
	if (thrustSound.load("sounds/ScatterNoise1.mp3")) {
	}
	else {
		cout << "Can't open sound file" << endl;
		ofExit();
	}
	if (explodeSound.load("sounds/explosion.wav")) {
	}
	else {
		cout << "Can't open sound file" << endl;
		ofExit();
	}
	thrustSound.setLoop(true);
	explodeSound.setVolume(0.50);
	backgroundSound.setVolume(0.50);
	backgroundSound.setLoop(true);
	backgroundSound.play();

	// texture loading
	//
	ofDisableArbTex();     // disable rectangular textures

	// load textures
	//
	if (!ofLoadImage(particleTex, "images/dot.png")) {
		cout << "Particle Texture File: images/dot.png not found" << endl;
		ofExit();
	}

	// load the shader
	//
	#ifdef TARGET_OPENGLES
		shader.load("shaders_gles/shader");
	#else
		shader.load("shaders/shader");
	#endif
}
 
// load vertex buffer in preparation for rendering
//
void ofApp::loadVbo() {
	if (emitter.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < emitter.sys->particles.size(); i++) {
		points.push_back(emitter.sys->particles[i].position);
		sizes.push_back(ofVec3f(radius));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	vbo.clear();
	vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}


//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
	if (started && !gameover) {
		//Update Keys
		if (keysPressed["up"]) {
			lander.addForce(new ComputeForward(10));
		}
		if (keysPressed["down"]) {
			lander.addForce(new ComputeBackward(10));
		}
		if (keysPressed["left"]) {
			lander.addForce(new ComputeLeft(35));
		}
		if (keysPressed["right"]) {
			lander.addForce(new ComputeRight(35));
		}
		if (keysPressed["space"]) {
			lander.addForce(new ComputeUp(glm::vec3(0, 10, 0)));
			emitter.sys->reset();
			emitter.start();
		}
		if (keysPressed["shift"]) {
			lander.addForce(new ComputeDown(glm::vec3(0, 10, 0)));
		}

		if (keysPressed["up"] || keysPressed["down"] || keysPressed["left"] || keysPressed["right"] || keysPressed["shift"] || keysPressed["space"]) {
			playThrust = true;
		}
		else {
			playThrust = false;
		}

		if (playThrust && !thrustPlaying) {
			thrustSound.play();
			thrustPlaying = true;
		} 
		if (!playThrust) {
			thrustSound.stop();
			thrustPlaying = false;
		}

		//Update Everything Else
		lander.update();
		glm::vec3 landerCamPosition = lander.position + glm::vec3(0, 10, 0);
		landerCam.setPosition(landerCamPosition);
		landerCam.lookAt(landerCamPosition + glm::vec3(0, -10, 0)); //Look right below camera point
		sideCam.setPosition(landingZone + glm::vec3(0, 2, 0));
		sideCam.lookAt(lander.position);
		emitter.setPosition(lander.position);
		emitter.update();
	}
	if (gameover) {
		lander.update();
		emitter.update();
	}
	bool collision = checkCollision();
	if (collision) {
		resolveCollision();
	}
	//check if in GoalZone
	if (!landingZoneBox.inside(Vector3(lander.position.x, lander.position.y, lander.position.z))) {
		landerBoxColor = ofColor::green;
	}
	else {
		landerBoxColor = ofColor::blue;
	}
	if (ofGetElapsedTimeMillis() - last_check > 1000) {
		currentAltitude = getAltitude();
		last_check = ofGetElapsedTimeMillis();
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	if (!started) {
		ofSetBackgroundColor(ofColor::black);
		ofPushMatrix();
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2 - 80);
		string text = "Press Enter to Start";
		float fontWidth = font.stringWidth(text);
		ofSetColor(ofColor::orange);
		font.drawString(text, -fontWidth / 2, 0);
		ofPopMatrix();

		ofPushMatrix();
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		text = "Land The Lunar Lander!";
		fontWidth = font.stringWidth(text);
		ofSetColor(ofColor::orange);
		font.drawString(text, -fontWidth / 2, 0);
		ofPopMatrix();

		ofPushMatrix();
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2 + 80);
		text = "Landing Zone Marked in Blue";
		fontWidth = font.stringWidth(text);
		ofSetColor(ofColor::orange);
		font.drawString(text, -fontWidth / 2, 0);
		ofPopMatrix();

		ofPushMatrix();
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2 + 160);
		text = "Careful! Don't Fall too fast!";
		fontWidth = font.stringWidth(text);
		ofSetColor(ofColor::orange);
		font.drawString(text, -fontWidth / 2, 0);
		ofPopMatrix();
	}
	if (gameover) {
		loadVbo();
		glDepthMask(false);
		ofSetColor(ofColor::white);
		background.draw(0, 0);

		if (!bHide) gui.draw();
		glDepthMask(true);

		theCam->begin();

		//Draw Particles
		shader.begin();
		ofEnableBlendMode(OF_BLENDMODE_ADD); // this makes everything look glowy :)
		ofEnablePointSprites();
		ofSetColor(ofColor::red);
		particleTex.bind();
		vbo.draw(GL_POINTS, 0, (int)emitter.sys->particles.size());
		vbo.draw(GL_POINTS, 0, (int)emitter.sys->particles.size());
		particleTex.unbind();
		shader.end();
		ofDisablePointSprites();
		ofDisableBlendMode();
		ofEnableAlphaBlending();

		ofPushMatrix();
		if (bWireframe) {                    // wireframe mode  (include axis)
			ofDisableLighting();
			ofSetColor(ofColor::slateGray);
			mars.drawWireframe();
			if (bLanderLoaded) {
				lander.model.drawWireframe();
				if (!bTerrainSelected) drawAxis(lander.model.getPosition());
			}
			if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
		}
		else {
			ofEnableLighting();              // shaded mode
			mars.drawFaces();
			ofMesh mesh;
			if (bLanderLoaded) {
				lander.draw();
				if (!bTerrainSelected) drawAxis(lander.model.getPosition());
				if (bDisplayBBoxes) {
					ofNoFill();
					ofSetColor(ofColor::white);
					for (int i = 0; i < lander.model.getNumMeshes(); i++) {
						ofPushMatrix();
						ofMultMatrix(lander.model.getModelMatrix());
						ofRotate(-90, 1, 0, 0);
						Octree::drawBox(bboxList[i]);
						ofPopMatrix();
					}
				}

				if (bLanderSelected) {

					ofVec3f min = lander.model.getSceneMin() + lander.position;
					ofVec3f max = lander.model.getSceneMax() + lander.position;

					Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
					ofSetColor(ofColor::white);
					Octree::drawBox(bounds);

					// draw colliding boxes
					//
					ofSetColor(ofColor::lightBlue);
					for (int i = 0; i < colBoxList.size(); i++) {
						Octree::drawBox(colBoxList[i]);
					}
				}
			}
		}

		//Draw Landing Zone
		ofSetColor(landerBoxColor);
		octree.drawBox(landingZoneBox);
		ofPopMatrix();
		theCam->end();


		//Draw Game Over Text
		ofSetColor(ofColor::orange);
		ofPushMatrix();
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2 - 80);
		int difference = difftime(time_finish, time_start);
		int minutes = difference / 60;
		int seconds = difference % 60;
		string text;
		if (victory) {
			text = "You Win!";
		}
		else {
			text = "Game Over";
		}
		float fontWidth = font.stringWidth(text);
		font.drawString(text, -fontWidth / 2, 0);
		ofPopMatrix();
		char buffer[100];
		if (minutes > 0) {
			snprintf(buffer, 100, "You Landed in %d Minutes and %02d Seconds", minutes, seconds);
		}
		else {
			snprintf(buffer, 100, "You Landed in %d Seconds", seconds);
		}
		ofSetColor(ofColor::orange);
		ofPushMatrix();
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		fontWidth = font.stringWidth(buffer);
		font.drawString(buffer, -fontWidth / 2, 0);
		ofPopMatrix();
		ofPushMatrix();
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2 + 160);
		text = "Press Enter To Restart";
		fontWidth = font.stringWidth(text);
		font.drawString(text, -fontWidth / 2, 0);
		ofPopMatrix();
	}
	if (started && !gameover) {
		loadVbo();
		glDepthMask(false);
		ofSetColor(ofColor::white);
		background.draw(0, 0);

		if (!bHide) gui.draw();
		glDepthMask(true);

		theCam->begin();

		//Draw Particles
		shader.begin();
		ofEnableBlendMode(OF_BLENDMODE_ADD); // this makes everything look glowy :)
		ofEnablePointSprites();
		ofSetColor(ofColor::red);
		particleTex.bind();
		vbo.draw(GL_POINTS, 0, (int)emitter.sys->particles.size());
		vbo.draw(GL_POINTS, 0, (int)emitter.sys->particles.size());
		particleTex.unbind();
		shader.end();
		ofDisablePointSprites();
		ofDisableBlendMode();
		ofEnableAlphaBlending();

		ofPushMatrix();
		if (bWireframe) {                    // wireframe mode  (include axis)
			ofDisableLighting();
			ofSetColor(ofColor::slateGray);
			mars.drawWireframe();
			if (bLanderLoaded) {
				lander.model.drawWireframe();
				if (!bTerrainSelected) drawAxis(lander.model.getPosition());
			}
			if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
		}
		else {
			ofEnableLighting();              // shaded mode
			mars.drawFaces();
			ofMesh mesh;
			if (bLanderLoaded) {
				lander.draw();
				if (!bTerrainSelected) drawAxis(lander.model.getPosition());
				if (bDisplayBBoxes) {
					ofNoFill();
					ofSetColor(ofColor::white);
					for (int i = 0; i < lander.model.getNumMeshes(); i++) {
						ofPushMatrix();
						ofMultMatrix(lander.model.getModelMatrix());
						ofRotate(-90, 1, 0, 0);
						Octree::drawBox(bboxList[i]);
						ofPopMatrix();
					}
				}

				if (bLanderSelected) {

					ofVec3f min = lander.model.getSceneMin() + lander.position;
					ofVec3f max = lander.model.getSceneMax() + lander.position;

					Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
					ofSetColor(ofColor::white);
					Octree::drawBox(bounds);

					// draw colliding boxes
					//
					ofSetColor(ofColor::lightBlue);
					for (int i = 0; i < colBoxList.size(); i++) {
						Octree::drawBox(colBoxList[i]);
					}
				}
			}
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));



		if (bDisplayPoints) {                // display points as an option    
			glPointSize(3);
			ofSetColor(ofColor::green);
			mars.drawVertices();
		}


		// recursively draw octree
		//
		ofDisableLighting();
		int level = 0;
		//	ofNoFill();

		if (bDisplayLeafNodes) {
			octree.drawLeafNodes(octree.root);
			cout << "num leaf: " << octree.numLeaf << endl;
		}
		else if (bDisplayOctree) {
			ofNoFill();
			ofSetColor(ofColor::white);
			octree.draw(numLevels, 0);
		}

		// if point selected, draw a sphere
		//
		if (pointSelected) {
			ofVec3f p = octree.mesh.getVertex(selectedNode.points[0]);
			ofVec3f d = p - mainCam.getPosition();
			ofSetColor(ofColor::lightGreen);
			ofDrawSphere(p, .02 * d.length());
		}

		//Draw Landing Zone
		ofSetColor(ofColor::green);
		octree.drawBox(landingZoneBox);
		ofPopMatrix();
		theCam->end();

		//Draw the time
		ofSetColor(ofColor::white);
		ofPushMatrix();
		ofTranslate(ofGetWidth() / 2, ofGetHeight() - 0.92 * ofGetHeight());
		time(&time_finish);
		int difference = difftime(time_finish, time_start);
		int minutes = difference / 60;
		int seconds = difference % 60;
		char buffer[100];
		if (minutes > 0) {
			snprintf(buffer, 100, "%d : %02d", minutes, seconds);
		}
		else {
			snprintf(buffer, 100, "%d", seconds);
		}
		float fontWidth = font.stringWidth(buffer);
		font.drawString(buffer, -fontWidth / 2, 0);
		ofPopMatrix();

		//Draw FPS
		int fps = ofGetFrameRate();
		ofSetColor(ofColor::white);
		ofPushMatrix();
		ofTranslate(ofGetWidth() - 0.15 * ofGetWidth(), ofGetHeight() - 0.92 * ofGetHeight());
		string text = "FPS " + to_string(fps);
		fontWidth = font.stringWidth(text);
		font.drawString(text, -fontWidth / 2, 0);
		ofPopMatrix();

		//Draw Down Velocity
		ofSetColor(ofColor::white);
		ofPushMatrix();
		ofTranslate(ofGetWidth() - 0.15 * ofGetWidth(), ofGetHeight() - 0.92 * ofGetHeight() + 80);
		text = "Velocity " + to_string(lander.velocity.y);
		fontWidth = font.stringWidth(text);
		font.drawString(text, -fontWidth / 2, 0);
		ofPopMatrix();

		//Draw Altitude
		//Draw Altitude
		ofSetColor(ofColor::white);
		ofPushMatrix();
		ofTranslate(ofGetWidth() - 0.15 * ofGetWidth(), ofGetHeight() - 0.92 * ofGetHeight() + 160);
		text = "Altitude " + to_string(currentAltitude);
		fontWidth = font.stringWidth(text);
		font.drawString(text, -fontWidth / 2, 0);
		ofPopMatrix();
	}
	
}


// 
// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {
	if (started && !gameover) {
		switch (key) {
		case 'a':
			break;
		case 'B':
			break;
		case 'b':
			bDisplayBBoxes = !bDisplayBBoxes;
			break;
		case 'c':
			if (mainCam.getMouseInputEnabled()) mainCam.disableMouseInput();
			else mainCam.enableMouseInput();
			break;
		case 'F':
			break;
		case 'f':
			ofToggleFullscreen();
			break;
		case 'h':
			break;
		case 'l':
			bDisplayLeafNodes = !bDisplayLeafNodes;
			break;
		case 'o':
			bDisplayOctree = !bDisplayOctree;
			break;
		case 'r':
			mainCam.reset();
			break;
		case 's':
			savePicture();
			break;
		case 't':
			setCameraTarget();
			break;
		case 'u':
			break;
		case 'v':
			togglePointsDisplay();
			break;
		case 'w':
			toggleWireframeMode();
			break;
		case OF_KEY_ALT:
			mainCam.enableMouseInput();
			bAltKeyDown = true;
			break;
		case OF_KEY_CONTROL:
			bCtrlKeyDown = true;
			break;
		case OF_KEY_DEL:
			break;
		case ' ':
			keysPressed["space"] = true;
			break;
		case OF_KEY_SHIFT:
			keysPressed["shift"] = true;
			break;
		case OF_KEY_UP:
			keysPressed["up"] = true;
			break;
		case OF_KEY_DOWN:
			keysPressed["down"] = true;
			break;
		case OF_KEY_LEFT:
			keysPressed["left"] = true;
			break;
		case OF_KEY_RIGHT:
			keysPressed["right"] = true;
			break;
		case OF_KEY_F1:
			theCam = &sideCam;
			break;
		case OF_KEY_F2:
			theCam = &landerCam;
			break;
		case OF_KEY_F3:
			mainCam.lookAt(lander.position);
			theCam = &mainCam;
			break;
		default:
			break;
		}
	}
	if (!started) {
		switch(key) {

		case OF_KEY_RETURN:
			started = !started;
			time(&time_start);
			break;
		default:
			break;

		}
	}
	if (gameover) {
		switch (key) {
		case OF_KEY_RETURN:
			gameover = false;
			started = false;
			reset();
			break;
		default:
			break;

		}
	}
	
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {

	switch (key) {
	
	case OF_KEY_ALT:
		mainCam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case ' ':
		keysPressed["space"] = false;
		break;
	case OF_KEY_SHIFT:
		keysPressed["shift"] = false;
		break;
	case OF_KEY_UP:
		keysPressed["up"] = false;
		break;
	case OF_KEY_DOWN:
		keysPressed["down"] = false;
		break;
	case OF_KEY_LEFT:
		keysPressed["left"] = false;
		break;
	case OF_KEY_RIGHT:
		keysPressed["right"] = false;
		break;
	default:
		break;

	}
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

	
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (mainCam.getMouseInputEnabled()) return;

	// if rover is loaded, test for selection
	//
	if (bLanderLoaded) {
		glm::vec3 origin = mainCam.getPosition();
		glm::vec3 mouseWorld = mainCam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		ofVec3f min = lander.model.getSceneMin() + lander.position;
		ofVec3f max = lander.model.getSceneMax() + lander.position;

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			bLanderSelected = true;
			mouseDownPos = getMousePointOnPlane(lander.position, mainCam.getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
		else {
			bLanderSelected = false;
		}
	}
	else {
		//ofVec3f p;
		//raySelectWithOctree(p);
	}
}

bool ofApp::raySelectWithOctree(ofVec3f &pointRet) {
	ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = mainCam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - mainCam.getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));
	cout << "Starting Octree Intersection" << endl;
	float start = ofGetElapsedTimeMicros();
	pointSelected = octree.intersect(ray, octree.root, selectedNode);
	float finished = (ofGetElapsedTimeMicros() - start);
	cout << "Finished Intersection" << endl;
	cout << "Intersection took " << finished << " microseconds to complete" << endl;
	if (pointSelected) {
		pointRet = octree.mesh.getVertex(selectedNode.points[0]);
		cout << "Point Selected:" << endl;
		cout << "X: " << mars.getMesh(0).getVertex(selectedNode.points[0]).x << endl;
		cout << "Y: " << mars.getMesh(0).getVertex(selectedNode.points[0]).y << endl;
		cout << "Z: " << mars.getMesh(0).getVertex(selectedNode.points[0]).z << endl;
	}
	return pointSelected;
}




//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (mainCam.getMouseInputEnabled()) return;

	if (bInDrag) {

		glm::vec3 landerPos = lander.position;

		glm::vec3 mousePos = getMousePointOnPlane(landerPos, mainCam.getZAxis());
		glm::vec3 delta = mousePos - mouseLastPos;
	
		landerPos += delta;
		lander.position = landerPos;
		mouseLastPos = mousePos;

		ofVec3f min = lander.model.getSceneMin() + lander.position;
		ofVec3f max = lander.model.getSceneMax() + lander.position;

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		colBoxList.clear();
		octree.intersect(bounds, octree.root, colBoxList);


	}
	else {
		ofVec3f p;
		//raySelectWithOctree(p);
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent2(ofDragInfo dragInfo) {

	ofVec3f point;
	mouseIntersectPlane(ofVec3f(0, 0, 0), mainCam.getZAxis(), point);
	if (lander.model.loadModel(dragInfo.files[0])) {
		lander.model.setScaleNormalization(false);
//		lander.setScale(.1, .1, .1);
	//	lander.setPosition(point.x, point.y, point.z);
		lander.model.setPosition(1, 1, 0);

		bLanderLoaded = true;
		for (int i = 0; i < lander.model.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.model.getMesh(i)));
		}

		cout << "Mesh Count: " << lander.model.getMeshCount() << endl;
	}
	else cout << "Error: Can't load model" << dragInfo.files[0] << endl;
}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point) {
	ofVec2f mouse(mouseX, mouseY);
	ofVec3f rayPoint = mainCam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	ofVec3f rayDir = rayPoint - mainCam.getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (lander.model.loadModel(dragInfo.files[0])) {
		bLanderLoaded = true;
		lander.model.setScaleNormalization(false);
		lander.model.setPosition(0, 0, 0);
		cout << "number of meshes: " << lander.model.getNumMeshes() << endl;
		bboxList.clear();
		for (int i = 0; i < lander.model.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.model.getMesh(i)));
		}

		//		lander.setRotation(1, 180, 1, 0, 0);

				// We want to drag and drop a 3D object in space so that the model appears 
				// under the mouse pointer where you drop it !
				//
				// Our strategy: intersect a plane parallel to the camera plane where the mouse drops the model
				// once we find the point of intersection, we can position the lander/lander
				// at that location.
				//

				// Setup our rays
				//
		glm::vec3 origin = mainCam.getPosition();
		glm::vec3 camAxis = mainCam.getZAxis();
		glm::vec3 mouseWorld = mainCam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
		float distance;

		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
		if (hit) {
			// find the point of intersection on the plane using the distance 
			// We use the parameteric line or vector representation of a line to compute
			//
			// p' = p + s * dir;
			//
			glm::vec3 intersectPoint = origin + distance * mouseDir;

			// Now position the lander's origin at that intersection point
			//
			glm::vec3 min = lander.model.getSceneMin();
			glm::vec3 max = lander.model.getSceneMax();
			float offset = (max.y - min.y) / 2.0;
			lander.model.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);

			// set up bounding box for lander while we are at it
			//
			landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
	}


}

//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	glm::vec3 origin = mainCam.getPosition();
	glm::vec3 camAxis = mainCam.getZAxis();
	glm::vec3 mouseWorld = mainCam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;

		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}

/*
* Get the altitude using a ray
*/
float ofApp::getAltitude() {
	Ray r = Ray(Vector3(lander.position.x, lander.position.y, lander.position.z), Vector3(0, -1, 0));
	octree.intersect(r, octree.root, selectedNode);
	glm::vec3 point = mars.getMesh(0).getVertex(selectedNode.points[0]);
	return glm::length(point - lander.position);
}

/*
* Check for collision using the octree
*/
bool ofApp::checkCollision() {
	ofVec3f min = lander.model.getSceneMin() + lander.position;
	ofVec3f max = lander.model.getSceneMax() + lander.position;

	Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

	nodeList.clear();
	octree.intersect(bounds, octree.root, nodeList);
	if (nodeList.size() > 0) {
		for (int i = 0; i < nodeList.size(); i++) {
			glm::vec3 point = octree.mesh.getVertex(nodeList[i].points[0]);
			if (bounds.inside(Vector3(point.x, point.y, point.z))) {
				return true;
			}
		}
	}
	return false;
}

/*
* Collision Resolution Function
*/
void ofApp::resolveCollision() {
	ofVec3f min = lander.model.getSceneMin() + lander.position;
	ofVec3f max = lander.model.getSceneMax() + lander.position;

	Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
	//Set lander back above ground
	float highestY = -100000.0f;
	int highestIndex = 0;
	for (int i = 0; i < nodeList.size(); i ++ ) {
		glm::vec3 point = octree.mesh.getVertex(nodeList[i].points[0]);
		if (bounds.inside(Vector3(point.x, point.y, point.z))) {
			if (point.y > highestY) {
				highestY = point.y;
				highestIndex = i;
			}
		}
	}
	float downVelocity = -lander.velocity.y;
	//Move lander Up a bit
	lander.position = glm::vec3(lander.position.x, lander.position.y + 0.5, lander.position.z);
	lander.removeForces();
	glm::vec3 normal = mars.getMesh(0).getNormal(highestIndex);
	glm::vec3 impulse = (1 + 0.85) * ((-lander.velocity).dot(normal) * normal);
	lander.addForce(new ImpulseForce(impulse));
	if (downVelocity < 2 && landingZoneBox.inside(Vector3(lander.position.x, lander.position.y, lander.position.z))) {
		victory = true;
		gameover = true;
	}
	else {
		if (!gameover) {
			gameover = true;
			emitter.setEmitterType(RadialEmitter);
			emitter.sys->removeForces();
			ImpulseRadialForce *impulse = new ImpulseRadialForce(500);
			impulse->setHeight(1);
			emitter.sys->addForce(impulse);
			emitter.setVelocity(ofVec3f(0, 0, 0));
			emitter.setRandomLife(true);
			emitter.setLifespanRange(ofVec2f(1, 2));
			emitter.setGroupSize(10000);
			emitter.sys->reset();
			emitter.start();
			explodeSound.play();
		}
	}
}

/*
* Reset the game
*/
void ofApp::reset() {
	//Set the Lander
	lander.position = glm::vec3(1, 100, 0);
	lander.removeForces();
	GravityForce* f = new GravityForce(glm::vec3(0, -1.625, 0));
	f->applyOnce = false;
	lander.addForce(f);

	//Set the adjustable camera
	mainCam.setPosition(0, 100, 0);
	//Set the emitter
	emitter.sys->removeForces();
	emitter.setPosition(lander.position);
	emitter.setEmitterType(DirectionalEmitter);
	emitter.sys->addForce(new DownwardForce(10));
	emitter.setVelocity(ofVec3f(0, 0, 0));
	emitter.setOneShot(true);
	emitter.setGroupSize(50);
	emitter.setRandomLife(true);
	emitter.setLifespanRange(ofVec2f(0.5, 1));
}
