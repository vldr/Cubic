#pragma once
#include "Block.h"
#include "VertexList.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class Game;

class Chunk
{
public:
	void init(Game* game, int x, int y, int z);
	void render();
	void renderWater();
	void update();
	float distanceToPlayer() const;

	bool isVisible;
	bool isLoaded;

	glm::ivec3 position;

	struct Comparator
	{
		bool operator()(const Chunk* a, const Chunk* b) const;
	};

	static const int WIDTH = 16;
	static const int HEIGHT = 16;
	static const int DEPTH = 16;

private:
	bool shouldRenderFace(const Block::Definition& current, const Block::Definition& neighbor, bool isBottom = false);

	Game* game;

	VertexList vertices;
	VertexList waterVertices;
};

