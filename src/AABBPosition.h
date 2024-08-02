#pragma once
#include <glm/glm.hpp>

struct AABBPosition 
{
  int x, y, z;
  int index;
  int face;
  bool isValid;
  bool destructible;
  glm::vec3 vector;
};