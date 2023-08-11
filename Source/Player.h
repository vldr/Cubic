#pragma once
#include "VertexList.h"
#include "Entity.h"

class Game;

class Player : public Entity
{
public:
	void init(Game* game);
	void rotate(float x, float y);
	void move(float x, float y, float z);
	void render();

	bool updated;
private:
	float bobbing;
	float oldBobbing;
};
