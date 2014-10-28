#pragma once

#include <vector>

class Mesh {
public:
	unsigned int numVerts;
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> texture_coords;

	//Material might not have a texture
	unsigned int material;

	Mesh(unsigned int numFaces) : numVerts(numFaces * 3), vertices(numVerts * 3), normals(numVerts * 3), texture_coords(numVerts * 2) {};
};


