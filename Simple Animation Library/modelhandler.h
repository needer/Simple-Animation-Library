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
	Model model;

	void import(const std::string& filename);
	bool unload();
	void drawModel();
	void drawModelWithoutAnimation();
	void renderMatrix(const aiMatrix4x4& matrix);

	static aiVector3D interpolatePosition(const aiNodeAnim* channel, double currentTime);
	static aiQuaternion interpolateRotation(const aiNodeAnim* channel, double currentTime);
	static aiVector3D interpolateScale(const aiNodeAnim* channel, double currentTime);

};