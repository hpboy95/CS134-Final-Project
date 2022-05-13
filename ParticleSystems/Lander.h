#include "Particle.h"
#include "ParticleSystem.h"
#include "ofxAssimpModelLoader.h"

class Lander : public Particle{
public:
	Lander();

	void load(string filePath);
	void update();
	void addForce(ParticleForce*);
	void removeForces() { forces.clear(); }
	void draw();

	vector<ParticleForce*> forces;
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