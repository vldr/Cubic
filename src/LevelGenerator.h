#pragma once
#include "Block.h"
#include "OctaveNoise.h"
#include "CombinedNoise.h"

#include <memory>

class Level;
class Random;

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

	std::unique_ptr<int[]> heights;
	std::unique_ptr<Random> random;
	std::shared_ptr<CombinedNoise> noise1;
	std::shared_ptr<CombinedNoise> noise2;
	std::shared_ptr<OctaveNoise> noise3;
};
