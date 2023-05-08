#pragma once
#include "VertexList.h"

#include <GL/glew.h>

class Game;

class HeldBlock
{
public:
	void init(Game* game);
	void update();
	void tick();
	void render();
	void swing();
	void reset();
private:
	float height;
	float lastHeight;

	int swingOffset;
	bool isSwinging;

	Game* game;
	VertexList vertices;
};

