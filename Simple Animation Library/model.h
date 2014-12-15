#pragma once

#include <vector>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

class Model {
private:
	Assimp::Importer importer;
	std::vector<float> mesh;
public:
	const aiScene* scene;
	Model(const std::string& filename);
	void draw(unsigned int animation, double timeIn);
	double animationLength(unsigned int animation);
};

