#include "Particle.h"
#include "ParticleSystem.h"
#include "ofxAssimpModelLoader.h"

class Lander : public Particle{
public:
	Lander();

	void load(string filePath);
	void update();
	void addForce(ParticleForce*);
	void removeForces() { forceList.clear(); }
	void draw();

	vector<ParticleForce*> forceList;
	ofxAssimpModelLoader model;
};


// Some convenient built-in forces
//
class ComputeUp : public ParticleForce {
	ofVec3f thrust;
public:
	void set(const ofVec3f& g) { thrust = g; }
	ComputeUp(const ofVec3f& thrust);
	ComputeUp() {}
	void updateForce(Particle*);
};

class ComputeDown : public ParticleForce {
	ofVec3f thrust;
public:
	void set(const ofVec3f& g) { thrust = g; }
	ComputeDown(const ofVec3f& thrust);
	ComputeDown() {}
	void updateForce(Particle*);
};

class ComputeForward : public ParticleForce {
	float thrust;
public:
	void set(const float& g) { thrust = g; }
	ComputeForward(const float& thrust);
	ComputeForward() {}
	void updateForce(Particle*);
};

class ComputeBackward : public ParticleForce {
	float thrust;
public:
	void set(const float& g) { thrust = g; }
	ComputeBackward(const float& thrust);
	ComputeBackward() {}
	void updateForce(Particle*);
};

class ComputeLeft : public ParticleForce {
	float thrust;
public:
	void set(const float& g) { thrust = g; }
	ComputeLeft(const float& thrust);
	ComputeLeft() {}
	void updateForce(Particle*);
};

class ComputeRight : public ParticleForce {
	float thrust;
public:
	void set(const float& g) { thrust = g; }
	ComputeRight(const float& thrust);
	ComputeRight() {}
	void updateForce(Particle*);
};

class ImpulseForce : public ParticleForce {
	ofVec3f thrust;
public:
	void set(const ofVec3f& g) { thrust = g; }
	ImpulseForce(const ofVec3f& thrust);
	ImpulseForce() {}
	void updateForce(Particle*);
};
