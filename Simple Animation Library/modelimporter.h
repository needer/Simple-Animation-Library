#pragma once

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>
#include "model.h"
#include <iostream>

class ModelImporter {
public:
	Model read(const std::string& filename) {
		Model model;


		Assimp::Importer importer;
		//aiProcess_Triangulate is redundant together with aiProcessPreset_TargetRealtime_Fast, 
		//but important to keep aiProcess_Triangulate if aiProcessPreset_TargetRealtime_Fast is removed
		const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcessPreset_TargetRealtime_Fast);

		//Read textures:
		for (unsigned int cm = 0; cm < scene->mNumMaterials; cm++) {
			//Assumes 1 texture per material
			model.textures.emplace_back();
			if (scene->mMaterials[cm]->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				aiString Path;
				scene->mMaterials[cm]->GetTexture(aiTextureType_DIFFUSE, 0, &Path);
				model.textures[cm].loadFromFile(Path.C_Str());
			}
		}

		//cm=mesh counter, cv=vertex counter, ctc=texture coord counter, cf=face counter, cfi=face index counter, 
		for (unsigned int cm = 0; cm < scene->mNumMeshes; cm++)
		{
			aiMesh *mesh = scene->mMeshes[cm];

			model.meshes.emplace_back(mesh->mNumFaces);

			model.meshes[cm].material = mesh->mMaterialIndex;

			size_t cv = 0, ctc = 0;
			for (unsigned int cf = 0; cf < mesh->mNumFaces; cf++)
			{
				const aiFace& face = mesh->mFaces[cf];

				//aiProcess_Triangulate (in importer.ReadFile): face.mNumIndices=3
				for (unsigned int cfi = 0; cfi < 3; cfi++)
				{
					model.meshes[cm].vertices[cv] = mesh->mVertices[face.mIndices[cfi]].x;
					model.meshes[cm].vertices[cv + 1] = mesh->mVertices[face.mIndices[cfi]].y;
					model.meshes[cm].vertices[cv + 2] = mesh->mVertices[face.mIndices[cfi]].z;

					model.meshes[cm].normals[cv] = mesh->mNormals[face.mIndices[cfi]].x;
					model.meshes[cm].normals[cv + 1] = mesh->mNormals[face.mIndices[cfi]].y;
					model.meshes[cm].normals[cv + 2] = mesh->mNormals[face.mIndices[cfi]].z;
					cv += 3;

					model.meshes[cm].texture_coords[ctc] = mesh->mTextureCoords[0][face.mIndices[cfi]].x;
					//When reading texture with SFML, flip vertically:
					model.meshes[cm].texture_coords[ctc + 1] = 1.0 - mesh->mTextureCoords[0][face.mIndices[cfi]].y;
					ctc += 2;
				}
			}
		}

		return model;
		//Cleanup is done by Assimp::Importer-destructor
	}
};

