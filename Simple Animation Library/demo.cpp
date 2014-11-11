#include "demo.h"


DEMO::DEMO() :
contextSettings(32), window(sf::VideoMode(800, 640), "Simple Animation Library DEMO", sf::Style::Default, contextSettings)
{}


void DEMO::start() {
	window.setFramerateLimit(180);
	window.setVerticalSyncEnabled(true);

	//Deactivate OpenGL context of window, will reopen in draw()
	window.setActive(false);

	//Start draw in a separate thread
	std::thread draw_thread(&DEMO::draw, this);

	//Event thread for input and window movement (main thread)
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


void DEMO::draw() {
	//Activate the window's context
	window.setActive();
	
	//Various settings for OpenGL
	glClearColor(0.5, 0.5, 0.5, 0.0f);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_TEXTURE_2D);

	ModelHandler modelHandler;
	modelHandler.import("C:/SALResources/collumn.dae");

	//Setup projection matrix (Camera)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLfloat ratio = float(window.getSize().x) / window.getSize().y;
	glFrustum(-ratio, ratio, -1.0, 1.0, 1.0, 500.0);

	//The rendering loop
	glMatrixMode(GL_MODELVIEW);
	double rotationAngle = 0.0; // Used to rotate the camera to get a better view of the scene
	while (window.isOpen()) {
		// Clear the screen for rerender
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		// Rotate scene
		glTranslatef(0.0f, 0.0f, -10.0f);
		glRotated(rotationAngle, 0.0, 1.0, 0.0);
		rotationAngle += 0.1;

		// Render with or without animating the scene
		modelHandler.drawAll();
		//modelHandler.drawWithoutAnimation();

		//Send to canvas for display
		window.display();
	}
}