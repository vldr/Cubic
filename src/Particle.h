#pragma once
#include "Entity.h"

class VertexList;

class Particle : public Entity
{
public:
  void init(float x, float y, float z, float xd, float yd, float zd, unsigned char blockType);
  void tick();
  void update(VertexList& vertexList);

  friend class ParticleManager;
private:
  VertexList* vertexList;

  int age;
  int maxAge;
  float size;

  float u0;
  float v0;
  float u1;
  float v1;
  float brightness;
};

