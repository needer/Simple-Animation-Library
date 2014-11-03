#pragma once

#include "mesh.h"
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

class Model {
public:
	std::vector<Mesh> meshes;
	//Some textures might be empty because we have one texture per material
	std::vector<sf::Texture> textures;
	std::vector<aiBone> bones;

	void draw(int animationID, double animationTime) {
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		for (Mesh& mesh : meshes) {
			bool texture = false;
			if (textures[mesh.material].getSize() != sf::Vector2u())
				texture = true;

			//If material has texture
			if (texture) {
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);

				sf::Texture::bind(&textures[mesh.material]);

				glTexCoordPointer(2, GL_FLOAT, 0, &mesh.texture_coords[0]);
			}
			
			glVertexPointer(3, GL_FLOAT, 0, &mesh.vertices[0]);
			glNormalPointer(GL_FLOAT, 0, &mesh.normals[0]);

			glDrawArrays(GL_TRIANGLES, 0, mesh.numVerts);

			if (texture)
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
	}
};

