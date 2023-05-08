#pragma once
#include "Particle.h"

#include <GL/glew.h>
#include <vector>

class ParticleManager
{
public:
	void init(Game* game);
	void tick();
	void render();
	void spawn(float x, float y, float z, unsigned char blockType);

	const static int particlesPerAxis = 4;
	const static int verticesPerParticle = 6;

	struct ParticleGroup
	{
		GLuint vao;
		GLuint buffer;
		size_t size;

		Particle particles[ParticleManager::particlesPerAxis * ParticleManager::particlesPerAxis * ParticleManager::particlesPerAxis];
	};

private:
	Game* game;

	std::vector<ParticleGroup> particleGroups;
};

