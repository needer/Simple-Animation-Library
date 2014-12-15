#pragma once

#include "model.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <string>
#include <vector>

class ModelHandler
{
public:
	std::vector<Model> models;

	ModelHandler();
	void import(const std::string& filename);
	bool unload(int index);
	void drawAll();
	void drawWithoutAnimation();
	void renderMatrix(const aiMatrix4x4& matrix);

	static aiVector3D& interpolatePosition(const aiNodeAnim* channel, double currentTime);
	static aiQuaternion& interpolateRotation(const aiNodeAnim* channel, double currentTime);
	static aiVector3D& interpolateScale(const aiNodeAnim* channel, double currentTime);

};