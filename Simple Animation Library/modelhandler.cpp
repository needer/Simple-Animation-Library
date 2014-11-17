#include "modelhandler.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <assimp\vector3.h>
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <SFML\OpenGL.hpp>
#include <iostream>

ModelHandler::ModelHandler() :
models(std::vector<Model>())
{}


/*
Imports the selected file
*/
void ModelHandler::import(const std::string& filename)
{
	models.emplace_back(filename);
}

/*
Removes the selected scene from vector and clears memory used
*/
bool ModelHandler::unload(int index)
{
	if (index > models.size() || index < 0)
		return false;

	models.erase(models.begin() + index);
	return true;
}

/*
Renders all scenes currently loaded without animation
*/
void ModelHandler::drawWithoutAnimation()
{
	// For every importer, get scene
	for (Model model : models)
	{
		const aiScene* scene = model.scene;
		if (scene == nullptr)
		{
			std::cout << "Scene is a nullpointer" << std::endl;
			break;
		}

		// Render scene
		glBegin(GL_TRIANGLES);
		// For every mesh in the scene
		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* meshptr = scene->mMeshes[i];
			if (meshptr == nullptr)
				break;
			// For every face
			size_t cv = 0, ctc = 0;
			for (int cf = 0; cf < meshptr->mNumFaces; cf++)
			{
				const aiFace& face = meshptr->mFaces[cf];
				// For all vertices in face
				for (int cfi = 0; cfi < 3; cfi++)
				{
					double x = meshptr->mVertices[face.mIndices[cfi]].x;
					double y = meshptr->mVertices[face.mIndices[cfi]].z;
					double z = meshptr->mVertices[face.mIndices[cfi]].y;
					glVertex3d(x, y, z);
				}
			}
		}
		glEnd();


	}
}

double currentTime = 0.0;
/*
This function animates and draws them to the current canvas
*/
void ModelHandler::drawAll()
{
	// For every scene
	for (Model model : models)
	{
		const aiScene* scene = model.scene;
		// Get animation and create the animation timer
		const aiAnimation* anim = scene->mAnimations[0];
		currentTime += 0.05;
		if (currentTime > anim->mDuration)
		{
			currentTime = 0.0;
		}

		// For each animation channel (Kinda for each bone, but not really)
		for (size_t i = 0; i < anim->mNumChannels; i++)
		{
			const aiNodeAnim* channel = anim->mChannels[i];
			// FIND TARGET NODE RECURSIVELY, START WITH ROOT NODE
			// Target node = selected bone? Most likely
			aiNode* targetNode = nodeSearch(scene->mRootNode, channel->mNodeName);

			// Interpolate position
			aiVector3D& curPosition = interpolatePosition(channel, currentTime);
			aiQuaternion& curRotation = interpolateRotation(channel, currentTime);
			
			aiMatrix4x4 trafo = aiMatrix4x4(curRotation.GetMatrix()); // Get rotation matrix
			trafo.a4 = curPosition.x; 
			trafo.b4 = curPosition.y; 
			trafo.c4 = curPosition.z;
			// assign this transformation to the node
			targetNode->mTransformation = trafo;
		}

		// Render scene
		

		// For every mesh in the scene
		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{

			aiMesh* meshptr = scene->mMeshes[i];

			std::vector<aiMatrix4x4> boneMatrices(meshptr->mNumBones);
			// For every bone in the mesh
			// Create a transofmation matrix and add to the boneMatrices vector
			for (size_t k = 0; k < meshptr->mNumBones; k++)
			{
				const aiBone* currentBone = meshptr->mBones[k];
				aiNode* currentNode = nodeSearch(scene->mRootNode, currentBone->mName);

				boneMatrices[k] = currentBone->mOffsetMatrix;
				glColor3d(1.0, 0.0, 0.0);
				renderMatrix(boneMatrices[k]);
				const aiNode* temporaryCurrentNode = currentNode;

				while (temporaryCurrentNode)
				{
					boneMatrices[k] *= temporaryCurrentNode->mTransformation;
					temporaryCurrentNode = temporaryCurrentNode->mParent;
				}
				glColor3d(0.0, 1.0, 0.0);
				renderMatrix(boneMatrices[k]);
			}

			// For every bone in the mesh (Skinning)
			// Transform mesh and put the result in the resultPosition vector
			std::vector<aiVector3D> resultPosition(meshptr->mNumVertices);
			for (size_t k = 0; k < meshptr->mNumBones; k++)
			{
				const aiBone* currentBone = meshptr->mBones[k];
				const aiMatrix4x4& positionMatrix = boneMatrices[k];
				for (size_t j = 0; j < currentBone->mNumWeights; j++)
				{
					const aiVertexWeight& weight = currentBone->mWeights[j];
					size_t vertexId = weight.mVertexId;
					const aiVector3D& srcPosition = meshptr->mVertices[vertexId];
					resultPosition[vertexId] += weight.mWeight * (positionMatrix * srcPosition);
				}
			}

			// For every face
			size_t cv = 0, ctc = 0;
			glColor3d(1.0, 1.0, 1.0);
			for (int cf = 0; cf < meshptr->mNumFaces; cf++)
			{
				const aiFace& face = meshptr->mFaces[cf];
				// For all vertices in face
				glBegin(GL_LINES);
				for (int cfi = 0; cfi < 3; cfi++)
				{
					/*
					double x = meshptr->mVertices[face.mIndices[cfi]].x;
					double y = meshptr->mVertices[face.mIndices[cfi]].y;
					double z = meshptr->mVertices[face.mIndices[cfi]].z;
					glVertex3d(x, y, z);*/

					double x = resultPosition[face.mIndices[cfi]].x;
					double y = resultPosition[face.mIndices[cfi]].y;
					double z = resultPosition[face.mIndices[cfi]].z;
					glVertex3d(x, y, z);
				}
				glEnd();
			}
		}
		


	}

}

/*
	Takes the current channel and time as parameters.
	Returns a position that has been linearly interpolated between all frames in the animation.
	*/
aiVector3D ModelHandler::interpolatePosition(const aiNodeAnim* channel, double time)
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

/*
	DOES NOT WORK PROPERLY
	Unfinished interpolation of rotations
	*/
aiQuaternion ModelHandler::interpolateRotation(const aiNodeAnim* channel, double time)
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

		std::cout << curRotation.y << std::endl;
		/*
		curRotation.w = (nextFrameRotation.w - frameRotation.w);
		curRotation.x = (nextFrameRotation.x - frameRotation.x);
		curRotation.y = (nextFrameRotation.y - frameRotation.y);
		curRotation.z = (nextFrameRotation.z - frameRotation.z);

		curRotation.w *= timePos;
		curRotation.x *= timePos;
		curRotation.y *= timePos;
		curRotation.z *= timePos;

		curRotation.w += frameRotation.w;
		curRotation.x += frameRotation.x;
		curRotation.y += frameRotation.y;
		curRotation.z += frameRotation.z;
		*/
		/*
		curRotation.w = (nextFrameRotation.x - frameRotation.x);
		curRotation.x = (nextFrameRotation.y - frameRotation.y);
		curRotation.y = (nextFrameRotation.z - frameRotation.z);
		curRotation.z = (nextFrameRotation.w - frameRotation.w);

		curRotation.w *= timePos;
		curRotation.x *= timePos;
		curRotation.y *= timePos;
		curRotation.z *= timePos;

		curRotation.w += frameRotation.x;
		curRotation.x += frameRotation.y;
		curRotation.y += frameRotation.z;
		curRotation.z += frameRotation.w;

		*/

		break;
	}
	return curRotation;
}

/*
	A recursive function that searches through all children and returnes a pointer to the target node.
	*/
aiNode* ModelHandler::nodeSearch(aiNode* currentNode, const aiString& targetName)
{
	if (currentNode->mName == targetName)
		return currentNode;
	for (size_t i = 0; i < currentNode->mNumChildren; i++)
	{
		aiNode* nodeToTest = nodeSearch(currentNode->mChildren[i], targetName);
		if (nodeToTest != nullptr)
			return nodeToTest;
	}
	return nullptr;
}

/*
Renders a line using the refrenced matrix
*/
void ModelHandler::renderMatrix(const aiMatrix4x4& matrix)
{
	aiVector3D scaling;
	aiQuaternion rotation;
	aiVector3D position;
	matrix.Decompose(scaling, rotation, position);
	glBegin(GL_LINES);
	glVertex3d(position.x, position.y, position.z);
	glVertex3d(position.x,
		position.y + 0.5,
		position.z);
	glEnd();
}