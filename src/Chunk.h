#pragma once
#include "VertexList.h"

#include <GL/glew.h>
#include <glm/glm.hpp>

class Chunk
{
public:
  void init(int x, int y, int z);
  void render();
  void renderWater();
  void update();
  float distanceToPlayer() const;

  bool isVisible;
  bool isLoaded;
  glm::ivec3 position;

  static const int SIZE = 16;
  struct Comparator
  {
    bool operator()(const Chunk* a, const Chunk* b) const;
  };

private:
  enum class FaceType { Front, Back, Left, Right, Top, Bottom };

  struct Face
  {
    bool valid = false;
    bool mirror;
    unsigned char blockType;
    float brightness;
    float blockShift;
    float height;

    bool operator==(const Face& rhs)
    {
      return rhs.valid &&
        this->valid &&
        this->blockType == rhs.blockType &&
        this->brightness == rhs.brightness &&
        this->blockShift == rhs.blockShift &&
        this->height == rhs.height;
    }
  };

  inline Face& getFace(Face* faces, int x, int y, int z);

  template <FaceType faceType>
  inline bool shouldRenderFace(const int x, const int y, const int z);

  template<FaceType faceType>
  inline void generateMesh(Face* faces);
  inline void generateFaces();

  VertexList vertices;
  VertexList waterVertices;

  static VertexList::Allocator* allocator;
  static VertexList::Allocator* waterAllocator;

  static Face topFaces[];
  static Face bottomFaces[];
  static Face leftFaces[];
  static Face rightFaces[];
  static Face frontFaces[];
  static Face backFaces[];
};

