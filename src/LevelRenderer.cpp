#include "LevelRenderer.h"
#include "Game.h"
#include "Chunk.h"
#include "Skybox.h"
#include "Random.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

void LevelRenderer::init()
{
  skybox.init();

  for (int x = 0; x < CHUNKS_X; x++)
  {
    for (int y = 0; y < CHUNKS_Y; y++)
    {
      for (int z = 0; z < CHUNKS_Z; z++)
      {
        auto chunk = getChunk(x, y, z);
        chunk->init(Chunk::SIZE * x, Chunk::SIZE * y, Chunk::SIZE * z);
      }
    }
  }
}

void LevelRenderer::render()
{
  int chunkUpdates = 0;
  while (chunkUpdates < MAX_CHUNK_UPDATES && !chunkQueue.empty())
  {
    Chunk* chunk = chunkQueue.top();
    chunk->isLoaded = true;
    chunk->update();

    chunkUpdates++;
    chunkQueue.pop();
  }

  game.chunkUpdates += chunkUpdates;

  glBindTexture(GL_TEXTURE_2D, game.atlasTexture);
  
  for (auto& chunk : chunks)
  {
    chunk.isVisible = game.frustum.contains(&chunk);

    if (chunk.isVisible)
    {
      chunk.render();
    }
  }

  skybox.renderBedrock();
  skybox.renderSky();
  skybox.renderClouds();
}

void LevelRenderer::renderPost()
{
  glBindTexture(GL_TEXTURE_2D, game.atlasTexture);
  glColorMask(false, false, false, false);

  for (auto& chunk : chunks)
  {
    if (chunk.isVisible)
    {
      chunk.renderWater();
    }
  }

  skybox.renderWater();

  glBindTexture(GL_TEXTURE_2D, game.atlasTexture);
  glColorMask(true, true, true, true);

  for (auto& chunk : chunks)
  {

    if (chunk.isVisible)
    {
      chunk.renderWater();
    }
  }

  skybox.renderWater();
}

void LevelRenderer::tick()
{
  glBindTexture(GL_TEXTURE_2D, game.atlasTexture);

  updateLavaTexture();
  updateWaterTexture();
}

void LevelRenderer::updateWaterTexture()
{
  static unsigned char waterTextureData[1024] = {};
  static float waterTextureRed[256] = {};
  static float waterTextureGreen[256] = {};
  static float waterTextureBlue[256] = {};
  static float waterTextureAlpha[256] = {};

  for (int x = 0; x < 16; x++) 
  {
    for (int y = 0; y < 16; y++) 
    {
      float v = 0.0;
      for (int i = x - 1; i <= x + 1; i++) 
      {
        int a = i & 15;
        int b = y & 15;
        v += waterTextureRed[a + (b << 4)];
      }

      waterTextureBlue[x + (y << 4)] = v / 3.3f + waterTextureGreen[x + (y << 4)] * 0.8f;
    }
  }

  for (int x = 0; x < 16; x++) 
  {
    for (int y = 0; y < 16; y++) 
    {
      waterTextureGreen[x + (y << 4)] += waterTextureAlpha[x + (y << 4)] * 0.05f;
      if (waterTextureGreen[x + (y << 4)] < 0.0) { waterTextureGreen[x + (y << 4)] = 0.0; }
      waterTextureAlpha[x + (y << 4)] -= 0.1f;
      if (game.random.uniform() < 0.05) { waterTextureAlpha[x + (y << 4)] = 0.5; }
    }
  }

  for (int i = 0; i < 256; i++) 
  {
    float bluePrevious = waterTextureBlue[i];
    waterTextureBlue[i] = waterTextureRed[i];
    waterTextureRed[i] = bluePrevious;
    float v = waterTextureRed[i];
    if (v > 1.0) { v = 1.0; }
    if (v < 0.0) { v = 0.0; }
    int a = (int)(32.0f + v * v * 32.0f);
    int b = (int)(50.0f + v * v * 64.0f);
    int c = (int)255;
    int d = (int)(180.0f + v * v * 45.0f);

    waterTextureData[i << 2] = a;
    waterTextureData[(i << 2) + 1] = b;
    waterTextureData[(i << 2) + 2] = c;
    waterTextureData[(i << 2) + 3] = d;
  }

  const auto texture = Block::Definitions[(unsigned char)Block::Type::BLOCK_WATER].topTexture;
  glTexSubImage2D(GL_TEXTURE_2D, 0, texture % 16 << 4, texture / 16 << 4, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, waterTextureData);
  
  skybox.updateWater(waterTextureData);
}

void LevelRenderer::updateLavaTexture()
{
  static unsigned char lavaTextureData[1024] = {};
  static float lavaTextureRed[256] = {};
  static float lavaTextureGreen[256] = {};
  static float lavaTextureBlue[256] = {};
  static float lavaTextureAlpha[256] = {};

  for (int x = 0; x < 16; x++) 
  {
    for (int y = 0; y < 16; y++) 
    {
      int sy = (int)(glm::sin(y * M_PI / 8.0) * 1.2);
      int sx = (int)(glm::sin(x * M_PI / 8.0) * 1.2);
      float v = 0.0;
      for (int i = x - 1; i <= x + 1; i++) {
        for (int j = y - 1; j <= y + 1; j++) {
          int a = i + sy & 15;
          int b = j + sx & 15;
          v += lavaTextureRed[a + (b << 4)];
        }
      }
      lavaTextureGreen[x + (y << 4)] = v / 10.0f + (lavaTextureBlue[(x & 15) + ((y & 15) << 4)] + lavaTextureBlue[(x + 1 & 15) + ((y & 15) << 4)]
        + lavaTextureBlue[(x + 1 & 15) + ((y + 1 & 15) << 4)] + lavaTextureBlue[(x & 15) + ((y + 1 & 15) << 4)]) / 4.0f * 0.8f;
      lavaTextureBlue[x + (y << 4)] += lavaTextureAlpha[x + (y << 4)] * 0.01f;
      if (lavaTextureBlue[x + (y << 4)] < 0.0) { lavaTextureBlue[x + (y << 4)] = 0.0; }
      lavaTextureAlpha[x + (y << 4)] -= 0.06f;
      if (game.random.uniform() < 0.005) { lavaTextureAlpha[x + (y << 4)] = 1.5; }
    }
  }

  for (int i = 0; i < 256; i++) 
  {
    float red = lavaTextureRed[i];
    lavaTextureRed[i] = lavaTextureGreen[i];
    lavaTextureGreen[i] = red;
    float v = lavaTextureRed[i] * 2.0f;
    if (v > 1.0) { v = 1.0f; }
    if (v < 0.0) { v = 0.0f; }
    int a = (int)(v * 100.0f + 155.0f);
    int b = (int)(v * v * 255.0f);
    int c = (int)(v * v * v * v * 128.0f);

    lavaTextureData[i << 2] = a;
    lavaTextureData[(i << 2) + 1] = b;
    lavaTextureData[(i << 2) + 2] = c;
    lavaTextureData[(i << 2) + 3] = -1;
  }

  const auto texture = Block::Definitions[(unsigned char)Block::Type::BLOCK_LAVA].topTexture;
  glTexSubImage2D(GL_TEXTURE_2D, 0, texture % 16 << 4, texture / 16 << 4, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, lavaTextureData);
}

void LevelRenderer::loadAllChunks()
{
  for (auto& chunk : chunks)
  {
    chunk.isLoaded = false;

    chunkQueue.push(&chunk);
  }
}

void LevelRenderer::loadChunks(int x, int y, int z)
{
  static const glm::ivec3 offsets[] = {
    glm::ivec3(0, 0, 0),
    glm::ivec3(1, 0, 0),
    glm::ivec3(-1, 0, 0),
    glm::ivec3(0, 1, 0),
    glm::ivec3(0, -1, 0),
    glm::ivec3(0, 0, 1),
    glm::ivec3(0, 0, -1),
  };

  for (const auto& offset : offsets)
  {
    auto offsetX = (x + offset.x) / Chunk::SIZE;
    auto offsetY = (y + offset.y) / Chunk::SIZE;
    auto offsetZ = (z + offset.z) / Chunk::SIZE;

    if (
      offsetX < 0 || offsetY < 0 || offsetZ < 0 || 
      offsetX > CHUNKS_X - 1 || offsetY > CHUNKS_Y - 1 || offsetZ > CHUNKS_Z - 1
    )
    {
      continue;
    }

    if (Chunk* chunk = getChunk(offsetX, offsetY, offsetZ); chunk->isLoaded)
    {
      chunk->isLoaded = false;

      chunkQueue.push(chunk);
    }
  }
}

Chunk* LevelRenderer::getChunk(int x, int y, int z)
{
  return &chunks[(z * CHUNKS_Y + y) * CHUNKS_X + x];
}
