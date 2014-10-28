#pragma once

#include <thread>

#include <cstring>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>
#include "mesh.h"
#include "model.h"
#include "r2d2.h"

//General 3D-model importer, only tested with OBJ exported by Blender (import other 3D-model-files to Blender and export to OBJ)
//For use together with low-lever OpenGL programming
//When exporting OBJ with textures from Blender, you might want to edit the accompanying mtl-file to use relative paths
//Supports (for simplicity): vertices, normals and textures only (for instance no support for colors, skeletons and animations)
//Assumptions: 2d-texture, one texture per material, and external texture files (that has to be flipped vertically when read by SFML)



class Example {
public:
	Example() : contextSettings(32),
		window(sf::VideoMode(800, 640), "SFML OpenGL & WebSocket Demo", sf::Style::Default, contextSettings),
		rotate_y(0.0) {}

	void start() {
		window.setFramerateLimit(180);
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

			glTranslatef(0.0f, 0.0f, -20.0f);

			r2d2.draw(0, 0.0);

			//Swap buffer (show result)
			window.display();
		}
	}
};
