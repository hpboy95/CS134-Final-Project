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