
#include "modelhandler.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <assimp\vector3.h>
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <SFML\OpenGL.hpp>
#include <iostream>



/*
Imports the selected file
*/
void ModelHandler::import(const std::string& filename)
{
	model = Model(filename);
}

/*
Removes the selected scene from vector and clears memory used
*/
bool ModelHandler::unload()
{
	model = (Model)0;
}

/*
Renders all scenes currently loaded without animation
*/
void ModelHandler::drawModelWithoutAnimation()
{
	// For every importer, get scene
	const aiScene* scene = model.scene;
	if (scene == nullptr)
	{
		std::cout << "Scene is a nullpointer" << std::endl;
		return;
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

double currentTime = 0.0;
/*
This function animates and draws them to the current canvas
*/
void ModelHandler::drawModel()
{
	const aiScene* scene = model.scene;

	// 1:

	// Get animation and iterate the animation timer
	const aiAnimation* anim = scene->mAnimations[0];
	currentTime += 0.1;
	if (currentTime > anim->mDuration)
	{
		currentTime = 0.0;
	}
	//std::cout << currentTime << std::endl;

	// 2:

	// For each animation channel (Kinda for each bone)
	// Interpolate the bone position and rotation with time
	// Assign the transformation to the node in question
	for (size_t i = 0; i < anim->mNumChannels; i++)
	{
		const aiNodeAnim* channel = anim->mChannels[i];
		// FIND TARGET NODE RECURSIVELY, START WITH ROOT NODE
		// Target node = selected bone? Most likely
		aiNode* targetNode = scene->mRootNode->FindNode(channel->mNodeName);		// Find the target node from the root node

		// Interpolate position
		aiVector3D& curPosition = interpolatePosition(channel, currentTime);
		aiQuaternion& curRotation = interpolateRotation(channel, currentTime);
		aiVector3D& curScale = interpolateScale(channel, currentTime);

		// Create transformation matrix from scale, rotation and position
		aiMatrix4x4 trafo = aiMatrix4x4(curScale, curRotation, curPosition);

		// assign this transformation to the node
		targetNode->mTransformation = trafo;
	}




	// For every mesh in the scene
	for (size_t i = 0; i < scene->mNumMeshes; i++)
	{

		aiMesh* meshptr = scene->mMeshes[i];

		// 3:

		std::vector<aiMatrix4x4> boneMatrices(meshptr->mNumBones);
		// For every bone in the mesh
		// Create a tranformation matrix and add to the boneMatrices vector
		for (size_t k = 0; k < meshptr->mNumBones; k++)
		{
			const aiBone* currentBone = meshptr->mBones[k];
			aiNode* currentNode = scene->mRootNode->FindNode(currentBone->mName);

			// Printing out the current nodes matrix
			/*std::cout << currentNode->mName.data << " node"<< std::endl;
			std::cout << currentNode->mTransformation.a1 << ", " << currentNode->mTransformation.a2 << ", " << currentNode->mTransformation.a3 << ", " << currentNode->mTransformation.a4 << std::endl;
			std::cout << currentNode->mTransformation.b1 << ", " << currentNode->mTransformation.b2 << ", " << currentNode->mTransformation.b3 << ", " << currentNode->mTransformation.b4 << std::endl;
			std::cout << currentNode->mTransformation.c1 << ", " << currentNode->mTransformation.c2 << ", " << currentNode->mTransformation.c3 << ", " << currentNode->mTransformation.c4 << std::endl;
			std::cout << currentNode->mTransformation.d1 << ", " << currentNode->mTransformation.d2 << ", " << currentNode->mTransformation.d3 << ", " << currentNode->mTransformation.d4 << std::endl;*/

			// Get the current bones matrix
			boneMatrices[k] = currentBone->mOffsetMatrix;

			// Print out the bones matrix before it gets animated
			/*std::cout << currentBone->mName.data << " bone" << std::endl;
			std::cout << currentBone->mOffsetMatrix.a1 << ", " << currentBone->mOffsetMatrix.a2 << ", " << currentBone->mOffsetMatrix.a3 << ", " << currentBone->mOffsetMatrix.a4 << std::endl;
			std::cout << currentBone->mOffsetMatrix.b1 << ", " << currentBone->mOffsetMatrix.b2 << ", " << currentBone->mOffsetMatrix.b3 << ", " << currentBone->mOffsetMatrix.b4 << std::endl;
			std::cout << currentBone->mOffsetMatrix.c1 << ", " << currentBone->mOffsetMatrix.c2 << ", " << currentBone->mOffsetMatrix.c3 << ", " << currentBone->mOffsetMatrix.c4 << std::endl;
			std::cout << currentBone->mOffsetMatrix.d1 << ", " << currentBone->mOffsetMatrix.d2 << ", " << currentBone->mOffsetMatrix.d3 << ", " << currentBone->mOffsetMatrix.d4 << std::endl;*/

			const aiNode* armature = scene->mRootNode->mChildren[0]; // Armature bone/node
			const aiNode* root = scene->mRootNode; // Root
			while (currentNode)
			{
				if (currentNode == armature)
					break;
				boneMatrices[k] *= currentNode->mTransformation;
				currentNode = currentNode->mParent;
			}
		}

		// 4:

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

		// 5:

		// For every face
		size_t cv = 0, ctc = 0;
		glColor3d(1.0, 1.0, 1.0);
		for (int cf = 0; cf < meshptr->mNumFaces; cf++)
		{
			const aiFace& face = meshptr->mFaces[cf];

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



/*
Renders a line using the refrenced matrix
*/
void ModelHandler::renderMatrix(const aiMatrix4x4& matrix)
{
	aiQuaternion rotation;
	aiVector3D position;
	matrix.DecomposeNoScaling(rotation, position);
	glBegin(GL_LINES);
	glVertex3d(position.x, position.y, position.z);
	glVertex3d(position.x,
		position.y + 0.5,
		position.z);
	glEnd();
}
