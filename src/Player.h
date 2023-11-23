#pragma once
#include "VertexList.h"
#include "Entity.h"

class Game;

class Player : public Entity
{
public:
	void init(Game* game);
	void tick();
	void rotate(float x, float y);
	void move(float x, float y, float z);
	void render();

	unsigned int updates;
	bool flushUpdates;
private:
	float bobbing;
	float oldBobbing;
};
