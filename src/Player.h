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

	bool flushUpdates;
	unsigned int updates;

	static GLuint playerTexture;
private:
	static VertexList head;
	static VertexList body;
	static VertexList leftArm;
	static VertexList rightArm;
	static VertexList leftLeg;
	static VertexList rightLeg;

	float bobbing;
	float oldBobbing;
	
	glm::vec2 rotationDelta;
};