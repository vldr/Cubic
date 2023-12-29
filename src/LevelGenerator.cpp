#include "LevelGenerator.h"
#include "Game.h"
#include "Level.h"
#include "Block.h"
#include "Noise.h"
#include "CombinedNoise.h"
#include "OctaveNoise.h"
#include "Random.h"

#include <ctime>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

void LevelGenerator::init(Game* game)
{
	this->game = game;
	this->game->level.init(game);

	this->state = State::Init;
}

void LevelGenerator::generateHeightMap()
{
	for (int z = 0; z < Level::DEPTH; z++)
	{
		for (int x = 0; x < Level::WIDTH; x++)
		{
			float noise1Value = noise1->compute(x * 1.3f, z * 1.3f) / 6.0f - 4.0f;
			float noise2Value = noise2->compute(x * 1.3f, z * 1.3f) / 5.0f + 6.0f;
			if (noise3->compute((float)x, (float)z) / 8.0f > 0.0f) { noise2Value = noise1Value; }

			float maxValue = glm::max(noise1Value, noise2Value) / 2.0f;
			heights[x + z * Level::WIDTH] = (int)maxValue;
		} 
	}

	for (int z = 0; z < Level::DEPTH; z++)
	{
		for (int x = 0; x < Level::WIDTH; x++)
		{
			float noise1Value = noise1->compute(x * 2.0f, z * 2.0f) / 8.0f;
			int noise2Value = noise2->compute(x * 2.0f, z * 2.0f) > 0.0f ? 1 : 0;

			if (noise1Value > 2.0f)
			{
				heights[x + z * Level::WIDTH] = ((heights[x + z * Level::WIDTH] - noise2Value) / 2 << 1) + noise2Value;
			}
		}
	}
}

void LevelGenerator::generateDirtStoneLava()
{
	for (int z = 0; z < Level::DEPTH; z++)
	{ 
		for (int x = 0; x < Level::WIDTH; x++)
		{
			int noise3Value = (int)(noise3->compute((float)x, (float)z) / 24.0f) - 4;
			int heightValue = heights[x + z * Level::WIDTH] + game->level.waterLevel;
			int combinedValue = heightValue + noise3Value;

			heights[x + z * Level::WIDTH] = heightValue > combinedValue ? heightValue : combinedValue;
			if (heights[x + z * Level::WIDTH] > Level::HEIGHT - 2) { heights[x + z * Level::WIDTH] = Level::HEIGHT - 2; }
			if (heights[x + z * Level::WIDTH] < 1) { heights[x + z * Level::WIDTH] = 1; }

			for (auto height = 0; height < Level::HEIGHT; height++)
			{
				auto tile = Block::Type::BLOCK_AIR;

				if (height <= heightValue) { tile = Block::Type::BLOCK_DIRT; }
				if (height <= combinedValue) { tile = Block::Type::BLOCK_STONE; }
				if (height == 0) { tile = Block::Type::BLOCK_LAVA; }

				game->level.setTile(x, height, z, (unsigned char)tile);
			}
		}
	}
}

void LevelGenerator::generateWater()
{
	for (int z = 0; z < Level::DEPTH; z++)
	{
		for (int x = 0; x < Level::WIDTH; x++)
		{
			int heightValue = heights[x + z * Level::WIDTH];

			for (auto height = heightValue; height < game->level.waterLevel; height++)
			{
				if (game->level.getTile(x, height, z) == (unsigned char)Block::Type::BLOCK_AIR)
				{
					game->level.setTile(x, height, z, (unsigned char)Block::Type::BLOCK_WATER);
				}
			}
		}
	}
}

void LevelGenerator::generateCaves()
{
	int size = (Level::WIDTH * Level::DEPTH * Level::HEIGHT) / 256 / 64 << 1;

	for (int i = 0; i < size; i++) 
	{
		int numberOfSteps = (int)((random->uniform() + random->uniform()) * 200.0f);

		float startX = (float)(random->uniform() * Level::WIDTH);
		float startY = (float)(random->uniform() * Level::HEIGHT);
		float startZ = (float)(random->uniform() * Level::DEPTH);

		float angleX = (float)(random->uniform() * 2.0 * M_PI);
		float angleY = (float)(random->uniform() * 2.0 * M_PI);
		float angleYOffset = 0.0f;

		float randomFactor = (float)(random->uniform() * random->uniform());

		for (int j = 0; j < numberOfSteps; j++) 
		{
			startX += glm::sin(angleX) * glm::cos(angleY);
			startY += glm::sin(angleY);
			startZ += glm::cos(angleX) * glm::cos(angleY);

			angleX = (angleX + angleX * 0.2f) * 0.9f;
			angleY = (angleY + angleYOffset * 0.5f) * 0.5f;
			angleYOffset = angleYOffset * 0.75f + (float)(random->uniform() - random->uniform());

			if (random->uniform() >= 0.25) 
			{
				float currentX = startX + (float)((random->uniform() * 4.0 - 2.0) * 0.2);
				float currentY = startY + (float)((random->uniform() * 4.0 - 2.0) * 0.2);
				float currentZ = startZ + (float)((random->uniform() * 4.0 - 2.0) * 0.2);

				float radius = (Level::HEIGHT - currentY) / Level::HEIGHT * 2;
				radius = 1.2f + (radius * 3.5f + 1.0f) * randomFactor;
				radius *= glm::sin(j * (float)M_PI / numberOfSteps);

				for (int blockX = (int)(currentX - radius); blockX <= (int)(currentX + radius); blockX++) 
				{
					for (int blockY = (int)(currentY - radius); blockY <= (int)(currentY + radius); blockY++)
					{
						for (int blockZ = (int)(currentZ - radius); blockZ <= (int)(currentZ + radius); blockZ++)
						{
							float distanceX = blockX - currentX;
							float distanceY = blockY - currentY;
							float distanceZ = blockZ - currentZ;

							if (distanceX * distanceX + 2.0 * distanceY * distanceY + distanceZ * distanceZ < radius * radius && blockX >= 1 && blockY >= 1 && blockZ >= 1 && blockX < Level::WIDTH - 1 && blockY < Level::HEIGHT - 1 && blockZ < Level::DEPTH - 1) {
								if (
									game->level.getTile(blockX, blockY + 1, blockZ) != (unsigned char)Block::Type::BLOCK_WATER &&
									game->level.getTile(blockX, blockY - 1, blockZ) != (unsigned char)Block::Type::BLOCK_WATER &&
									game->level.getTile(blockX + 1, blockY, blockZ) != (unsigned char)Block::Type::BLOCK_WATER &&
									game->level.getTile(blockX - 1, blockY, blockZ) != (unsigned char)Block::Type::BLOCK_WATER &&
									game->level.getTile(blockX, blockY, blockZ + 1) != (unsigned char)Block::Type::BLOCK_WATER &&
									game->level.getTile(blockX, blockY, blockZ - 1) != (unsigned char)Block::Type::BLOCK_WATER
								)
								{
									if (
										game->level.getTile(blockX, blockY, blockZ) == (unsigned char)Block::Type::BLOCK_STONE
									)
									{
										game->level.setTile(blockX, blockY, blockZ, (unsigned char)Block::Type::BLOCK_AIR);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void LevelGenerator::generateOre(Block::Type blockType, int amount)
{
	int size = Level::WIDTH * Level::DEPTH * Level::HEIGHT / 256 / 64 * amount / 100;

	for (int i = 0; i < size; i++) 
	{
		int numberOfSteps = (int)((random->uniform() + random->uniform()) * 75.0 * amount / 100.0);

		float startX = (float)(random->uniform() * Level::WIDTH);
		float startY = (float)(random->uniform() * Level::HEIGHT);
		float startZ = (float)(random->uniform() * Level::DEPTH);

		float angleX = (float)(random->uniform() * 2.0 * M_PI);
		float angleY = (float)(random->uniform() * 2.0 * M_PI);
		float angleXOffset = 0.0;
		float angleYOffset = 0.0;

		for (int j = 0; j < numberOfSteps; j++) 
		{ 
			startX += glm::sin(angleX) * glm::cos(angleY);
			startY += glm::sin(angleY);
			startZ += glm::cos(angleX) * glm::cos(angleY);
			angleX += angleXOffset * 0.2f;
			angleXOffset = angleXOffset * 0.9f + (float)(random->uniform() - random->uniform());
			angleY = (angleY + angleYOffset * 0.5f) * 0.5f;
			angleYOffset = angleYOffset * 0.9f + (float)(random->uniform() - random->uniform());

			float radius = glm::sin(j * (float)M_PI / numberOfSteps) * amount / 100.0f + 1.0f;

			for (int blockX = (int)(startX - radius); blockX <= (int)(startX + radius); blockX++) 
			{
				for (int blockY = (int)(startY - radius); blockY <= (int)(startY + radius); blockY++)
				{
					for (int blockZ = (int)(startZ - radius); blockZ <= (int)(startZ + radius); blockZ++)
					{
						float distanceX = blockX - startX;
						float distanceY = blockY - startY;
						float distanceZ = blockZ - startZ;

						if (distanceX * distanceX + 2.0 * distanceY * distanceY + distanceZ * distanceZ < radius * radius && blockX >= 1 && blockY >= 1 && blockZ >= 1 && blockX < Level::WIDTH - 1 && blockY < Level::HEIGHT - 1 && blockZ < Level::DEPTH - 1)
						{
							if (game->level.getTile(blockX, blockY, blockZ) == (unsigned char)Block::Type::BLOCK_STONE)
							{
								game->level.setTile(blockX, blockY, blockZ, (unsigned char)blockType);
							}
						}
					}
				}
			}
		}
	}
}

void LevelGenerator::generateGrassSandGravel()
{
	for (int z = 0; z < Level::DEPTH; z++)
	{
		for (int x = 0; x < Level::WIDTH; x++)
		{ 
			int height = heights[x + z * Level::WIDTH];

			bool isNoise1 = noise1->compute((float)x, (float)z) > 8.0f;
			bool isNoise2 = noise2->compute((float)x, (float)z) > 12.0f;

			auto blockAbove = game->level.getTile(x, height + 1, z);

			if (blockAbove == (unsigned char)Block::Type::BLOCK_WATER && height <= (Level::HEIGHT / 2) - 1 && isNoise2) 
			{
				game->level.setTile(x, height, z, (unsigned char)Block::Type::BLOCK_GRAVEL);
			}

			if (blockAbove == (unsigned char)Block::Type::BLOCK_AIR) 
			{
				if (height <= (Level::HEIGHT / 2) - 1 && isNoise1) 
				{
					game->level.setTile(x, height, z, (unsigned char)Block::Type::BLOCK_SAND);
				}
				else 
				{
					game->level.setTile(x, height, z, (unsigned char)Block::Type::BLOCK_GRASS);
				}
			}
		}
	}
}

void LevelGenerator::generateFlowers()
{
	int size = Level::WIDTH * Level::DEPTH / 3000;

	for (int i = 0; i < size; i++) 
	{
		int xCoord = (int)random->integerRange(0, Level::WIDTH - 1);
		int zCoord = (int)random->integerRange(0, Level::DEPTH - 1);
		int flowerType = (int)random->integerRange(0, 1);

		for (int j = 0; j < 10; j++) 
		{
			int currXCoord = xCoord;
			int currZCoord = zCoord;

			for (int innerInnerIndex = 0; innerInnerIndex < 5; innerInnerIndex++) 
			{
				currXCoord += (int)random->integerRange(0, 5) - (int)random->integerRange(0, 5);
				currZCoord += (int)random->integerRange(0, 5) - (int)random->integerRange(0, 5);

				if ((flowerType < 2 || random->integerRange(0, 3) == 0) && currXCoord >= 0 && currZCoord >= 0 && currXCoord < Level::WIDTH && currZCoord < Level::DEPTH) 
				{
					int yCoord = heights[currXCoord + currZCoord * Level::WIDTH];

					if (game->level.getTile(currXCoord, yCoord, currZCoord) == (unsigned char)Block::Type::BLOCK_GRASS) 
					{
						if (flowerType == 0) { game->level.setTile(currXCoord, yCoord + 1, currZCoord, (unsigned char)Block::Type::BLOCK_DANDELION); }
						else if (flowerType == 1) { game->level.setTile(currXCoord, yCoord + 1, currZCoord, (unsigned char)Block::Type::BLOCK_ROSE); }
					}
				}
			}
		}
	}
}

void LevelGenerator::generateMushrooms()
{
	int size = Level::WIDTH * Level::DEPTH * Level::HEIGHT / 2000;

	for (int i = 0; i < size; i++) 
	{
		int mushroomType = (int)random->integerRange(0, 1);
		int blockX = (int)random->integerRange(0, Level::WIDTH - 1);
		int blockY = (int)random->integerRange(0, Level::HEIGHT - 1);
		int blockZ = (int)random->integerRange(0, Level::DEPTH - 1);

		for (int j = 0; j < 20; j++)
		{
			int currentX = blockX;
			int currentY = blockY;
			int currentZ = blockZ;

			for (int directionChangeIndex = 0; directionChangeIndex < 5; directionChangeIndex++)
			{
				currentX += (int)random->integerRange(0, 5) - (int)random->integerRange(0, 5);
				currentY += (int)random->integerRange(0, 1) - (int)random->integerRange(0, 1);
				currentZ += (int)random->integerRange(0, 5) - (int)random->integerRange(0, 5);

				if ((mushroomType < 2 || random->integerRange(0, 3) == 0) && currentX >= 0 && currentZ >= 0 && currentY >= 1 && currentX < Level::WIDTH && currentZ < Level::DEPTH && currentY < heights[currentX + currentZ * Level::WIDTH] - 1) 
				{
					if (game->level.getTile(currentX, blockY, currentZ) == (unsigned char)Block::Type::BLOCK_AIR) {
						if (game->level.getTile(currentX, blockY - 1, currentZ) == (unsigned char)Block::Type::BLOCK_STONE) {
							if (mushroomType == 0) { game->level.setTile(currentX, blockY, currentZ, (unsigned char)Block::Type::BLOCK_BROWN_SHROOM); }
							else { game->level.setTile(currentX, blockY, currentZ, (unsigned char)Block::Type::BLOCK_RED_SHROOM); }
						}
					}
				}
			}
		}
	}
}

void LevelGenerator::generateTrees()
{
	for (int z = 4; z < Level::DEPTH - 4; z += 5)
	{
		for (int x = 4; x < Level::WIDTH - 4; x += 5)
		{
			int treeHeight = heights[x + z * Level::WIDTH];

			if (random->integerRange(0, 4) == 0)
			{
				int treeTrunkSize = (int)random->integerRange(0, 2) + 5;

				if (game->level.getTile(x, treeHeight, z) == (unsigned char)Block::Type::BLOCK_GRASS && treeHeight < Level::DEPTH - treeTrunkSize - 1)
				{
					game->level.setTile(x, treeHeight, z, (unsigned char)Block::Type::BLOCK_DIRT);

					for (int treeLeavesLevel = treeHeight - 3 + treeTrunkSize; treeLeavesLevel <= treeHeight + treeTrunkSize; ++treeLeavesLevel) 
					{
						int treeLeavesDistanceFromTop = treeLeavesLevel - (treeHeight + treeTrunkSize);
						int treeLeavesWidth = 1 - treeLeavesDistanceFromTop / 2;

						for (int treeLeavesX = x - treeLeavesWidth; treeLeavesX <= x + treeLeavesWidth; ++treeLeavesX) 
						{
							int xDistanceFromBase = treeLeavesX - x;

							for (int treeLeavesZ = z - treeLeavesWidth; treeLeavesZ <= z + treeLeavesWidth; ++treeLeavesZ) 
							{
								int zDistanceFromBase = treeLeavesZ - z;
								if (abs(xDistanceFromBase) != treeLeavesWidth || abs(zDistanceFromBase) != treeLeavesWidth || (random->integerRange(0, 1) != 0 && treeLeavesDistanceFromTop != 0)) 
								{
									game->level.setTile(treeLeavesX, treeLeavesLevel, treeLeavesZ, (unsigned char)Block::Type::BLOCK_LEAVES);
								}
							}
						}
					}

					for (int treeTrunkLevel = 0; treeTrunkLevel < treeTrunkSize; treeTrunkLevel++)
					{
						game->level.setTile(x, treeHeight + treeTrunkLevel, z, (unsigned char)Block::Type::BLOCK_LOG);
					}
				}
			}
		}
	}
}

void LevelGenerator::update()
{
	switch (state)
	{
	case State::Init:
		game->ui.openStatusMenu("Generating World", "Generating height map...");

		heights = std::make_unique<int[]>(Level::WIDTH * Level::DEPTH);

		random = std::make_unique<Random>();
		random->init(std::time(nullptr));

		noise1 = std::make_unique<CombinedNoise>(std::make_unique<OctaveNoise>(*random, 8), std::make_unique<OctaveNoise>(*random, 8));
		noise2 = std::make_unique<CombinedNoise>(std::make_unique<OctaveNoise>(*random, 8), std::make_unique<OctaveNoise>(*random, 8));
		noise3 = std::make_unique<OctaveNoise>(*random, 6);

		state = State::HeightMap;
		break;
	case State::HeightMap:
		game->ui.openStatusMenu("Generating World", "Generating dirt, stone and lava...");

		generateHeightMap();

		state = State::DirtStoneLava;
		break;
	case State::DirtStoneLava:
		game->ui.openStatusMenu("Generating World", "Generating water...");

		generateDirtStoneLava();

		state = State::Water;
		break;
	case State::Water:
		game->ui.openStatusMenu("Generating World", "Generating caves...");

		generateWater();

		state = State::Caves;
		break;
	case State::Caves:
		game->ui.openStatusMenu("Generating World", "Generating ores...");

		generateCaves();

		state = State::Ore;
		break;
	case State::Ore:
		game->ui.openStatusMenu("Generating World", "Generating grass, sand and gravel...");

		generateOre(Block::Type::BLOCK_COAL_ORE, 90);
		generateOre(Block::Type::BLOCK_IRON_ORE, 70);
		generateOre(Block::Type::BLOCK_GOLD_ORE, 50);

		state = State::GrassSandGravel;
		break;
	case State::GrassSandGravel:
		game->ui.openStatusMenu("Generating World", "Generating flowers...");

		generateGrassSandGravel();

		state = State::Flowers;
		break;
	case State::Flowers:
		game->ui.openStatusMenu("Generating World", "Generating mushrooms...");

		generateFlowers();

		state = State::Mushrooms;
		break;
	case State::Mushrooms:
		game->ui.openStatusMenu("Generating World", "Generating trees...");

		generateMushrooms();

		state = State::Trees;
		break;
	case State::Trees:
		game->ui.openStatusMenu("Generating World", "Generating light depths...");

		generateTrees();

		state = State::Destroy;
		break;
	case State::Destroy:
		game->level.calculateLightDepths(0, 0, Level::WIDTH, Level::DEPTH);
		game->level.calculateSpawnPosition();
		game->level.reset();

		game->levelRenderer.initChunks();
		game->network.connect();

		heights.reset();
		random.reset();
		noise1.reset();
		noise2.reset();
		noise3.reset();

		state = State::Finished;
		break;
	case State::Finished:
		return;
	}
}