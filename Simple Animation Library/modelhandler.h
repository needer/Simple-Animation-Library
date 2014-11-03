#pragma once

#include <assimp\Importer.hpp>
#include <string>
#include <vector>

class ModelHandler
{
private:
	Assimp::Importer importer;
	int numOfModels;
	std::vector<const aiScene*> scenes;
	
public:
	ModelHandler();
	~ModelHandler();
	bool import(std::string file);
	void drawAll();
};