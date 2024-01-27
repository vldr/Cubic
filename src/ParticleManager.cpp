#include "ParticleManager.h"
#include "Game.h"

#include <glm/gtc/type_ptr.hpp>

void ParticleManager::init() 
{
}

void ParticleManager::tick()
{
	for (auto particleGroup = particleGroups.begin(); particleGroup != particleGroups.end();)
	{
		auto expiredCount = 0;

		for (int i = 0; i < PARTICLES_PER_AXIS * PARTICLES_PER_AXIS * PARTICLES_PER_AXIS; i++)
		{
			if (particleGroup->particles[i].age < particleGroup->particles[i].maxAge)
			{
				particleGroup->particles[i].tick();
			}
			else
			{
				expiredCount++;
			}
		}

		if (expiredCount == PARTICLES_PER_AXIS * PARTICLES_PER_AXIS * PARTICLES_PER_AXIS)
		{
			particleGroup->vertexList.destroy();
			particleGroup = particleGroups.erase(particleGroup);
		}
		else
		{
			particleGroup++;
		}
	}
}

void ParticleManager::render()
{
	glBindTexture(GL_TEXTURE_2D, game.atlasTexture);

	for (auto& particleGroup : particleGroups)
	{
		for (int i = 0; i < PARTICLES_PER_AXIS * PARTICLES_PER_AXIS * PARTICLES_PER_AXIS; i++)
		{
			particleGroup.particles[i].update(particleGroup.vertexList);
		}

		particleGroup.vertexList.update();
		particleGroup.vertexList.render();
	}
}

void ParticleManager::spawn(float x, float y, float z, unsigned char blockType)
{
	ParticleGroup particleGroup{};
	particleGroup.vertexList.init(PARTICLES_PER_AXIS * PARTICLES_PER_AXIS * PARTICLES_PER_AXIS * VERTICES_PER_PARTICLE);

	for (int i = 0; i < PARTICLES_PER_AXIS; i++)
	{
		for (int j = 0; j < PARTICLES_PER_AXIS; j++)
		{
			for (int k = 0; k < PARTICLES_PER_AXIS; k++)
			{
				float xd = x + (i + 0.5f) / 4.0f;
				float yd = y + (j + 0.5f) / 4.0f;
				float zd = z + (k + 0.5f) / 4.0f;

				int index = (i * PARTICLES_PER_AXIS + j) * PARTICLES_PER_AXIS + k;

				particleGroup.particles[index].init(
					xd, yd, zd, xd - x - 0.5f, yd - y - 0.5f, zd - z - 0.5f, blockType
				);
			}
		}
	}

	particleGroups.push_back(particleGroup);
}
