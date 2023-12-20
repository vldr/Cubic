#include "Entity.h"
#include "Game.h"
#include "Level.h"

void Entity::init(Game* game)
{
	this->game = game;
	this->onGround = false;
	this->horizontalCollision = false;
	this->collision = false;
	this->slide = true;
	this->heightOffset = 0.0f;
	this->aabbWidth = 0.6f;
	this->aabbHeight = 1.8f;
	this->oldWalkDistance = 0.0f;
	this->walkDistance = 0.0f;
	this->fallDistance = 0.0f;
	this->slideOffset = 0.0f;
	this->footSize = 0.0f;
	this->noPhysics = false;
	this->velocity = glm::vec3();
	this->position = glm::vec3();
	this->oldPosition = glm::vec3();
	this->rotation = glm::vec2();
	this->oldRotation = glm::vec2();
}

void Entity::setSize(float w, float h) 
{
	aabbWidth = w;
	aabbHeight = h;
}

void Entity::setPosition(float x, float y, float z)
{
	oldPosition.x = x;
	oldPosition.y = y;
	oldPosition.z = z;

	position.x = x;
	position.y = y;
	position.z = z;

	velocity.x = 0;
	velocity.y = 0;
	velocity.z = 0;

	float w = aabbWidth / 2.0f;
	float h = aabbHeight / 2.0f;
	aabb.x0 = x - w;
	aabb.y0 = y - h;
	aabb.z0 = z - w;
	aabb.x1 = x + w;
	aabb.y1 = y + h;
	aabb.z1 = z + w;
}

void Entity::tick()
{
	oldWalkDistance = walkDistance;

	oldPosition.x = position.x;
	oldPosition.y = position.y;
	oldPosition.z = position.z;

	oldRotation.y = rotation.y;
	oldRotation.x = rotation.x;
}

void Entity::turn(float rx, float ry) 
{
	float ory = rotation.y;
	float orx = rotation.x;

	rotation.y -= ry;
	rotation.x += rx;
	rotation.y = rotation.y < -90.0f ? -89.9f : (rotation.y > 90.0f ? 89.9f : rotation.y);

	oldRotation.y += rotation.y - ory; 
	oldRotation.x += rotation.x - orx;
}

bool Entity::isFree(float ax, float ay, float az) 
{
	AABB aabb = this->aabb.move(ax, ay, az);
	bool free = game->level.getTileAABBCount(aabb) > 0 ? false : !game->level.containsAnyLiquid(aabb);

	return free;
}

bool Entity::isInWater() 
{
	return game->level.containsLiquid(aabb.grow(0.0f, -0.4f, 0.0f), Block::Type::BLOCK_WATER) ||
		game->level.containsLiquid(aabb.grow(0.0f, -0.4f, 0.0f), Block::Type::BLOCK_STILL_WATER);
}

bool Entity::isInLava() 
{
	return game->level.containsLiquid(aabb.grow(0.0f, -0.4f, 0.0f), Block::Type::BLOCK_LAVA) ||
		game->level.containsLiquid(aabb.grow(0.0f, -0.4f, 0.0f), Block::Type::BLOCK_STILL_LAVA);
}

void Entity::moveRelative(float x, float z, float speed) 
{
	float length = glm::sqrt(x * x + z * z);

	if (length >= 0.01f) 
	{
		if (length < 1.0f) { length = 1.0f; }

		length = speed / length;
		x *= length;
		z *= length;

		float s = glm::sin(glm::radians(rotation.x));
		float c = glm::cos(glm::radians(rotation.x));

		velocity.x += x * c - z * s;
		velocity.z += z * c + x * s;
	}
}

void Entity::move(float ax, float ay, float az) 
{
	if (noPhysics) 
	{
		aabb = aabb.move(ax, ay, az);
		position.x = (aabb.x0 + aabb.x1) / 2.0f;
		position.z = (aabb.z0 + aabb.z1) / 2.0f;
		position.y = aabb.y0 + heightOffset - slideOffset;
	}
	else 
	{
		auto oldAABB = aabb;
		float x = position.x;
		float z = position.z;
		float ox = ax;
		float oy = ay;
		float oz = az;

		auto cubes = game->level.getTileAABB(aabb.expand(ax, ay, az));

		///////////////////////////////////////////////////////

		for (size_t i = 0; i < cubes.size(); i++)
		{
			ay = cubes[i].clipY(aabb, ay);
		}

		aabb = aabb.move(0.0f, ay, 0.0f);
		if (!slide && oy != ay) 
		{
			ax = 0.0f;
			ay = 0.0f;
			az = 0.0f;
		}

		///////////////////////////////////////////////////////

		for (size_t i = 0; i < cubes.size(); i++)
		{
			ax = cubes[i].clipX(aabb, ax);
		}

		aabb = aabb.move(ax, 0.0f, 0.0f);
		if (!slide && ox != ax) 
		{
			ax = 0.0f;
			ay = 0.0f;
			az = 0.0f;
		}

		///////////////////////////////////////////////////////

		for (size_t i = 0; i < cubes.size(); i++)
		{
			az = cubes[i].clipZ(aabb, az);
		}

		aabb = aabb.move(0.0f, 0.0f, az);
		if (!slide && oz != az) 
		{
			ax = 0.0f;
			ay = 0.0f;
			az = 0.0f;
		}

		///////////////////////////////////////////////////////

		if (
			(onGround || (oy != ay && oy < 0.0f)) && 
			(ox != ax || oz != az) &&
			footSize > 0.0f &&
			slideOffset < 0.05f
		)
		{
			float bx = ax;
			float by = ay;
			float bz = az;
			ax = ox;
			ay = footSize;
			az = oz;

			AABB tempAABB = aabb;
			aabb = oldAABB;

			cubes = game->level.getTileAABB(aabb.expand(ox, ay, oz));

			for (size_t i = 0; i < cubes.size(); i++)
			{
				ay = cubes[i].clipY(this->aabb, ay);
			}

			aabb = aabb.move(0.0f, ay, 0.0f);
			if (!slide && oy != ay) 
			{
				ax = 0.0f;
				ay = 0.0f;
				az = 0.0f;
			}

			///////////////////////////////////////////////////////

			for (size_t i = 0; i < cubes.size(); i++)
			{
				ax = cubes[i].clipX(this->aabb, ax);
			}

			aabb = aabb.move(ax, 0.0f, 0.0f);
			if (!slide && ox != ax) 
			{
				ax = 0.0f;
				ay = 0.0f;
				az = 0.0f;
			}

			///////////////////////////////////////////////////////

			for (size_t i = 0; i < cubes.size(); i++)
			{
				az = cubes[i].clipZ(this->aabb, az);
			}

			aabb = aabb.move(0.0f, 0.0f, az);
			if (!slide && oz != az) 
			{
				ax = 0.0f;
				ay = 0.0f;
				az = 0.0f;
			}

			///////////////////////////////////////////////////////

			if (bx * bx + bz * bz >= ax * ax + az * az) 
			{
				ax = bx;
				ay = by;
				az = bz;
				aabb = tempAABB;
			}
			else 
			{ 
				slideOffset += 0.5f;
			}
		}

		onGround = oy != ay && oy < 0.0f;
		horizontalCollision = ox != ax || oz != az;
		collision = horizontalCollision || oy != ay;

		if (onGround) 
		{
			if (fallDistance > 0.0f) 
			{ 
				fallDistance = 0.0f;
			}
		}
		else if (ay < 0.0f) 
		{ 
			fallDistance -= ay; 
		}

		if (ox != ax) { velocity.x = 0.0f; }
		if (oy != ay) { velocity.y = 0.0f; }
		if (oz != az) { velocity.z = 0.0f; }

		position.x = (aabb.x0 + aabb.x1) / 2.0f;
		position.z = (aabb.z0 + aabb.z1) / 2.0f;
		position.y = aabb.y0 + heightOffset - slideOffset;

		walkDistance += glm::sqrt((position.x - x) * (position.x - x) + (position.z - z) * (position.z - z)) * 0.6f;
		slideOffset *= 0.4f;
	}
}