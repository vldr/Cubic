#pragma once
#include "Block.h"
#include "AABBPosition.h"

#include <glm/glm.hpp>
#include <queue>
#include <memory>

class AABB;

class Level {
public:
  const static int WIDTH = 128;
  const static int HEIGHT = 64;
  const static int DEPTH = 128;

  void init();
  void tick();
  void reset();

  void setTile(int x, int y, int z, unsigned char blockType, bool mode = false);
  bool setTileWithNeighborChange(int x, int y, int z, unsigned char blockType, bool mode = false);
  bool setTileWithNoNeighborChange(int x, int y, int z, unsigned char blockType, bool mode = false);

  void addedTile(int x, int y, int z, unsigned char blockType);
  void removedTile(int x, int y, int z, unsigned char blockType);
  void updateTile(int x, int y, int z, bool deferred = false);

  bool canFlood(int x, int y, int z, unsigned char blockType);

  bool isAirTile(int x, int y, int z);
  bool isAirTile(unsigned char blockType);
  bool isWaterTile(int x, int y, int z);
  bool isRenderWaterTile(float x, float y, float z);
  bool isMovingWaterTile(int x, int y, int z);
  bool isMovingWaterTile(unsigned char blockType);
  bool isWaterTile(unsigned char blockType);
  bool isLavaTile(float x, float y, float z);
  bool isMovingLavaTile(int x, int y, int z);
  bool isMovingLavaTile(unsigned char blockType);
  bool isLavaTile(int x, int y, int z);
  bool isLavaTile(unsigned char blockType);
  bool isInBounds(int x, int y, int z);
  bool isTileLit(int x, int y, int z);

  float getTileBrightness(int x, int y, int z);
  unsigned char getTile(int x, int y, int z);
  unsigned char getRenderTile(int x, int y, int z);

  unsigned int getTileAABBCount(AABB box);
  std::vector<AABB> getTileAABB(AABB box);

  void calculateSpawnPosition();
  void calculateLightDepths(int x, int z, int offsetX, int offsetZ);

  bool containsAnyLiquid(AABB box);
  bool containsLiquid(AABB box, Block::Type blockType);

  AABBPosition clip(glm::vec3 start, glm::vec3 end, const glm::ivec3* expected = nullptr);

  int groundLevel;
  int waterLevel;

  glm::vec3 spawn;
  std::unique_ptr<unsigned char[]> blocks;

private:
  struct Update
  { 
    int x, y, z;
  };

  std::queue<Update> updates;
  std::unique_ptr<int[]> lightDepths;
};