#pragma once

#include <vector>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

class Model {
private:
	Assimp::Importer importer;
	std::vector<float> mesh;
	aiVector3D Model::interpolateScale(const aiNodeAnim* channel, double time);
	aiQuaternion Model::interpolateRotation(const aiNodeAnim* channel, double time);
	aiVector3D Model::interpolatePosition(const aiNodeAnim* channel, double time);
public:
	const aiScene* scene;
	bool wireframe = false;

	Model(const std::string& filename);
	~Model();
	void draw(unsigned int animation, double timeIn);
	double animationLength(unsigned int animation);
	aiMatrix4x4 parentMultiplication(const aiNode* currentNode) const;
	unsigned int numOfAnimations();
};

