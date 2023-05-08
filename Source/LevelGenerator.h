#pragma once
#include "Block.h"

class Game;
class Level;
class Noise;
class Random;

class LevelGenerator
{
public:
	void init(Game* game, int size = 0);
	void generate();

private:
	void generateHeights(Noise* noise1, Noise* noise2, Noise* noise3);
	void generateBase(Noise* noise3);
	void generateWater();
	void generateCaves();
	void generateOre(Block::Type blockType, int amount);
	void generateGrassSandGravel(Noise* noise1, Noise* noise2);
	void generateFlowers();
	void generateMushrooms();
	void generateTrees();
	void generateSpawnPosition();

	int* heights;
	Game* game;
	Random* random;
};

