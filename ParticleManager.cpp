#include "ParticleManager.h"
#include "Game.h"

#include <glm/gtc/type_ptr.hpp>

void ParticleManager::init(Game* game)
{
	this->game = game;
}

void ParticleManager::tick()
{
	for (auto particleGroup = particleGroups.begin(); particleGroup != particleGroups.end();)
	{
		auto invalidCount = 0;

		for (int i = 0; i < particlesPerAxis * particlesPerAxis * particlesPerAxis; i++)
		{
			if (particleGroup->particles[i].isValid)
			{
				particleGroup->particles[i].tick();
			}
			else
			{
				invalidCount++;
			}
		}

		if (invalidCount == particlesPerAxis * particlesPerAxis * particlesPerAxis)
		{
			glDeleteBuffers(1, &particleGroup->buffer);
			glDeleteVertexArrays(1, &particleGroup->vao);

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
	glBindTexture(GL_TEXTURE_2D, game->atlasTexture);

	for (auto& particleGroup : particleGroups)
	{
		glBindVertexArray(particleGroup.vao);
		glBindBuffer(GL_ARRAY_BUFFER, particleGroup.buffer);

		for (int i = 0; i < particlesPerAxis * particlesPerAxis * particlesPerAxis; i++)
		{
			if (particleGroup.particles[i].isValid)
			{
				particleGroup.particles[i].update();
			}
		}

		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)particleGroup.size);
	}
}

void ParticleManager::spawn(float x, float y, float z, unsigned char blockType)
{
	ParticleGroup particleGroup;
	particleGroup.size = particlesPerAxis * particlesPerAxis * particlesPerAxis * verticesPerParticle;

	glGenVertexArrays(1, &particleGroup.vao);
	glBindVertexArray(particleGroup.vao);

	glGenBuffers(1, &particleGroup.buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particleGroup.buffer);

	glBufferData(GL_ARRAY_BUFFER,
		particleGroup.size * sizeof(glm::vec3) +
		particleGroup.size * sizeof(glm::vec2) +
		particleGroup.size * sizeof(float),
		NULL,
		GL_DYNAMIC_DRAW
	);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(particleGroup.size * sizeof(glm::vec3)));
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (void*)(particleGroup.size * sizeof(glm::vec3) + particleGroup.size * sizeof(glm::vec2)));

	for (int i = 0; i < particlesPerAxis; i++)
	{
		for (int j = 0; j < particlesPerAxis; j++)
		{
			for (int k = 0; k < particlesPerAxis; k++)
			{
				float xd = x + (i + 0.5f) / 4.0f;
				float yd = y + (j + 0.5f) / 4.0f;
				float zd = z + (k + 0.5f) / 4.0f;

				int index = (i * particlesPerAxis + j) * particlesPerAxis + k;

				particleGroup.particles[index].init(
					game, index, xd, yd, zd, xd - x - 0.5f, yd - y - 0.5f, zd - z - 0.5f, blockType
				);
			}
		}
	}

	particleGroups.push_back(particleGroup);
}
