#include "modelhandler.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <SFML\OpenGL.hpp>
#include <iostream>

ModelHandler::ModelHandler() :
importer(Assimp::Importer()), numOfModels(0)
{}

ModelHandler::~ModelHandler()
{}

bool ModelHandler::import(std::string file)
{
	scenes.push_back(importer.ReadFile(file, aiProcess_Triangulate | aiProcessPreset_TargetRealtime_Fast));
	numOfModels++;
	if (scenes[scenes.size() - 1] != nullptr)
		return true;
	return false;
}

double r = 0.0;
void ModelHandler::drawAll()
{

	// For every scene
	for (const aiScene* scene : scenes)
	{
		// Animate scenes

		const aiAnimation* anim = scene->mAnimations[0];
		r += 0.005;
		if (r > anim->mDuration)
		{
			r = 0.0;
			std::cout << "reset" << std::endl;
		}

		for (int i = 0; i < anim->mNumChannels; i++)
		{

			const aiNodeAnim* channel = anim->mChannels[i];
			aiVector3D curPosition;
			aiQuaternion curRotation;

			// FIND TARGET NODE RECURSIVELY, START WITH ROOT NODE
			aiNode* targetNode = scene->mRootNode->mChildren[0]->mChildren[0];

			size_t posIndex = 0;
			while (1)
			{
				// break if this is the last key - there are no more keys after this one, we need to use it
				if (posIndex + 1 >= channel->mNumPositionKeys)
					break;
				// break if the next key lies in the future - the current one is the correct one then
				if (channel->mPositionKeys[posIndex + 1].mTime > r)
					break;
			}
			// maybe add a check here if the anim has any position keys at all
			curPosition = channel->mPositionKeys[posIndex].mValue;
			// same goes for rotation, but I shorten it now
			size_t rotIndex = 0;
			while (1)
			{
				if (rotIndex + 1 >= channel->mNumRotationKeys)
					break;
				if (channel->mRotationKeys[rotIndex + 1].mTime > r)
					break;
			}
			curRotation = channel->mRotationKeys[posIndex].mValue;
			// now build a transformation matrix from it. First rotation, thenn push position in it as well. 
			aiMatrix4x4 trafo;
			trafo.a4 = curPosition.x; trafo.b4 = curPosition.y; trafo.c4 = curPosition.z;
			std::cout << curPosition.x << curPosition.y << curPosition.z << std::endl;
			// assign this transformation to the node
			targetNode->mTransformation = trafo;

		}

		// Render scene

		glBegin(GL_TRIANGLES);

		// For every mesh in scene
		for (int i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* meshptr = scene->mMeshes[i];
			size_t cv = 0, ctc = 0;

			// For every face
			for (int cf = 0; cf < meshptr->mNumFaces; cf++)
			{
				const aiFace& face = meshptr->mFaces[cf];
				// For all vertices in face
				for (int cfi = 0; cfi < 3; cfi++)
				{
					double x = meshptr->mVertices[face.mIndices[cfi]].x;
					double y = meshptr->mVertices[face.mIndices[cfi]].y;
					double z = meshptr->mVertices[face.mIndices[cfi]].z;
					glVertex3d(x, y, z);
				}
			}
		}
		glEnd();


	}

}

