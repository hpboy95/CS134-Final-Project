#pragma once

#include "ofMain.h"

class ParticleForceField;

class Particle {
public:
	Particle();

	ofVec3f position;
	ofVec3f velocity;
	ofVec3f acceleration;
	ofVec3f forces;
	float rotation = 0.0;
	float angularForce = 0;
	float angularVelocity = 0.0;
	float angularAcceleration = 0.0;
	float	damping;
	float   mass;
	float   lifespan;
	float   radius;
	float   birthtime;
	void    integrate();
	void    draw();
	float   age();        // sec
	glm::vec3 heading = glm::vec3(1, 0, 0);
	ofColor color;

	/*
		Returns the heading for the player
	*/
	glm::vec3 getHeading() {
		return glm::normalize(glm::vec3(getRotationMatrix() * glm::vec4(heading, 1)));
	}

	//Gets the rotation matrix of the shape
	glm::mat4 getRotationMatrix() {
		return glm::rotate(glm::mat4(1.0), glm::radians(rotation), glm::vec3(0, 1, 0));
	}
};


