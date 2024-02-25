#pragma once
#include "Skybox.h"
#include "Chunk.h"

#include <queue>
#include <memory>
#include <GL/glew.h>

class Level;
class Random;

class LevelRenderer
{
public:
	void init();
	void render();
	void renderPost();
	void tick();

	void initChunks();
	void loadChunks(int x0, int y0, int z0, int x1, int y1, int z1);

	Chunk* getChunk(int x, int y, int z);

	unsigned char waterTextureData[1024] = {};
	unsigned char lavaTextureData[1024] = {};

	std::priority_queue<Chunk*, std::vector<Chunk*>, Chunk::Comparator> chunkQueue;
private:
	void updateWaterTexture();
	void updateLavaTexture();

	Skybox skybox;

	std::unique_ptr<Chunk[]> chunks;

	float waterTextureRed[256] = {};
	float waterTextureGreen[256] = {};
	float waterTextureBlue[256] = {};
	float waterTextureAlpha[256] = {};

	float lavaTextureRed[256] = {};
	float lavaTextureGreen[256] = {};
	float lavaTextureBlue[256] = {};
	float lavaTextureAlpha[256] = {};

	int xChunks;
	int yChunks;
	int zChunks;

	const int MAX_CHUNK_UPDATES = 4;
};

