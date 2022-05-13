#pragma once

#include "Lander.h"

Lander::Lander() {

}

void Lander::load(string filePath) {
	model.loadModel(filePath);
}


void Lander::draw() {
	model.drawFaces();
}

void Lander::update() {
	for (int k = 0; k < forces.size(); k++) {
		if (!forces[k]->applied)
			forces[k]->updateForce(this);
	}

	for (int i = 0; i < forces.size(); i++) {
		if (forces[i]->applyOnce) {
			forces[i]->applied = true;
		}
	}

	integrate();
	model.setPosition(position.x, position.y, position.z);
}

void Lander::addForce(ParticleForce* f) {
	f->applied = false;
	forces.push_back(f);
}

// Gravity Force Field 
//
ComputeUp::ComputeUp(const ofVec3f& t) {
	thrust = t;
	applyOnce = true;
}

void ComputeUp::updateForce(Particle* particle) {
	//
	// f = mg
	//
	particle->forces += thrust * particle->mass;
}