#include "model.h"

#include <iostream>
#include <string>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include "modelhandler.h"
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <SFML\OpenGL.hpp>

Model::Model(const std::string& filename) : 
	importer(Assimp::Importer())
{
	scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcessPreset_TargetRealtime_Fast);
	if (scene == nullptr)
		std::cout << "Model import failed: " << filename << ". Message: " << importer.GetErrorString() << std::endl;
}

double Model::animationLength(unsigned int animation)
{
	if (scene->mNumAnimations <= animation)
		return 0.0;
	return scene->mAnimations[animation]->mDuration;
}

void Model::draw(unsigned int animation, double timeIn)
{
	if (scene->mNumAnimations <= animation)
		return;
	
	aiAnimation* currentAnimation = scene->mAnimations[animation];

	//2: Interpolate position, rotation, scale in the all channels and add to transformation
	for (unsigned int i = 0; i < currentAnimation->mNumChannels; i++)
	{
		aiNodeAnim* currentChannel = currentAnimation->mChannels[i];

		aiVector3D& currentPosition = ModelHandler::interpolatePosition(currentChannel, timeIn);
		aiQuaternion& currentRotation = ModelHandler::interpolateRotation(currentChannel, timeIn);
		aiVector3D& currentScale = ModelHandler::interpolateScale(currentChannel, timeIn);

		aiMatrix4x4& transformation = aiMatrix4x4(currentScale, currentRotation, currentPosition);

		aiNode* currentNode = scene->mRootNode->FindNode(currentChannel->mNodeName);
		currentNode->mTransformation = transformation;
	}

	for (unsigned int k = 0; k < scene->mNumMeshes; k++)
	{

		//3: Bone transformation, change transformation of the bone according to animation
		aiMesh* currentMesh = scene->mMeshes[k];

		std::vector<aiMatrix4x4> boneTransformations = std::vector<aiMatrix4x4>(currentMesh->mNumBones);
		for (unsigned int i = 0; i < currentMesh->mNumBones; i++)
		{
			boneTransformations[i] = currentMesh->mBones[i]->mOffsetMatrix;
			aiBone* currentBone = currentMesh->mBones[i];
			aiNode* currentNode = scene->mRootNode->FindNode(currentBone->mName);
			
			while (currentNode)
			{
				if (currentNode == scene->mRootNode)
					break;
				boneTransformations[i] *= currentNode->mTransformation;
				currentNode = currentNode->mParent;
			}
		}

		//4: Skinning
		std::vector<aiVector3D> resultPosition(currentMesh->mNumVertices);
		for (size_t k = 0; k < currentMesh->mNumBones; k++)
		{
			const aiBone* currentBone = currentMesh->mBones[k];

			const aiMatrix4x4& positionMatrix = boneTransformations[k];
			for (size_t j = 0; j < currentBone->mNumWeights; j++)
			{
				const aiVertexWeight& weight = currentBone->mWeights[j];
				size_t vertexId = weight.mVertexId;
				const aiVector3D& srcPosition = currentMesh->mVertices[vertexId];
				resultPosition[vertexId] += weight.mWeight * (positionMatrix * srcPosition);
			}
		}

		//5: Draw the final model
		// For every face
		size_t cv = 0, ctc = 0;
		glColor3d(1.0, 1.0, 1.0);
		for (int cf = 0; cf < currentMesh->mNumFaces; cf++)
		{
			const aiFace& face = currentMesh->mFaces[cf];

			// For all vertices in face (Final drawing)
			glBegin(GL_LINES);
			for (int cfi = 0; cfi < 3; cfi++)
			{
				double x = resultPosition[face.mIndices[cfi]].x;
				double y = resultPosition[face.mIndices[cfi]].y;
				double z = resultPosition[face.mIndices[cfi]].z;
				glVertex3d(x, y, z);
			}

			glEnd();
		}
	}



}