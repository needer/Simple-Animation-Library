#pragma once

#include "model.h"
#include "modelimporter.h"

class R2D2 {
private:
	Model model;
public:
	R2D2() {
		ModelImporter modelImporter;
		model = modelImporter.read("C:/SALResources/plate.dae");
	}

	void draw(int animationID, double animationTime) {
		model.draw(animationID, animationTime);
	}
};

