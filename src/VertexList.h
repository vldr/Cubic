#pragma once
#include <GL/glew.h>
#include <cstdlib>
#include <utility>

class VertexList
{
public:
  struct Vertex
  {
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    inline Vertex(T1 x, T2 y, T3 z, T4 u, T5 v, T6 s)
      : x((float)x), y((float)y), z((float)z), u((float)u), v((float)v), s((float)s) {}

    float x;
    float y;
    float z;
    float u;
    float v;
    float s;
  };

  struct Allocator
  {
    Allocator(size_t size = 4) : size(size)
    {
      data = static_cast<Vertex*>(std::malloc(size * sizeof(*data)));
    }

    ~Allocator() 
    {
      std::free(data);
    }

    Vertex* data;
    size_t size;
  };

  void init(size_t capacity = 4);
  void init(Allocator* allocator);

  void destroy();
  void update();
  void render();
  void reset();

  template <typename... Args>
  void push(Args&&... args)
  {
    if (index == allocator->size)
    {
      if (!allocator->size)
      {
        allocator->size++;
      }

      allocator->size *= 2;
      allocator->data = static_cast<VertexList::Vertex*>(
        std::realloc(allocator->data, allocator->size * sizeof(VertexList::Vertex))
      );
    }

    allocator->data[index++] = VertexList::Vertex(std::forward<Args>(args)...);
  }

private:
  Allocator* allocator;

  GLuint vao;
  GLuint buffer;

  size_t bufferLength;
  size_t length;
  size_t index;
};

