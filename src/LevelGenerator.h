#pragma once
#include "Block.h"
#include "OctaveNoise.h"
#include "CombinedNoise.h"
#include "Level.h"
#include "Random.h"

#include <cstdint>
#include <ctime>

class LevelGenerator
{
public:
  void init();
  void update();

private:
  enum class State
  {
    Init,
    HeightMap,
    DirtStoneLava,
    Water,
    Caves,
    Ore,
    GrassSandGravel,
    Flowers,
    Mushrooms,
    Trees,
    Destroy,
    Finished,
  };

  void generateHeightMap();
  void generateDirtStoneLava();
  void generateWater();
  void generateCaves();
  void generateOre(Block::Type blockType, int amount);
  void generateGrassSandGravel();
  void generateFlowers();
  void generateMushrooms();
  void generateTrees();

  State state;
  int heights[Level::WIDTH * Level::DEPTH];

  Random random = { uint64_t(std::time(nullptr)) };
  CombinedNoise noise1 = { OctaveNoise(random, 8), OctaveNoise(random, 8) };
  CombinedNoise noise2 = { OctaveNoise(random, 8), OctaveNoise(random, 8) };
  OctaveNoise noise3 = { random, 6 };
};
