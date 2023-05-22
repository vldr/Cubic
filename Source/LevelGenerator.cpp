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

void LevelGenerator::init(Game* game, int size)
{
	this->game = game;
	this->game->level.init(game, 128 << size, 128 << size);
}

void LevelGenerator::render(const char* title, const char* description)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniform1f(game->fogEnableUniform, 1.0f);
	glUniformMatrix4fv(game->viewMatrixUniform, 1, GL_FALSE, glm::value_ptr(game->identityMatrix));
	glUniformMatrix4fv(game->projectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(game->orthographicProjectionMatrix));

	game->ui.openStatusMenu(title, description);
	game->ui.render();

	SDL_GL_SwapWindow(game->window);
}

void LevelGenerator::generateHeights(Noise* noise1, Noise* noise2, Noise* noise3)
{
	render("Generating World", "Generating height-map...");

	for (int z = 0; z < game->level.depth; z++)
	{
		for (int x = 0; x < game->level.width; x++)
		{
			float noise1Value = noise1->compute(x * 1.3f, z * 1.3f) / 6.0f - 4.0f;
			float noise2Value = noise2->compute(x * 1.3f, z * 1.3f) / 5.0f + 6.0f;
			if (noise3->compute((float)x, (float)z) / 8.0f > 0.0f) { noise2Value = noise1Value; }

			float maxValue = glm::max(noise1Value, noise2Value) / 2.0f;
			heights[x + z * game->level.width] = (int)maxValue;
		} 
	}

	for (int z = 0; z < game->level.depth; z++)
	{
		for (int x = 0; x < game->level.width; x++)
		{
			float noise1Value = noise1->compute(x * 2.0f, z * 2.0f) / 8.0f;
			int noise2Value = noise2->compute(x * 2.0f, z * 2.0f) > 0.0f ? 1 : 0;

			if (noise1Value > 2.0f)
			{
				heights[x + z * game->level.width] = ((heights[x + z * game->level.width] - noise2Value) / 2 << 1) + noise2Value;
			}
		}
	}
}

void LevelGenerator::generateBase(Noise* noise3)
{
	render("Generating World", "Generating dirt, stone, and lava blocks...");

	for (int z = 0; z < game->level.depth; z++)
	{ 
		for (int x = 0; x < game->level.width; x++)
		{
			int noise3Value = (int)(noise3->compute((float)x, (float)z) / 24.0f) - 4;
			int heightValue = heights[x + z * game->level.width] + game->level.waterLevel;
			int combinedValue = heightValue + noise3Value;

			heights[x + z * game->level.width] = heightValue > combinedValue ? heightValue : combinedValue;
			if (heights[x + z * game->level.width] > game->level.height - 2) { heights[x + z * game->level.width] = game->level.height - 2; }
			if (heights[x + z * game->level.width] < 1) { heights[x + z * game->level.width] = 1; }

			for (auto height = 0; height < game->level.height; height++)
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
	render("Generating World", "Generating water blocks...");

	for (int z = 0; z < game->level.depth; z++)
	{
		for (int x = 0; x < game->level.width; x++)
		{
			int heightValue = heights[x + z * game->level.width];

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
	render("Generating World", "Generating caves...");

	int size = (game->level.width * game->level.depth * game->level.height) / 256 / 64 << 1;

	for (int i = 0; i < size; i++) 
	{
		int numberOfSteps = (int)((random->uniform() + random->uniform()) * 200.0f);

		float startX = (float)(random->uniform() * game->level.width);
		float startY = (float)(random->uniform() * game->level.height);
		float startZ = (float)(random->uniform() * game->level.depth);

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

				float radius = (game->level.height - currentY) / game->level.height * 2;
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

							if (distanceX * distanceX + 2.0 * distanceY * distanceY + distanceZ * distanceZ < radius * radius && blockX >= 1 && blockY >= 1 && blockZ >= 1 && blockX < game->level.width - 1 && blockY < game->level.height - 1 && blockZ < game->level.depth - 1) {
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
										if (blockY < 15)
										{
											game->level.setTile(blockX, blockY, blockZ, (unsigned char)Block::Type::BLOCK_LAVA);
										}
										else
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
}

void LevelGenerator::generateOre(Block::Type blockType, int amount)
{
	render("Generating World", "Generating ore blocks...");

	int size = game->level.width * game->level.depth * game->level.height / 256 / 64 * amount / 100;

	for (int i = 0; i < size; i++) 
	{
		int numberOfSteps = (int)((random->uniform() + random->uniform()) * 75.0 * amount / 100.0);

		float startX = (float)(random->uniform() * game->level.width);
		float startY = (float)(random->uniform() * game->level.height);
		float startZ = (float)(random->uniform() * game->level.depth);

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

						if (distanceX * distanceX + 2.0 * distanceY * distanceY + distanceZ * distanceZ < radius * radius && blockX >= 1 && blockY >= 1 && blockZ >= 1 && blockX < game->level.width - 1 && blockY < game->level.height - 1 && blockZ < game->level.depth - 1)
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

void LevelGenerator::generateGrassSandGravel(Noise* noise1, Noise* noise2)
{
	render("Generating World", "Generating gravel and sand blocks...");

	for (int z = 0; z < game->level.depth; z++)
	{
		for (int x = 0; x < game->level.width; x++)
		{ 
			int height = heights[x + z * game->level.width];

			bool isNoise1 = noise1->compute((float)x, (float)z) > 8.0f;
			bool isNoise2 = noise2->compute((float)x, (float)z) > 12.0f;

			auto blockAbove = game->level.getTile(x, height + 1, z);

			if (blockAbove == (unsigned char)Block::Type::BLOCK_WATER && height <= (game->level.height / 2) - 1 && isNoise2) 
			{
				game->level.setTile(x, height, z, (unsigned char)Block::Type::BLOCK_GRAVEL);
			}

			if (blockAbove == (unsigned char)Block::Type::BLOCK_AIR) 
			{
				if (height <= (game->level.height / 2) - 1 && isNoise1) 
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
	render("Generating World", "Generating flower blocks...");

	int size = game->level.width * game->level.depth / 3000;

	for (int i = 0; i < size; i++) 
	{
		int xCoord = (int)random->integerRange(0, game->level.width - 1);
		int zCoord = (int)random->integerRange(0, game->level.depth - 1);
		int flowerType = (int)random->integerRange(0, 1);

		for (int j = 0; j < 10; j++) 
		{
			int currXCoord = xCoord;
			int currZCoord = zCoord;

			for (int innerInnerIndex = 0; innerInnerIndex < 5; innerInnerIndex++) 
			{
				currXCoord += (int)random->integerRange(0, 5) - (int)random->integerRange(0, 5);
				currZCoord += (int)random->integerRange(0, 5) - (int)random->integerRange(0, 5);

				if ((flowerType < 2 || random->integerRange(0, 3) == 0) && currXCoord >= 0 && currZCoord >= 0 && currXCoord < game->level.width && currZCoord < game->level.depth) 
				{
					int yCoord = heights[currXCoord + currZCoord * game->level.width];

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
	render("Generating World", "Generating mushroom blocks...");

	int size = game->level.width * game->level.depth * game->level.height / 2000;

	for (int i = 0; i < size; i++) 
	{
		int mushroomType = (int)random->integerRange(0, 1);
		int blockX = (int)random->integerRange(0, game->level.width - 1);
		int blockY = (int)random->integerRange(0, game->level.height - 1);
		int blockZ = (int)random->integerRange(0, game->level.depth - 1);

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

				if ((mushroomType < 2 || random->integerRange(0, 3) == 0) && currentX >= 0 && currentZ >= 0 && currentY >= 1 && currentX < game->level.width && currentZ < game->level.depth && currentY < heights[currentX + currentZ * game->level.width] - 1) 
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
	render("Generating World", "Generating tree blocks...");

	for (int z = 4; z < game->level.depth - 4; z += 5)
	{
		for (int x = 4; x < game->level.width - 4; x += 5)
		{
			int treeHeight = heights[x + z * game->level.width];

			if (random->integerRange(0, 4) == 0)
			{
				int treeTrunkSize = (int)random->integerRange(0, 2) + 5;

				if (game->level.getTile(x, treeHeight, z) == (unsigned char)Block::Type::BLOCK_GRASS && treeHeight < game->level.depth - treeTrunkSize - 1)
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

void LevelGenerator::generateSpawnPosition()
{
	render("Generating World", "Finding a spawn position...");

	int maxX = -1;
	int maxY = -1;
	int maxZ = -1;

	for (int z = 0; z < game->level.depth; z++)
	{
		for (int x = 0; x < game->level.width; x++)
		{
			int y = heights[x + z * game->level.width];

			if (
				y > maxY && 
				game->level.getTile(x, y + 1, z) == (unsigned char)Block::Type::BLOCK_AIR &&
				game->level.getTile(x, y + 2, z) == (unsigned char)Block::Type::BLOCK_AIR 
			)
			{
				maxX = x;
				maxY = y;
				maxZ = z;
			}
		}
	}

	if (maxX != -1 && maxY != -1 && maxZ != -1)
	{
		game->level.spawn.x = maxX + 0.5f;
		game->level.spawn.y = maxY + 2.0f;
		game->level.spawn.z = maxZ + 0.5f;
	}
}

void LevelGenerator::generate()
{
	heights = new int[game->level.width * game->level.depth];

	random = new Random();
	random->init(std::time(nullptr));

	OctaveNoise oct[] = 
	{ 
		OctaveNoise(random, 8), 
		OctaveNoise(random, 8),
		OctaveNoise(random, 8),
		OctaveNoise(random, 8)
	};

	auto noise1 = CombinedNoise(&oct[0], &oct[1]);
	auto noise2 = CombinedNoise(&oct[2], &oct[3]);
	auto noise3 = OctaveNoise(random, 6);

	generateHeights(&noise1, &noise2, &noise3);
	generateBase(&noise3);
	generateWater();
	generateCaves();
	generateOre(Block::Type::BLOCK_COAL_ORE, 90);
	generateOre(Block::Type::BLOCK_IRON_ORE, 70);
	generateOre(Block::Type::BLOCK_GOLD_ORE, 50);
	generateGrassSandGravel(&noise1, &noise2);
	generateFlowers();
	generateMushrooms();
	generateTrees();
	generateSpawnPosition();

	game->level.calculateLightDepths(0, 0, game->level.width, game->level.depth);
	game->localPlayer.setPosition(game->level.spawn.x, game->level.spawn.y, game->level.spawn.z);

	game->level.spawn = game->localPlayer.position;

	delete[] heights;
}
