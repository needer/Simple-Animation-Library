#include "model.h"

#include <iostream>
#include <string>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

Model::Model(const std::string& filename) : 
	importer(Assimp::Importer())
{
	scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcessPreset_TargetRealtime_Fast);
	if (scene == nullptr)
		std::cout << "Model import failed: " << filename << ". Message: " << importer.GetErrorString() << std::endl;
}
