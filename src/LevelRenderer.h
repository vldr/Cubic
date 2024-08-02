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

  void loadAllChunks();
  void loadChunks(int x, int y, int z);
  Chunk* getChunk(int x, int y, int z);

private:
  void updateWaterTexture();
  void updateLavaTexture();

  Skybox skybox;

  std::unique_ptr<Chunk[]> chunks;
  std::priority_queue<Chunk*, std::vector<Chunk*>, Chunk::Comparator> chunkQueue;

  int xChunks;
  int yChunks;
  int zChunks;

  const int MAX_CHUNK_UPDATES = 4;
};

