#pragma once
#include "AABB.h"

#include <glm/glm.hpp>

class Game;

class Entity
{
public:
	void init(Game* game);
	void tick();

	void setSize(float w, float h);
	void setPosition(float x, float y, float z);
	void turn(float rx, float ry);
	bool isFree(float ax, float ay, float az);
	bool isInWater();
	bool isInLava();
	void moveRelative(float x, float z, float speed);
	void move(float ax, float ay, float az);

	glm::vec3 position;
	glm::vec3 oldPosition;

	glm::vec2 rotation;
	glm::vec2 oldRotation;

	glm::vec3 velocity;

	float oldWalkDistance;
	float walkDistance;

	AABB aabb;
protected:
	Game* game;

	bool noPhysics;
	bool onGround;
	bool collision;
	bool horizontalCollision;
	bool slide;
	bool makeStepSound;

	float heightOffset;
	float aabbWidth;
	float aabbHeight;
	float fallDistance;
	float slideOffset;
	float footSize;

	int nextStep;
};

