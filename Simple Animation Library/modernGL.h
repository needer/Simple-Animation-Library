#ifndef MODERNGL_H
#define MODERNGL_H

#include "SFML\Window.hpp"

class Modern
{
private:
	sf::Window window;

public:
	Modern();
	~Modern();

	void run();
	void update();
};

#endif