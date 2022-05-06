#include "Particle.h"
#include "ofxAssimpModelLoader.h"

class Lander : public Particle{
public:
	Lander();

	void load(string filePath);

	void draw();

	ofxAssimpModelLoader model;
};