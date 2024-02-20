#pragma once
#include "Particle.h"
#include "VertexList.h"

#include <GL/glew.h>
#include <vector>

class ParticleManager
{
public:
	void tick();
	void render();
	void spawn(float x, float y, float z, unsigned char blockType);

	const static int PARTICLES_PER_AXIS = 4;
	const static int VERTICES_PER_PARTICLE = 6;

	struct ParticleGroup
	{
		VertexList vertexList;
		Particle particles[
			ParticleManager::PARTICLES_PER_AXIS * 
			ParticleManager::PARTICLES_PER_AXIS * 
			ParticleManager::PARTICLES_PER_AXIS
		];
	};

private:
	std::vector<ParticleGroup> particleGroups;
};

