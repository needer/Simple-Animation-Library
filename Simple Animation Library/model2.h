#include <vector>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <assimp\vector3.h>

class Face
{

};

class Bone
{

};

class Mesh
{
public:
	std::vector<aiVector3D> vertices;
	std::vector<aiFace> faces;
};

class Model2 
{
private:
	std::vector<Mesh> meshes;

public:

	void import(const std::string filename);
};


void Model2::import(const std::string filename)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcessPreset_TargetRealtime_Fast);

	// Data import from mesh (Vertices and faces)
	for (size_t meshcount = 0; meshcount < scene->mNumMeshes; meshcount++)
	{
		Mesh& currentMesh = meshes[meshcount];
		aiMesh* currentImportMesh = scene->mMeshes[meshcount];


		// Import all vertices
		for (size_t vertexcount = 0; vertexcount < currentImportMesh->mNumVertices; vertexcount++)
		{
			currentMesh.vertices[vertexcount] = currentImportMesh->mVertices[vertexcount];
		}

		// Import all faces
		for (size_t facecount = 0; facecount < currentImportMesh->mNumFaces; facecount++)
		{
			currentMesh.faces[facecount] = currentImportMesh->mFaces[facecount];
		}
	}
	
}
