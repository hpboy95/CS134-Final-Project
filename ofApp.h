#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "Octree/Octree.h"
#include <glm/gtx/intersect.hpp>
#include "ParticleSystems/Lander.h"
#include "ParticleSystems/ParticleEmitter.h"


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent2(ofDragInfo dragInfo);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void reset();
		void savePicture();
		void toggleWireframeMode();
		void togglePointsDisplay();
		void toggleSelectTerrain();
		void setCameraTarget();
		bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point);
		bool raySelectWithOctree(ofVec3f& pointRet);
		bool checkCollision();
		void resolveCollision();
		void loadVbo();
		glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 p , glm::vec3 n);

		ofEasyCam mainCam;
		ofCamera landerCam;
		ofCamera sideCam;
		ofCamera* theCam;
		ofxAssimpModelLoader mars;
		Lander lander;
		ofColor landerBoxColor = ofColor::blue;
		ofLight light;
		Box boundingBox, landerBounds,landingZoneBox;
		Box testBox;
		vector<Box> colBoxList;
		bool bLanderSelected = false;
		Octree octree;
		TreeNode selectedNode;
		glm::vec3 mouseDownPos, mouseLastPos;
		bool bInDrag = false;


		ofxIntSlider numLevels;
		ofxPanel gui;

		bool started;
		bool gameover;
		bool victory = false;
		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bWireframe;
		bool bDisplayPoints;
		bool bPointSelected;
		bool bHide;
		bool pointSelected = false;
		bool bDisplayLeafNodes = false;
		bool bDisplayOctree = false;
		bool bDisplayBBoxes = false;
		
		bool bLanderLoaded;
		bool bTerrainSelected;
	
		ofVec3f selectedPoint;
		ofVec3f intersectPoint;

		//Altitude Stuff
		float currentAltitude;
		float getAltitude();


		//Prep Explosions
		ParticleEmitter* explosions;

		//Set Images
		ofImage playerSprite;
		ofImage enemySprite;
		ofImage background;
		ofImage asteroid;

		//Set Font
		ofTrueTypeFont font;

		//Set Sounds
		ofSoundPlayer backgroundSound;
		ofSoundPlayer thrustSound;
		ofSoundPlayer explodeSound;

		vector<Box> bboxList;
		vector<TreeNode> nodeList;

		float last_check;
		time_t time_start, time_finish;
		glm::vec3 landingZone;

		const float selectionRange = 4.0;
		float radius = 10.0;

		// textures
		//
		ofTexture  particleTex;

		// shaders
		//
		ofVbo vbo;
		ofShader shader;

		// Emitter and some forces;
		//
		ParticleEmitter emitter;
};
