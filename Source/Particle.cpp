#include "Particle.h"
#include "Game.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

void Particle::init(
	Game* game, 
	int index, 
	float x, float y, float z, 
	float xd, float yd, float zd, 
	unsigned char blockType
)
{
	Entity::init(game);

	setSize(0.2f, 0.2f);
	setPosition(x, y, z);

	this->index = index;
	this->isValid = true;
	this->heightOffset = aabbHeight / 2.0f;

	this->velocity.x = xd + float(game->random.uniform() * 2.0f - 1.0f) * 0.4f;
	this->velocity.y = yd + float(game->random.uniform() * 2.0f - 1.0f) * 0.4f;
	this->velocity.z = zd + float(game->random.uniform() * 2.0f - 1.0f) * 0.4f;

	float speed = float(game->random.uniform() + game->random.uniform() + 1.0f) * 0.15f * 0.4f / glm::length(velocity);

	this->velocity.x *= speed;
	this->velocity.y *= speed;
	this->velocity.z *= speed;
	this->velocity.y += 0.1f;

	this->size = 0.1f * ((float)game->random.uniform() * 0.5f + 0.5f);
	this->lifeTime = int(4.0f / (game->random.uniform() * 0.9f + 0.1f));
	this->age = 0;

	auto u = (float)game->random.uniform() * 3.0f;
	auto v = (float)game->random.uniform() * 3.0f;

	auto texture = Block::Definitions[blockType].sideTexture;
	float u0 = (texture % 16 + u / 4.0f) / 16.0f;
	float v0 = (texture / 16 + v / 4.0f) / 16.0f;
	float u1 = u0 + 0.015609375f;
	float v1 = v0 + 0.015609375f;

	float uvs[ParticleManager::verticesPerParticle * 2] =
	{
		u0, v0,
		u0, v1,
		u1, v1,
		u0, v0,
		u1, v1,
		u1, v0,
	};

	glBufferSubData(GL_ARRAY_BUFFER, 
		ParticleManager::particlesPerAxis * 
		ParticleManager::particlesPerAxis * 
		ParticleManager::particlesPerAxis * 
		ParticleManager::verticesPerParticle * 
		sizeof(glm::vec3) +
		index * sizeof(uvs), sizeof(uvs), uvs
	);

	float brightness = game->level.getTileBrightness((int)x, (int)y, (int)z);
	float shades[ParticleManager::verticesPerParticle] =
	{
		brightness,
		brightness,
		brightness,

		brightness,
		brightness,
		brightness,
	};

	glBufferSubData(GL_ARRAY_BUFFER,
		ParticleManager::particlesPerAxis *
		ParticleManager::particlesPerAxis *
		ParticleManager::particlesPerAxis *
		ParticleManager::verticesPerParticle *
		sizeof(glm::vec3) +
		ParticleManager::particlesPerAxis *
		ParticleManager::particlesPerAxis *
		ParticleManager::particlesPerAxis *
		ParticleManager::verticesPerParticle * 
		sizeof(glm::vec2) +
		index * sizeof(shades), sizeof(shades), shades
	);
}

void Particle::tick()
{
	Entity::tick();

	this->age++;
	this->velocity.y -= 0.04f;

	move(this->velocity.x, this->velocity.y, this->velocity.z);

	this->velocity.x *= 0.98f;
	this->velocity.y *= 0.98f;
	this->velocity.z *= 0.98f;

	if (onGround) 
	{
		this->velocity.x *= 0.7f;
		this->velocity.z *= 0.7f;
	}
}

void Particle::update()
{
	if (age < lifeTime) 
	{ 
		const auto viewPosition = oldPosition + ((position - oldPosition) * game->timer.delta);

		float x = -glm::cos(glm::radians(game->localPlayer.rotation.y));
		float z = -glm::sin(glm::radians(game->localPlayer.rotation.y));
		float y = glm::cos(glm::radians(game->localPlayer.rotation.x));

		float rotX = -z * glm::sin(glm::radians(game->localPlayer.rotation.x));
		float rotZ = x * glm::sin(glm::radians(game->localPlayer.rotation.x));

		float vertices[ParticleManager::verticesPerParticle * 3] =
		{
			viewPosition.x - x * size + rotX * size, viewPosition.y + y * size, viewPosition.z - z * size + rotZ * size,
			viewPosition.x - x * size - rotX * size, viewPosition.y - y * size, viewPosition.z - z * size - rotZ * size,
			viewPosition.x + x * size - rotX * size, viewPosition.y - y * size, viewPosition.z + z * size - rotZ * size,

			viewPosition.x - x * size + rotX * size, viewPosition.y + y * size, viewPosition.z - z * size + rotZ * size,
			viewPosition.x + x * size - rotX * size, viewPosition.y - y * size, viewPosition.z + z * size - rotZ * size,
			viewPosition.x + x * size + rotX * size, viewPosition.y + y * size, viewPosition.z + z * size + rotZ * size,
		};

		glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(vertices), sizeof(vertices), vertices);
	}
	else
	{
		float vertices[ParticleManager::verticesPerParticle * 3] = {};
		glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(vertices), sizeof(vertices), vertices);

		isValid = false;
	}
}