#pragma once
#include "Skybox.h"
#include "Chunk.h"

#include <queue>
#include <GL/glew.h>

class Level;
class Game;
class Random;

class LevelRenderer
{
public:
	void init(Game* game);
	void render();
	void renderPost();
	void tick();

	void initChunks();
	void loadChunks(int x0, int y0, int z0, int x1, int y1, int z1);

	Chunk* getChunk(int x, int y, int z);

	unsigned char waterTextureData[1024] = {0};
	unsigned char lavaTextureData[1024] = {0};

	std::priority_queue<Chunk*, std::vector<Chunk*>, Chunk::Comparator> chunksQueue;
private:
	void updateWaterTexture();
	void updateLavaTexture();

	Skybox skybox;

	Game* game;
	Chunk* chunks;

	float waterTextureRed[256] = {0};
	float waterTextureGreen[256] = {0};
	float waterTextureBlue[256] = {0};
	float waterTextureAlpha[256] = {0};

	float lavaTextureRed[256] = {0};
	float lavaTextureGreen[256] = {0};
	float lavaTextureBlue[256] = {0};
	float lavaTextureAlpha[256] = {0};

	int xChunks;
	int yChunks;
	int zChunks;

	const int maxChunkUpdates = 4;
};

