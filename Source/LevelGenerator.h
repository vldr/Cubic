#pragma once
#include "Block.h"

class Game;
class Level;
class Noise;
class Random;
class CombinedNoise;
class OctaveNoise;

class LevelGenerator
{
public:
	void init(Game* game, int size = 0);
	void update();

private:
	enum class State {
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

	void generateHeightMap(Noise* noise1, Noise* noise2, Noise* noise3);
	void generateDirtStoneLava(Noise* noise3);
	void generateWater();
	void generateCaves();
	void generateOre(Block::Type blockType, int amount);
	void generateGrassSandGravel(Noise* noise1, Noise* noise2);
	void generateFlowers();
	void generateMushrooms();
	void generateTrees();

	Game* game;
	State state;

	int* heights;
	Random* random;
	OctaveNoise* octaves[4];
	CombinedNoise* noise1;
	CombinedNoise* noise2;
	OctaveNoise* noise3;
};

