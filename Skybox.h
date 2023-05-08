#pragma once
#include "VertexList.h"

#include <GL/glew.h>
#include <glm/glm.hpp>

class Game;
class Level;

class Skybox
{
public:
	void init(Game* game);
	void renderBedrock();
	void updateWater(unsigned char* waterTextureData);
	void renderWater();
	void renderClouds();
	void renderSky();
private:
	void initBedrock();
	void initWater();
	void initClouds();
	void initSky();

	Game* game;

	VertexList bedrockVertices;
	GLuint bedrockTexture;

	VertexList waterVertices;
	GLuint waterTexture;

	VertexList cloudsVertices;
	GLuint cloudsTexture;
	
	VertexList skyVertices;
	GLuint skyTexture;
};

