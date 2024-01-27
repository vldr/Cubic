#pragma once
#include "VertexList.h"

#include <GL/glew.h>
#include <glm/glm.hpp>

class Level;

class Skybox
{
public:
	void init();
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

	VertexList bedrockVertices;
	GLuint bedrockTexture;

	VertexList waterVertices;
	GLuint waterTexture;

	VertexList cloudsVertices;
	GLuint cloudsTexture;
	
	VertexList skyVertices;
	GLuint skyTexture;
};

