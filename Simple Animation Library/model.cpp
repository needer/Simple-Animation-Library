#include "model.h"

#include <iostream>
#include <string>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
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

Model::~Model()
{
	importer.FreeScene();
	std::cout << "Model has been deleted" << std::endl;
}

//Returns the animation length in seconds/ticks (24 frames per seconds)
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

		aiVector3D currentPosition = interpolatePosition(currentChannel, timeIn);
		aiQuaternion currentRotation = interpolateRotation(currentChannel, timeIn);
		aiVector3D currentScale = interpolateScale(currentChannel, timeIn);

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
			aiBone* currentBone = currentMesh->mBones[i];
			aiNode* currentNode = scene->mRootNode->FindNode(currentBone->mName);
			boneTransformations[i] = parentMultiplication(currentNode);
			boneTransformations[i] *= currentMesh->mBones[i]->mOffsetMatrix;
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
			if (wireframe)
				glBegin(GL_LINE_LOOP);
			else
				glBegin(GL_TRIANGLES);
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

// Bone multiplication, generates a matrix with the final transformation in an animation
aiMatrix4x4 Model::parentMultiplication(const aiNode* currentNode) const
{
	std::vector<aiNode*> nodes;
	aiNode* tempNode = currentNode->mParent;
	while (tempNode->mParent != scene->mRootNode)
	{
		nodes.push_back(tempNode);
		tempNode = tempNode->mParent;
	}
	aiMatrix4x4 matrix;
	for (unsigned int i = nodes.size(); i != 0; i--)
	{
		matrix *= nodes[i - 1]->mTransformation;
	}
	matrix *= currentNode->mTransformation;
	return matrix;
}

unsigned int Model::numOfAnimations()
{
	return scene->mNumAnimations;
}


//Interpolates the position between two frames
aiVector3D Model::interpolatePosition(const aiNodeAnim* channel, double time)
{
	aiVector3D curPosition;

	for (size_t t = 0; t < channel->mNumPositionKeys; t++)
	{
		// If this is the last key, set position to last frame and break loop
		if (t == channel->mNumPositionKeys - 1)
		{
			curPosition = channel->mPositionKeys[t].mValue;
			break;
		}

		// Next key is behind in time, go to next key
		if (channel->mPositionKeys[t + 1].mTime < time)
			continue;


		// Current frame found, linear interpolation of position
		const aiVector3D& framePosition = channel->mPositionKeys[t].mValue;
		const aiVector3D& nextFramePosition = channel->mPositionKeys[t + 1].mValue;

		double frameLength = channel->mPositionKeys[t + 1].mTime - channel->mPositionKeys[t].mTime;
		double timePos = (time - channel->mPositionKeys[t].mTime) / frameLength;


		curPosition = (nextFramePosition - framePosition);
		curPosition.x *= timePos;
		curPosition.y *= timePos;
		curPosition.z *= timePos;
		curPosition += framePosition;

		break;
	}
	return curPosition;
}

//Interpolates the rotation between two frames
aiQuaternion Model::interpolateRotation(const aiNodeAnim* channel, double time)
{
	aiQuaternion curRotation;
	for (size_t t = 0; t < channel->mNumRotationKeys; t++)
	{
		// If this is the last key, set position to last frame and break loop
		if (t == channel->mNumRotationKeys - 1)
		{
			curRotation = channel->mRotationKeys[t].mValue;
			break;
		}

		// Next key is behind in time, go to next key
		if (channel->mRotationKeys[t + 1].mTime < time)
			continue;


		// Current frame found, linear interpolation of position
		const aiQuaternion& frameRotation = channel->mRotationKeys[t].mValue;
		const aiQuaternion& nextFrameRotation = channel->mRotationKeys[t + 1].mValue;

		double frameLength = channel->mRotationKeys[t + 1].mTime - channel->mRotationKeys[t].mTime;
		double timePos = (time - channel->mRotationKeys[t].mTime) / frameLength;

		aiQuaternion::Interpolate(curRotation, frameRotation, nextFrameRotation, timePos);


		break;
	}
	return curRotation;
}

//Interpolates the scale between two frames
aiVector3D Model::interpolateScale(const aiNodeAnim* channel, double time)
{
	aiVector3D curScale(1.0f);
	for (size_t t = 0; t < channel->mNumScalingKeys; t++)
	{
		// If this is the last key, set position to last frame and break loop
		if (t == channel->mNumScalingKeys - 1)
		{
			curScale = channel->mScalingKeys[t].mValue;
			break;
		}

		// Next key is behind in time, go to next key
		if (channel->mPositionKeys[t + 1].mTime < time)
			continue;


		// Current frame found, linear interpolation of position
		const aiVector3D& frameScale = channel->mScalingKeys[t].mValue;
		const aiVector3D& nextFrameScale = channel->mScalingKeys[t + 1].mValue;

		double frameLength = channel->mScalingKeys[t + 1].mTime - channel->mScalingKeys[t].mTime;
		double timePos = (time - channel->mScalingKeys[t].mTime) / frameLength;


		curScale = (nextFrameScale - frameScale);
		curScale.x *= timePos;
		curScale.y *= timePos;
		curScale.z *= timePos;
		curScale += frameScale;

		break;
	}
	return curScale;
}