#pragma once

#include "Lander.h"

Lander::Lander() {

}

void Lander::load(string filePath) {
	model.loadModel(filePath);
}


void Lander::draw() {
	ofPushMatrix();
	ofTranslate(position);
	ofRotateY(rotation);
	model.drawFaces();
	ofPopMatrix();
}

void Lander::update() {
	for (int k = 0; k < forceList.size(); k++) {
		if (!forceList[k]->applied)
			forceList[k]->updateForce(this);
	}

	for (int i = 0; i < forceList.size(); i++) {
		if (forceList[i]->applyOnce) {
			forceList[i]->applied = true;
		}
	}

	integrate();
}

void Lander::addForce(ParticleForce* f) {
	f->applied = false;
	forceList.push_back(f);
}

// Lander Forces
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

ComputeDown::ComputeDown(const ofVec3f& t) {
	thrust = -t;
	applyOnce = true;
}

void ComputeDown::updateForce(Particle* particle) {
	//
	// f = mg
	//
	particle->forces += thrust * particle->mass;
}

ComputeForward::ComputeForward(const float& t) {
	thrust = t;
	applyOnce = true;
}

void ComputeForward::updateForce(Particle* particle) {
	//
	// f = mg
	//
	particle->forces += thrust * particle->getHeading();
}

ComputeBackward::ComputeBackward(const float& t) {
	thrust = -t;
	applyOnce = true;
}

void ComputeBackward::updateForce(Particle* particle) {
	//
	// f = mg
	//
	particle->forces += thrust * particle->getHeading();
}

ComputeLeft::ComputeLeft(const float& t) {
	thrust = t;
	applyOnce = true;
}

void ComputeLeft::updateForce(Particle* particle) {
	//
	// f = mg
	//
	particle->angularForce -= thrust;
}

ComputeRight::ComputeRight(const float& t) {
	thrust = t;
	applyOnce = true;
}

void ComputeRight::updateForce(Particle* particle) {
	//
	// f = mg
	//
	particle->angularForce += thrust;
}

ImpulseForce::ImpulseForce(const ofVec3f& t) {
	thrust = t;
	applyOnce = true;
}

void ImpulseForce::updateForce(Particle* particle) {
	//
	// f = mg
	//
	particle->forces += thrust * particle->mass;
}



