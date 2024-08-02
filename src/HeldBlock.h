#pragma once
#include "VertexList.h"

#include <GL/glew.h>

class HeldBlock
{
public:
  void init();
  void update();
  void tick();
  void render();
  void swing();
  void reset();
private:
  float height;
  float lastHeight;

  int swingOffset;
  bool isSwinging;

  VertexList vertices;
};

