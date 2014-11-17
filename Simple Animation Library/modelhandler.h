#pragma once

#include "model.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <string>
#include <vector>

class ModelHandler
{
private:
	std::vector<Model> models;

	aiVector3D interpolatePosition(const aiNodeAnim* channel, double currentTime);
	aiQuaternion interpolateRotation(const aiNodeAnim* channel, double currentTime);
	aiNode* nodeSearch(aiNode* currentNode, const aiString& targetName);

public:
	ModelHandler();
	void import(const std::string& filename);
	bool unload(int index);
	void drawAll();
	void drawWithoutAnimation();
	void renderMatrix(const aiMatrix4x4& matrix);
};