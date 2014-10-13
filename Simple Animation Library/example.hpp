#ifndef DEMO_HPP
#define	DEMO_HPP

#include <thread>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>

class Example {
public:
	Example() : contextSettings(32),
		window(sf::VideoMode(800, 600), "SFML OpenGL & WebSocket Demo", sf::Style::Default, contextSettings),
		rotate_x(0.0), rotate_z(0.0) {}

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
	double rotate_x;
	double rotate_z;

	void draw() {
		//Activate the window's context
		window.setActive();

		//Various settings
		glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
		glShadeModel(GL_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glClearDepth(1.0f);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glEnable(GL_TEXTURE_2D);

		//Lighting
		GLfloat light_color[] = { 0.0, 0.9, 0.0, 1.f };
		glMaterialfv(GL_FRONT, GL_DIFFUSE, light_color);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

		//Load texture from image
		sf::Texture texture;
		//texture.loadFromFile("OpenGL.jpeg");
		//Binds the texture for the following glTextCoord2d calls
		//sf::Texture::bind(&texture);

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

			glTranslatef(0.0f, 0.0f, -5.0f);

			glRotatef(rotate_z, 0.0, 0.0, 1.0);
			rotate_z += 1.0;
			glRotatef(rotate_x, 1.0, 0.0, 0.0);
			rotate_x += 2.0;

			glColor3f(0.0f, 1.0f, 0.0f);

			glBegin(GL_QUADS);
			glNormal3d(0, 0, 1);
			//glTexCoord2d(0.0, 1.0);
			glVertex3d(-2.0, -1.0, 1.0);

			//glTexCoord2d(1.0, 1.0);
			glVertex3d(2.0, -1.0, 1.0);

			//glTexCoord2d(1.0, 0.0);
			glVertex3d(2.0, 1.0, 1.0);

			//glTexCoord2d(0.0, 0.0);
			glVertex3d(-2.0, 1.0, 1.0);
			glEnd();

			//Swap buffer (show result)
			window.display();
		}
	}
};

#endif	/* DEMO_HPP */

