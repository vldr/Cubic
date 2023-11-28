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

	static const int SIZE = 16;

private:
	enum class FaceType { None, Front, Back, Left, Right, Top, Bottom };

	struct Face
	{
		bool valid = false;
		unsigned char blockType;
		Block::Definition blockDefinition;
		float brightness;
		float blockShift;
		bool mirror;

		bool operator==(const Face& rhs)
		{
			return rhs.valid &&
				this->valid &&
				this->blockType == rhs.blockType &&
				this->brightness == rhs.brightness &&
				this->blockShift == rhs.blockShift &&
				this->blockDefinition.height == rhs.blockDefinition.height;
		}
	};

	static Face topFaces[];
	static Face bottomFaces[];
	static Face leftFaces[];
	static Face rightFaces[];
	static Face frontFaces[];
	static Face backFaces[];

	template<FaceType faceType>
	inline void generateMesh(Face* faces);
	inline void generateFaces();

	inline Face& getFace(Face* faces, int x, int y, int z);

	bool shouldRenderFace(const Block::Definition& current, const Block::Definition& neighbor, bool isBottom = false);
	bool shouldRenderTopFace(const Block::Definition& blockDefinition, int x, int y, int z);

	Game* game;

	VertexList vertices;
	VertexList waterVertices;
};

