#pragma once

#include <vector>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

class Model {
private:
	Assimp::Importer importer;
public:
	const aiScene* scene;
	Model(const std::string& filename);
};

