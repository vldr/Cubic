#pragma once
#include "Entity.h"

class Particle : public Entity
{
public:
	void init(Game* game, int index, float x, float y, float z, float xd, float yd, float zd, unsigned char blockType);
	void tick();
	void update();

	bool isValid;
	int index;
private:
	int age;
	int lifeTime;
	float size;
};

