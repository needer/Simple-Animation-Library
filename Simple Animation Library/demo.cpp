#include "demo.h"
#include "model.h"
#include "SFML\System.hpp"

#include <iostream>

DEMO::DEMO() :
contextSettings(32), window(sf::VideoMode(800, 640), "Simple Animation Library DEMO", sf::Style::Default, contextSettings)
{}


void DEMO::start() {
	
	//Set framerate limit and disable vertical sync
	window.setFramerateLimit(300);
	window.setVerticalSyncEnabled(false);

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
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_TEXTURE_2D);


	//Setup projection matrix (Camera)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLfloat ratio = float(window.getSize().x) / window.getSize().y;
	glFrustum(-ratio, ratio, -1.0, 1.0, 1.0, 500.0);


	// Create and import models
	Model model1 = Model("C:/SALResources/collumn.dae");
	Model model2 = Model("C:/SALResources/c.dae");
	model2.wireframe = true;
	model1.wireframe = true;

	// Create timers
	sf::Clock clock;
	sf::Time animStart1 = clock.getElapsedTime();
	sf::Time animStart2 = clock.getElapsedTime();


	//The rendering loop
	glMatrixMode(GL_MODELVIEW);

	double rotationAngle = 0.0; // Used to rotate the camera to get a better view of the scene
	while (window.isOpen()) {
		// Clear the screen, preparing next render
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		

		// Rotate scene, so the model can be viewed from all sides
		glTranslatef(0.0f, -2.0f, -15.0f);
		glRotated(rotationAngle, 0.0, 1.0, 0.0);
		rotationAngle += 0.05;


		// Move the first and second objects, so that they are standing side by side
		glTranslated(-2.5, 0.0, 0.0);

		// Render the imported models
		double deltaAnim1 = clock.getElapsedTime().asSeconds() - animStart1.asSeconds();
		model1.draw(0, deltaAnim1);
		if (deltaAnim1 > model1.animationLength(0)) // If the animation has finished
			animStart1 = clock.getElapsedTime();   // Restart animation (loop)

		// Moving second
		glTranslated(5.0, 0.0, 0.0);
		
		double deltaAnim2 = clock.getElapsedTime().asSeconds() - animStart2.asSeconds();
		model2.draw(0, deltaAnim2);
		if (deltaAnim2 > model2.animationLength(0)) // If the animation has finished
			animStart2 = clock.getElapsedTime();   // Restart animation (loop)


		//Send to canvas for display
		window.display();
	}
}