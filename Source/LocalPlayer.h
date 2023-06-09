#pragma once
#include "Entity.h"
#include "AABBPosition.h"
#include "Block.h"

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/common.hpp>

class Game;
struct AABBPosition;

class LocalPlayer : public Entity
{
public:
	void init(Game* game);

	void input(const SDL_Event& event);
	void tick();
	void update();
	void setPosition(float x, float y, float z);

	glm::vec2 viewAngles;
	glm::vec3 viewPosition;
	glm::vec3 lookAt;

	float tilt;
	float oldTilt;
	float bobbing;
	float oldBobbing;

	static const int inventorySize = 9;
	int inventoryIndex = 0;
	unsigned char inventory[inventorySize] = {
		(unsigned char)Block::Type::BLOCK_STONE,
		(unsigned char)Block::Type::BLOCK_COBBLE,
		(unsigned char)Block::Type::BLOCK_BRICK,
		(unsigned char)Block::Type::BLOCK_DIRT,
		(unsigned char)Block::Type::BLOCK_WOOD,
		(unsigned char)Block::Type::BLOCK_LOG,
		(unsigned char)Block::Type::BLOCK_LEAVES,
		(unsigned char)Block::Type::BLOCK_GLASS,
		(unsigned char)Block::Type::BLOCK_SLAB,
	};

	AABBPosition selected;
	int selectedIndex;

private:
	uint64_t lastClick;

	enum Move {
		Move_None = 0,
		Move_Left = 1 << 0,
		Move_Right = 1 << 1,
		Move_Forward = 1 << 2,
		Move_Backward = 1 << 3,
		Move_Jump = 1 << 4,
		Move_Sprint = 1 << 5,
	};

	enum Interact {
		Interact_None = 0,
		Interact_Left = 1 << 0,
		Interact_Right = 1 << 1,
		Interact_Middle = 1 << 2,
	};

	unsigned int moveState = Move::Move_None;
	unsigned int interactState = Interact::Interact_None;

	const float CAMERA_OFFSET = 0.10f;
	const float BUILD_SPEED = 5.0f;
	const float REACH = 5.0f;
	const glm::vec3 UP = glm::vec3(0.0, 1.0, 0.0);
};

