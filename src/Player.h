#pragma once
#include "VertexList.h"
#include "Entity.h"

class Player : public Entity
{
public:
	void init();
	void tick();
	void rotate(float x, float y);
	void move(float x, float y, float z);
	void render();

	unsigned int updates;
	bool flushUpdates;

	static GLuint playerTexture;
private:
	float bobbing;
	float oldBobbing;

	static VertexList head;
	static VertexList body;
	static VertexList leftArm;
	static VertexList rightArm;
	static VertexList leftLeg;
	static VertexList rightLeg;
};
