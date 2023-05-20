#pragma once
#include "VertexList.h"

class Game;

class Player
{
public:
	void init(Game* game);
	void render();
private:
	Game* game;
	GLuint playerTexture;

	VertexList head;
	VertexList body;
	VertexList leftArm;
	VertexList rightArm;
	VertexList leftLeg;
	VertexList rightLeg;
};
