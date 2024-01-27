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
	void init();
	void input(const SDL_Event& event);

	void tick();
	void update();
	void interact();
	void setPosition(float x, float y, float z);

	glm::vec2 viewAngles;
	glm::vec3 viewPosition;
	glm::vec3 lookAt;

	float tilt;
	float oldTilt;
	float bobbing;
	float oldBobbing;

	static const int INVENTORY_SIZE = 9;

	int inventoryIndex = 0;
	unsigned char inventory[INVENTORY_SIZE] = {
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
	enum class Move {
		None = 0,
		Left = 1 << 0,
		Right = 1 << 1,
		Forward = 1 << 2,
		Backward = 1 << 3,
		Jump = 1 << 4,
		Sprint = 1 << 5,
	};

	enum class Interact {
		None = 0,
		Left = 1 << 0,
		Right = 1 << 1,
		Middle = 1 << 2,
	};

	void turn(float rx, float ry);
	void previousInventorySlot();
	void nextInventorySlot();

	glm::vec2 controllerState = glm::vec2();
	unsigned int moveState = (unsigned int)Move::None;
	unsigned int interactState = (unsigned int)Interact::None;

	uint64_t lastClick;

	const float CONTROLLER_DEAD_ZONE = 0.20f;
	const float CONTROLLER_SPEED = 1.85f;
	const int CONTROLLER_TRIGGER_OFFSET = 10000;
	const int CONTROLLER_Y_OFFSET = 10000;
	const int CONTROLLER_X_OFFSET = 20000;

	const float CAMERA_OFFSET = 0.10f;
	const float BUILD_SPEED = 5.0f;
	const float REACH = 5.0f;
	const glm::vec3 UP = glm::vec3(0.0, 1.0, 0.0);

	friend class UI;
};

