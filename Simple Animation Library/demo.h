#pragma once

#include <thread>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>
#include <assimp/Importer.hpp>      
#include <assimp/scene.h>           
#include <assimp/postprocess.h>

class DEMO
{
private:
	sf::ContextSettings contextSettings;
	sf::Window window;

public:
	DEMO();
	void start();
	void draw();
};