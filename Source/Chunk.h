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

	static const int width = 16;
	static const int height = 16;
	static const int depth = 16;

	bool isVisible;
	bool isLoaded;

	glm::ivec3 position;

	struct Comparator
	{
		bool operator()(const Chunk* a, const Chunk* b) const;
	};

private:
	bool shouldRenderFace(const Block::Definition& current, const Block::Definition& neighbor, bool isBottom = false);

	Game* game;

	VertexList vertices;
	VertexList waterVertices;
};

