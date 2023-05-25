#include "Level.h"
#include "AABB.h"
#include "AABBPosition.h"
#include "Game.h"

#include <glm/glm.hpp>
#include <random>

void Level::init(Game* game, int width, int depth)
{
	this->game = game;
	this->width = width;
	this->height = 64;
	this->depth = depth;
	this->waterLevel = height / 2;
	this->groundLevel = this->waterLevel - 2;
	this->blocks = new unsigned char[width * height * depth]();

	this->spawn.x = this->width - 1.0f;
	this->spawn.x = this->height - 1.0f;
	this->spawn.x = this->depth - 1.0f;

	this->lightDepths = new int[width * depth];
	for (int i = 0; i < width * depth; i++)
	{
		this->lightDepths[i] = this->height - 1;
	}
}

void Level::tick()
{
	if (game->timer.ticks % 7 == 0)
	{
		auto liquidUpdatesSize = liquidUpdates.size();

		for (auto i = 0; i < liquidUpdatesSize; i++)
		{
			auto tile = liquidUpdates.front();

			if (canFlood(tile.x - 1, tile.y, tile.z))
			{
				setTileWithNeighborChange(tile.x - 1, tile.y, tile.z, tile.blockType);
			}

			if (canFlood(tile.x + 1, tile.y, tile.z))
			{
				setTileWithNeighborChange(tile.x + 1, tile.y, tile.z, tile.blockType);
			}

			if (canFlood(tile.x, tile.y - 1, tile.z))
			{
				setTileWithNeighborChange(tile.x, tile.y - 1, tile.z, tile.blockType);
			}

			if (canFlood(tile.x, tile.y, tile.z - 1))
			{
				setTileWithNeighborChange(tile.x, tile.y, tile.z - 1, tile.blockType);
			}

			if (canFlood(tile.x, tile.y, tile.z + 1))
			{
				setTileWithNeighborChange(tile.x, tile.y, tile.z + 1, tile.blockType);
			}

			liquidUpdates.pop();
		}
	}
}

bool Level::canFlood(int x, int y, int z)
{
	for (int i = x - 2; i <= x + 2; i++)
	{
		for (int j = y - 2; j <= y + 2; j++)
		{
			for (int k = z - 2; k <= z + 2; k++)
			{
				if (getTile(i, j, k) == (unsigned char)Block::Type::BLOCK_SPONGE)
				{
					return false;
				}
			}
		}
	}

	return (Block::Definitions[getTile(x, y, z)].collide == Block::CollideType::COLLIDE_NONE);
}

bool Level::isTileLit(int x, int y, int z)
{
	return !(x < 0 || y < 0 || z < 0 || x >= width || y >= height || z >= depth) ? y >= lightDepths[x + z * width] : true;
}

float Level::getTileBrightness(int x, int y, int z)
{
	if (isLavaTile(x, y, z) || isTileLit(x, y, z))
	{
		return 1.0f;
	}

	return 0.6f;
}

unsigned int Level::getTileAABBCount(AABB box)
{
	unsigned int tiles = 0;

	int x0 = (int)box.x0, y0 = (int)box.y0, z0 = (int)box.z0;
	int x1 = (int)(box.x1 + 1.0f), y1 = (int)(box.y1 + 1.0f), z1 = (int)(box.z1 + 1.0f);

	if (box.x0 < 0)
	{
		x0--;
	}

	if (box.y0 < 0)
	{
		y0--;
	}

	if (box.z0 < 0)
	{
		z0--;
	}

	for (int i = x0; i < x1; i++)
	{
		for (int j = y0; j < y1; j++)
		{
			for (int k = z0; k < z1; k++)
			{
				if (i >= 0 && j >= 0 && k >= 0 && i < width && j < height && k < depth)
				{
					Block::Definition blockDefinition = Block::Definitions[getTile(i, j, k)];

					if (blockDefinition.collide != Block::CollideType::COLLIDE_NONE && blockDefinition.collide != Block::CollideType::COLLIDE_LIQUID)
					{
						AABB aabb = {
							aabb.x0 = (float)i,
							aabb.y0 = (float)j,
							aabb.z0 = (float)k,
							aabb.x1 = (float)i + 1.0f,
							aabb.y1 = (float)j + blockDefinition.height,
							aabb.z1 = (float)k + 1.0f
						};

						if (box.intersectsInner(aabb))
						{
							tiles++;
						}
					}
				}
				else if (i < 0 || j < 0 || k < 0 || i >= width || k >= depth)
				{
					if (j < groundLevel)
					{
						Block::Definition blockDefinition = Block::Definitions[(unsigned char)Block::Type::BLOCK_BEDROCK];

						AABB aabb = {
							aabb.x0 = (float)i,
							aabb.y0 = (float)j,
							aabb.z0 = (float)k,
							aabb.x1 = (float)i + 1.0f,
							aabb.y1 = (float)j + blockDefinition.height,
							aabb.z1 = (float)k + 1.0f
						};

						if (box.intersectsInner(aabb))
						{
							tiles++;
						}
					}
				}
			}
		}
	}

	return tiles;
}

std::vector<AABB> Level::getTileAABB(AABB box)
{
	std::vector<AABB> tiles;

	int x0 = (int)box.x0, y0 = (int)box.y0, z0 = (int)box.z0;
	int x1 = (int)(box.x1 + 1.0f), y1 = (int)(box.y1 + 1.0f), z1 = (int)(box.z1 + 1.0f);

	if (box.x0 < 0)
	{
		x0--;
	}

	if (box.y0 < 0)
	{
		y0--;
	}

	if (box.z0 < 0)
	{
		z0--;
	}

	for (int i = x0; i < x1; i++)
	{
		for (int j = y0; j < y1; j++)
		{
			for (int k = z0; k < z1; k++)
			{
				if (i >= 0 && j >= 0 && k >= 0 && i < width && j < height && k < depth)
				{
					Block::Definition blockDefinition = Block::Definitions[getTile(i, j, k)];

					if (blockDefinition.collide != Block::CollideType::COLLIDE_NONE && blockDefinition.collide != Block::CollideType::COLLIDE_LIQUID)
					{
						AABB aabb = {
							aabb.x0 = (float)i,
							aabb.y0 = (float)j,
							aabb.z0 = (float)k,
							aabb.x1 = (float)i + 1.0f,
							aabb.y1 = (float)j + blockDefinition.height,
							aabb.z1 = (float)k + 1.0f
						};

						if (box.intersectsInner(aabb))
						{
							tiles.push_back(aabb);
						}
					}
				}
				else if (i < 0 || j < 0 || k < 0 || i >= width || k >= depth)
				{
					if (j < groundLevel)
					{
						Block::Definition blockDefinition = Block::Definitions[(unsigned char)Block::Type::BLOCK_BEDROCK];

						AABB aabb = {
							aabb.x0 = (float)i,
							aabb.y0 = (float)j,
							aabb.z0 = (float)k,
							aabb.x1 = (float)i + 1.0f,
							aabb.y1 = (float)j + blockDefinition.height,
							aabb.z1 = (float)k + 1.0f
						};

						if (box.intersectsInner(aabb))
						{
							tiles.push_back(aabb);
						}
					}
				}
			}
		}
	}

	return tiles;
}

bool Level::containsAnyLiquid(AABB box)
{
	int x0 = (int)box.x0, y0 = (int)box.y0, z0 = (int)box.z0;
	int x1 = (int)(box.x1 + 1), y1 = (int)(box.y1 + 1), z1 = (int)(box.z1 + 1);

	if (box.x0 < 0)
	{
		x0--;
	}

	if (box.y0 < 0)
	{
		y0--;
	}

	if (box.z0 < 0)
	{
		z0--;
	}

	for (int i = x0; i < x1; i++)
	{
		for (int j = y0; j < y1; j++)
		{
			for (int k = z0; k < z1; k++)
			{
				Block::Definition blockDefinition = Block::Definitions[(unsigned char)getTile(i, j, k)];

				if (i < 0 || j < 0 || k < 0 || i >= width || k >= depth)
				{
					if (j < waterLevel && j > groundLevel)
					{
						return true;
					}
				}
				else if (blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID)
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool Level::containsLiquid(AABB box, Block::Type blockType) 
{
	int x0 = (int)box.x0, y0 = (int)box.y0, z0 = (int)box.z0;
	int x1 = (int)(box.x1 + 1), y1 = (int)(box.y1 + 1), z1 = (int)(box.z1 + 1);

	if (box.x0 < 0)
	{
		x0--;
	}

	if (box.y0 < 0)
	{
		y0--;
	}

	if (box.z0 < 0)
	{
		z0--;
	}

	for (int i = x0; i < x1; i++)
	{
		for (int j = y0; j < y1; j++)
		{
			for (int k = z0; k < z1; k++)
			{
				if (i < 0 || j < 0 || k < 0 || i >= width || k >= depth)
				{
					if (
						j < waterLevel && 
						j > groundLevel && 
						(blockType == Block::Type::BLOCK_STILL_WATER || blockType == Block::Type::BLOCK_WATER)
					)
					{
						return true;
					}
				}
				else if (getTile(i, j, k) == (unsigned char)blockType)
				{
					return true;
				}
			}
		}
	}

	return false;
}

void Level::calculateSpawnPosition()
{
	glm::vec3 maxPosition = glm::vec3(-INFINITY, -INFINITY, -INFINITY);

	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < depth; z++)
		{
			int y = lightDepths[x + z * width];

			if (
				getTile(x, y + 1, z) == (unsigned char)Block::Type::BLOCK_AIR &&
				getTile(x, y + 2, z) == (unsigned char)Block::Type::BLOCK_AIR
			)
			{
				if (y > maxPosition.y)
				{
					maxPosition.x = float(x);
					maxPosition.y = float(y);
					maxPosition.z = float(z);
				}
			}
		}
	}

	game->level.spawn.x = maxPosition.x + 0.5f;
	game->level.spawn.y = maxPosition.y + 2.0f;
	game->level.spawn.z = maxPosition.z + 0.5f;

	game->localPlayer.setPosition(game->level.spawn.x, game->level.spawn.y, game->level.spawn.z);
	game->level.spawn = game->localPlayer.position;
}

void Level::calculateLightDepths(int x, int z, int offsetX, int offsetZ)
{
	for (int i = x; i < x + offsetX; i++)
	{
		for (int j = z; j < z + offsetZ; j++)
		{
			int blocker = lightDepths[i + j * width];
			int k = height - 1;

			while (
				!Block::Definitions[getTile(i, k, j)].blocksLight &&
				k > 0
			)
			{
				k--;
			}

			lightDepths[i + j * width] = k;

			if (blocker != k)
			{
				int min = blocker < k ? blocker : k;
				int max = blocker > k ? blocker : k;

				game->levelRenderer.loadChunks(i - 1, min - 1, j - 1, i + 1, max + 1, j + 1);
			}
		}
	}
}

unsigned char Level::getTile(int x, int y, int z)
{
	return isInBounds(x, y, z) ? blocks[(z * height + y) * width + x] : (unsigned char)Block::Type::BLOCK_AIR;
}

unsigned char Level::getRenderTile(int x, int y, int z)
{
	if (x < 0 || y < 0 || z < 0 || x >= width || z >= depth)
	{
		if (y < waterLevel && y >= groundLevel)
		{
			return (unsigned char)Block::Type::BLOCK_WATER;
		}
	}

	return isInBounds(x, y, z) ? blocks[(z * height + y) * width + x] : (unsigned char)Block::Type::BLOCK_AIR;
}

void Level::updateTile(int x, int y, int z, unsigned char tile)
{
	if (isInBounds(x, y, z))
	{
		auto blockType = getTile(x, y, z);

		if (isMovingWaterTile(x, y, z) || isMovingLavaTile(x, y, z))
		{
			if (
				Block::Definitions[getTile(x - 1, y, z)].collide == Block::CollideType::COLLIDE_NONE ||
				Block::Definitions[getTile(x + 1, y, z)].collide == Block::CollideType::COLLIDE_NONE ||
				Block::Definitions[getTile(x, y - 1, z)].collide == Block::CollideType::COLLIDE_NONE ||
				Block::Definitions[getTile(x, y, z - 1)].collide == Block::CollideType::COLLIDE_NONE ||
				Block::Definitions[getTile(x, y, z + 1)].collide == Block::CollideType::COLLIDE_NONE
			)
			{
				liquidUpdates.push({ x, y, z, blockType });
			}

			if (
				(isMovingWaterTile(x, y, z) && isMovingLavaTile(tile)) ||
				(isMovingLavaTile(x, y, z) && isMovingWaterTile(tile))
			)
			{
				setTileWithNeighborChange(x, y, z, (unsigned char)Block::Type::BLOCK_OBSIDIAN);
			}
		}
		else if (blockType == (unsigned char)Block::Type::BLOCK_SAND || blockType == (unsigned char)Block::Type::BLOCK_GRAVEL)
		{
			int height = 1;
			while (
				Block::Definitions[getTile(x, y - height, z)].collide != Block::CollideType::COLLIDE_SOLID &&
				isInBounds(x, y - height, z)
			)
			{
				height++;
			}

			if (height > 1)
			{
				setTileWithNoNeighborChange(x, y - height + 1, z, blockType);
				setTileWithNeighborChange(x, y, z, (unsigned char)Block::Type::BLOCK_AIR);
			}
		}
	}
}

bool Level::setTileWithNeighborChange(int x, int y, int z, unsigned char blockType)
{
	if (setTileWithNoNeighborChange(x, y, z, blockType))
	{
		updateTile(x - 1, y, z, blockType);
		updateTile(x + 1, y, z, blockType);
		updateTile(x, y - 1, z, blockType);
		updateTile(x, y + 1, z, blockType);
		updateTile(x, y, z - 1, blockType);
		updateTile(x, y, z + 1, blockType);
		updateTile(x, y, z, blockType);

		return true;
	}

	return false;
}

void Level::addedTile(int x, int y, int z, unsigned char tile)
{
	if (tile == (unsigned char)Block::Type::BLOCK_SPONGE)
	{
		for (int i = x - 2; i <= x + 2; i++)
		{
			for (int j = y - 2; j <= y + 2; j++)
			{
				for (int k = z - 2; k <= z + 2; k++)
				{
					if (isWaterTile(i, j, k))
					{
						setTileWithNoNeighborChange(i, j, k, (unsigned char)Block::Type::BLOCK_AIR);
					}
				}
			}
		}
	}
	else if (tile == (unsigned char)Block::Type::BLOCK_SLAB)
	{
		if (getTile(x, y - 1, z) == (unsigned char)Block::Type::BLOCK_SLAB)
		{
			setTileWithNeighborChange(x, y, z, (unsigned char)Block::Type::BLOCK_AIR);
			setTileWithNeighborChange(x, y - 1, z, (unsigned char)Block::Type::BLOCK_DOUBLE_SLAB);
		}
	}
}

void Level::removedTile(int x, int y, int z, unsigned char tile)
{
	if (tile == (unsigned char)Block::Type::BLOCK_SPONGE)
	{
		for (int i = x - 3; i <= x + 3; i++)
		{
			for (int j = y - 3; j <= y + 3; j++)
			{
				for (int k = z - 3; k <= z + 3; k++)
				{
					updateTile(i, j, k, getTile(i, j, k));
				}
			}
		}
	}
}

bool Level::setTileWithNoNeighborChange(int x, int y, int z, unsigned char tile)
{
	if (x < 0 || y < 0 || z < 0 || x >= width || y >= height || z >= depth)
	{
		return false;
	}

	auto previousTile = getTile(x, y, z);
	if (previousTile == tile)
	{
		return false;
	}

	if (
		(x == 0 || z == 0 || x == width - 1 || z == depth - 1) &&
		y >= groundLevel &&
		y < waterLevel &&
		tile == (unsigned char)Block::Type::BLOCK_AIR
	)
	{
		tile = (unsigned char)Block::Type::BLOCK_WATER;
	}

	setTileWithRender(x, y, z, tile);

	if (previousTile != (unsigned char)Block::Type::BLOCK_AIR) { removedTile(x, y, z, previousTile); }
	if (tile != (unsigned char)Block::Type::BLOCK_AIR) { addedTile(x, y, z, tile); }

	return true;
}

void Level::setTileWithRender(int x, int y, int z, unsigned char tile)
{
	auto previousTile = getTile(x, y, z);
	if (previousTile != tile)
	{
		setTile(x, y, z, tile);
		calculateLightDepths(x, z, 1, 1);

		game->levelRenderer.loadChunks(x - 1, y - 1, z - 1, x + 1, y + 1, z + 1);
	}
}

void Level::setTile(int x, int y, int z, unsigned char value)
{
	if (isInBounds(x, y, z))
	{
		blocks[(z * height + y) * width + x] = value;

		if (game->network.isConnected() && game->network.isHost())
		{
			game->network.setBlock(x, y, z, value);
		}
	}
}

bool Level::isInBounds(int x, int y, int z)
{
	return x >= 0 && y >= 0 && z >= 0 && x < width && y < height && z < depth;
}

bool Level::isAirTile(int x, int y, int z)
{
	const auto blockType = getTile(x, y, z);

	return blockType == (unsigned char)Block::Type::BLOCK_AIR;
}

bool Level::isAirTile(unsigned char blockType)
{
	return blockType == (unsigned char)Block::Type::BLOCK_AIR;
}

bool Level::isRenderWaterTile(float x, float y, float z)
{
	if (x < 0 || y < 0 || z < 0 || x >= width || z >= depth)
	{
		if (y < waterLevel && y > groundLevel)
		{
			return true;
		}
	}

	const auto blockType = getTile((int)x, (int)y, (int)z);

	return blockType == (unsigned char)Block::Type::BLOCK_WATER || blockType == (unsigned char)Block::Type::BLOCK_STILL_WATER;
}

bool Level::isWaterTile(int x, int y, int z)
{
	const auto blockType = getTile(x, y, z);

	return blockType == (unsigned char)Block::Type::BLOCK_WATER || blockType == (unsigned char)Block::Type::BLOCK_STILL_WATER;
}

bool Level::isWaterTile(unsigned char blockType)
{
	return blockType == (unsigned char)Block::Type::BLOCK_WATER || blockType == (unsigned char)Block::Type::BLOCK_STILL_WATER;
}

bool Level::isMovingWaterTile(int x, int y, int z)
{
	const auto blockType = getTile(x, y, z);

	return blockType == (unsigned char)Block::Type::BLOCK_WATER;
}

bool Level::isMovingWaterTile(unsigned char blockType)
{
	return blockType == (unsigned char)Block::Type::BLOCK_WATER;
}

bool Level::isLavaTile(float x, float y, float z)
{
	const auto blockType = getTile((int)x, (int)y, (int)z);

	return blockType == (unsigned char)Block::Type::BLOCK_LAVA || blockType == (unsigned char)Block::Type::BLOCK_STILL_LAVA;
}

bool Level::isMovingLavaTile(int x, int y, int z)
{
	const auto blockType = getTile(x, y, z);

	return blockType == (unsigned char)Block::Type::BLOCK_LAVA;
}

bool Level::isMovingLavaTile(unsigned char blockType)
{
	return blockType == (unsigned char)Block::Type::BLOCK_LAVA;
}

bool Level::isLavaTile(int x, int y, int z)
{
	const auto blockType = getTile(x, y, z);

	return blockType == (unsigned char)Block::Type::BLOCK_LAVA || blockType == (unsigned char)Block::Type::BLOCK_STILL_LAVA;
}

bool Level::isLavaTile(unsigned char blockType)
{
	return blockType == (unsigned char)Block::Type::BLOCK_LAVA || blockType == (unsigned char)Block::Type::BLOCK_STILL_LAVA;
}

AABBPosition Level::clip(glm::vec3 start, glm::vec3 end, const glm::ivec3* expected)
{
	AABBPosition aabbPosition;
	aabbPosition.isValid = false;

	if (start.x == NAN || start.y == NAN || start.z == NAN)
	{
		return aabbPosition;
	}

	if (end.x == NAN || end.y == NAN || end.z == NAN)
	{
		return aabbPosition;
	}

	const int ITERATIONS = 20;

	int startBlockX = (int)start.x, startBlockY = (int)start.y, startBlockZ = (int)start.z;
	int endBlockX = (int)end.x, endBlockY = (int)end.y, endBlockZ = (int)end.z;

	int i = ITERATIONS;
	do 
	{
		if (start.x == NAN || start.y == NAN || start.z == NAN)
		{
			break;
		}

		if (startBlockX == endBlockX && startBlockY == endBlockY && startBlockZ == endBlockZ)
		{
			break;
		}

		if (i < ITERATIONS)
		{
			auto nextIntersection = glm::vec3(999.0f, 999.0f, 999.0f);
			auto intersectionDistances = glm::vec3(999.0f, 999.0f, 999.0f);
			auto movementVector = end - start;

			if (endBlockX > startBlockX) { nextIntersection.x = (float)startBlockX + 1.0f; }
			if (endBlockX < startBlockX) { nextIntersection.x = (float)startBlockX; }
			if (endBlockY > startBlockY) { nextIntersection.y = (float)startBlockY + 1.0f; }
			if (endBlockY < startBlockY) { nextIntersection.y = (float)startBlockY; }
			if (endBlockZ > startBlockZ) { nextIntersection.z = (float)startBlockZ + 1.0f; }
			if (endBlockZ < startBlockZ) { nextIntersection.z = (float)startBlockZ; }

			if (nextIntersection.x != 999.0) { intersectionDistances.x = (nextIntersection.x - start.x) / movementVector.x; }
			if (nextIntersection.y != 999.0) { intersectionDistances.y = (nextIntersection.y - start.y) / movementVector.y; }
			if (nextIntersection.z != 999.0) { intersectionDistances.z = (nextIntersection.z - start.z) / movementVector.z; }

			int axisOfLeastPenetration;
			if (intersectionDistances.x < intersectionDistances.y&& intersectionDistances.x < intersectionDistances.z)
			{
				axisOfLeastPenetration = endBlockX > startBlockX ? 4 : 5;
				start.x = nextIntersection.x;
				start.y += movementVector.y * intersectionDistances.x;
				start.z += movementVector.z * intersectionDistances.x;
			}
			else if (intersectionDistances.y < intersectionDistances.z)
			{
				axisOfLeastPenetration = endBlockY > startBlockY ? 0 : 1;
				start.y = nextIntersection.y;
				start.x += movementVector.x * intersectionDistances.y;
				start.z += movementVector.z * intersectionDistances.y;
			}
			else
			{
				axisOfLeastPenetration = endBlockZ > startBlockZ ? 2 : 3;
				start.z = nextIntersection.z;
				start.x += movementVector.x * intersectionDistances.z;
				start.y += movementVector.y * intersectionDistances.z;
			}

			startBlockX = (int)start.x;
			startBlockY = (int)start.y;
			startBlockZ = (int)start.z;

			if (axisOfLeastPenetration == 5) { startBlockX--; }
			if (axisOfLeastPenetration == 1) { startBlockY--; }
			if (axisOfLeastPenetration == 3) { startBlockZ--; }
		}

		auto blockType = getTile(startBlockX, startBlockY, startBlockZ);
		auto blockDefinition = Block::Definitions[blockType];
		auto blockAABB = blockDefinition.boundingBox;

		if (expected)
		{
			if (
				blockType == (unsigned char)Block::Type::BLOCK_AIR || 
				blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID
			)
			{
				for (int x = -1; x != 2; x++)
				{
					for (int z = -1; z != 2; z++)
					{
						auto neighbor = glm::ivec3(startBlockX + x, startBlockY, startBlockZ + z);

						if (neighbor == *expected)
						{
							aabbPosition.x = (int)neighbor.x;
							aabbPosition.y = (int)neighbor.y;
							aabbPosition.z = (int)neighbor.z;
							aabbPosition.index = (aabbPosition.z * height + aabbPosition.y) * width + aabbPosition.x;
							aabbPosition.vector = aabbPosition.vector + glm::vec3(neighbor);
							aabbPosition.destructible = false;
							aabbPosition.isValid = true;

							if (x == 0)
							{
								if (z == 1) { aabbPosition.face = 2; }
								else { aabbPosition.face = 3; }
							}
							else if (z == 0)
							{
								if (x == 1) { aabbPosition.face = 4; }
								else { aabbPosition.face = 5; }
							}
							else
							{
								aabbPosition.isValid = false;
							}

							return aabbPosition;
						}
					}
				}
			}
		}
		else 
		{
			if (
				blockType != (unsigned char)Block::Type::BLOCK_AIR &&
				blockDefinition.collide != Block::CollideType::COLLIDE_LIQUID
			)
			{
				auto origin = glm::vec3(startBlockX, startBlockY, startBlockZ);

				aabbPosition = blockAABB.clip(start - origin, end - origin);
				if (aabbPosition.isValid)
				{
					aabbPosition.x = (int)origin.x;
					aabbPosition.y = (int)origin.y;
					aabbPosition.z = (int)origin.z;
					aabbPosition.index = (aabbPosition.z * height + aabbPosition.y) * width + aabbPosition.x;
					aabbPosition.vector = aabbPosition.vector + origin;
					aabbPosition.destructible = true;

					return aabbPosition;
				}
			}
		}

	} while (i-- >= 0);

	return aabbPosition;
}