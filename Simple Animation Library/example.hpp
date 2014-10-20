#ifndef DEMO_HPP
#define	DEMO_HPP

#include <thread>

#include <cstring>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>

//General 3D-model importer, only tested with OBJ exported by Blender (import other 3D-model-files to Blender and export to OBJ)
//For use together with low-lever OpenGL programming
//When exporting OBJ with textures from Blender, you might want to edit the accompanying mtl-file to use relative paths
//Supports (for simplicity): vertices, normals and textures only (for instance no support for colors, skeletons and animations)
//Assumptions: 2d-texture, one texture per material, and external texture files (that has to be flipped vertically when read by SFML)
class ModelImporter {
public:
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

	class Model {
	public:
		std::vector<Mesh> meshes;
		//Some textures might be empty because we have one texture per material
		std::vector<sf::Texture> textures;

		void draw() {
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);
			for (auto& mesh : meshes) {
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

	Model read(const std::string& filename) {
		Model model;

		Assimp::Importer importer;
		//aiProcess_Triangulate is redundant together with aiProcessPreset_TargetRealtime_Fast, 
		//but important to keep aiProcess_Triangulate if aiProcessPreset_TargetRealtime_Fast is removed
		const aiScene *scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcessPreset_TargetRealtime_Fast);

		//Read textures:
		for (unsigned int cm = 0; cm<scene->mNumMaterials; cm++) {
			//Assumes 1 texture per material
			model.textures.emplace_back();
			if (scene->mMaterials[cm]->GetTextureCount(aiTextureType_DIFFUSE)>0) {
				aiString Path;
				scene->mMaterials[cm]->GetTexture(aiTextureType_DIFFUSE, 0, &Path);
				model.textures[cm].loadFromFile(Path.C_Str());
			}
		}

		//cm=mesh counter, cv=vertex counter, ctc=texture coord counter, cf=face counter, cfi=face index counter, 
		for (unsigned int cm = 0; cm<scene->mNumMeshes; cm++) {
			aiMesh *mesh = scene->mMeshes[cm];

			model.meshes.emplace_back(mesh->mNumFaces);

			model.meshes[cm].material = mesh->mMaterialIndex;

			size_t cv = 0, ctc = 0;
			for (unsigned int cf = 0; cf<mesh->mNumFaces; cf++) {
				const aiFace& face = mesh->mFaces[cf];

				//aiProcess_Triangulate (in importer.ReadFile): face.mNumIndices=3
				for (unsigned int cfi = 0; cfi<3; cfi++) {
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

class R2D2 {
private:
	ModelImporter::Model model;
public:
	R2D2() {
		ModelImporter modelImporter;
		model = modelImporter.read("r2d2.obj");
	}

	void draw() {
		model.draw();
	}
};

class Example {
public:
	Example() : contextSettings(32),
		window(sf::VideoMode(800, 640), "SFML OpenGL & WebSocket Demo", sf::Style::Default, contextSettings),
		rotate_y(0.0) {}

	void start() {
		window.setFramerateLimit(100);
		window.setVerticalSyncEnabled(true);

		//Deactivate OpenGL context of window, will reopen in draw()
		window.setActive(false);

		//Start draw in a separate thread
		std::thread draw_thread(&Example::draw, this);

		//Event thread (main thread)
		while (window.isOpen()) {
			sf::Event event;
			while (window.pollEvent(event)) {
				if (event.type == sf::Event::KeyPressed) {
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
						window.close();
					}
				}
				if (event.type == sf::Event::Closed) {
					window.close();
				}
			}
		}

		//Wait for draw_thread to also finish for clean exit
		draw_thread.join();
	}

private:
	sf::ContextSettings contextSettings;
	sf::Window window;
	double rotate_y;

	void draw() {
		//Activate the window's context
		window.setActive();

		//Various settings
		glClearColor(0.5, 0.5, 0.5, 0.0f);
		glShadeModel(GL_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glClearDepth(1.0f);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glEnable(GL_TEXTURE_2D);

		//Lighting
		GLfloat light_color[] = { 0.9, 0.9, 0.9, 1.f };
		glMaterialfv(GL_FRONT, GL_DIFFUSE, light_color);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

		//Load our robot
		R2D2 r2d2;

		//Setup projection matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		GLfloat ratio = float(window.getSize().x) / window.getSize().y;
		glFrustum(-ratio, ratio, -1.0, 1.0, 1.0, 500.0);

		//The rendering loop
		glMatrixMode(GL_MODELVIEW);
		while (window.isOpen()) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glLoadIdentity();

			glTranslatef(0.0f, -20.0f, -60.0f);

			glRotatef(rotate_y, 0.0, 1.0, 0.0);
			rotate_y += 2.0;

			glTranslatef(20.0, 0.0, 0.0);

			r2d2.draw();

			//Swap buffer (show result)
			window.display();
		}
	}
};

#endif	/* DEMO_HPP */