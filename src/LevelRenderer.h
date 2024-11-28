#pragma once
#include "Skybox.h"
#include "Chunk.h"
#include "Level.h"

#include <queue>
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

  void loadAllChunks();
  void loadChunks(int x, int y, int z);
  Chunk* getChunk(int x, int y, int z);

private:
  void updateWaterTexture();
  void updateLavaTexture();

  const static int MAX_CHUNK_UPDATES = 4;
  const static int CHUNKS_X = Level::WIDTH / Chunk::SIZE;
  const static int CHUNKS_Y = Level::HEIGHT / Chunk::SIZE;
  const static int CHUNKS_Z = Level::DEPTH / Chunk::SIZE;

  Skybox skybox;

  Chunk chunks[CHUNKS_X * CHUNKS_Y * CHUNKS_Z];
  std::priority_queue<Chunk*, std::vector<Chunk*>, Chunk::Comparator> chunkQueue;
};

