#pragma once
#include <GL/glew.h>
#include <cstdlib>
#include <utility>

class VertexList
{
public:
  struct Vertex
  {
    inline Vertex(float x, float y, float z, float u, float v, float s)
      : x(x), y(y), z(z), u(u), v(v), s(s) {}

    inline Vertex(int x, float y, float z, float u, float v, float s)
      : x((float)x), y(y), z(z), u(u), v(v), s(s) {}

    inline Vertex(int x, int y, float z, float u, float v, float s)
      : x((float)x), y((float)y), z(z), u(u), v(v), s(s) {}

    inline Vertex(int x, int y, int z, float u, float v, float s)
      : x((float)x), y((float)y), z((float)z), u(u), v(v), s(s) {}

    inline Vertex(float x, int y, int z, float u, float v, float s)
      : x(x), y((float)y), z((float)z), u(u), v(v), s(s) {}

    inline Vertex(float x, int y, float z, float u, float v, float s)
      : x(x), y((float)y), z(z), u(u), v(v), s(s) {}

    inline Vertex(float x, float y, int z, float u, float v, float s)
      : x(x), y(y), z((float)z), u(u), v(v), s(s) {}

    inline Vertex(int x, float y, int z, float u, float v, float s)
      : x((float)x), y(y), z((float)z), u(u), v(v), s(s) {}

    inline Vertex(float x, float y, float z, int u, float v, float s)
      : x(x), y(y), z(z), u((float)u), v(v), s(s) {}

    inline Vertex(int x, float y, float z, int u, float v, float s)
      : x((float)x), y(y), z(z), u((float)u), v(v), s(s) {}

    inline Vertex(int x, int y, float z, int u, float v, float s)
      : x((float)x), y((float)y), z(z), u((float)u), v(v), s(s) {}

    inline Vertex(int x, int y, int z, int u, float v, float s)
      : x((float)x), y((float)y), z((float)z), u((float)u), v(v), s(s) {}

    inline Vertex(float x, int y, int z, int u, float v, float s)
      : x(x), y((float)y), z((float)z), u((float)u), v(v), s(s) {}

    inline Vertex(float x, int y, float z, int u, float v, float s)
      : x(x), y((float)y), z(z), u((float)u), v(v), s(s) {}

    inline Vertex(float x, float y, int z, int u, float v, float s)
      : x(x), y(y), z((float)z), u((float)u), v(v), s(s) {}

    inline Vertex(int x, float y, int z, int u, float v, float s)
      : x((float)x), y(y), z((float)z), u((float)u), v(v), s(s) {}
    
    inline Vertex(float x, float y, float z, float u, int v, float s)
      : x(x), y(y), z(z), u(u), v((float)v), s(s) {}

    inline Vertex(int x, float y, float z, float u, int v, float s)
      : x((float)x), y(y), z(z), u(u), v((float)v), s(s) {}

    inline Vertex(int x, int y, float z, float u, int v, float s)
      : x((float)x), y((float)y), z(z), u(u), v((float)v), s(s) {}

    inline Vertex(int x, int y, int z, float u, int v, float s)
      : x((float)x), y((float)y), z((float)z), u(u), v((float)v), s(s) {}

    inline Vertex(float x, int y, int z, float u, int v, float s)
      : x(x), y((float)y), z((float)z), u(u), v((float)v), s(s) {}

    inline Vertex(float x, int y, float z, float u, int v, float s)
      : x(x), y((float)y), z(z), u(u), v((float)v), s(s) {}

    inline Vertex(float x, float y, int z, float u, int v, float s)
      : x(x), y(y), z((float)z), u(u), v((float)v), s(s) {}

    inline Vertex(int x, float y, int z, float u, int v, float s)
      : x((float)x), y(y), z((float)z), u(u), v((float)v), s(s) {}

    inline Vertex(float x, float y, float z, int u, int v, float s)
      : x(x), y(y), z(z), u((float)u), v((float)v), s(s) {}

    inline Vertex(int x, float y, float z, int u, int v, float s)
      : x((float)x), y(y), z(z), u((float)u), v((float)v), s(s) {}

    inline Vertex(int x, int y, float z, int u, int v, float s)
      : x((float)x), y((float)y), z(z), u((float)u), v((float)v), s(s) {}

    inline Vertex(int x, int y, int z, int u, int v, float s)
      : x((float)x), y((float)y), z((float)z), u((float)u), v((float)v), s(s) {}

    inline Vertex(float x, int y, int z, int u, int v, float s)
      : x(x), y((float)y), z((float)z), u((float)u), v((float)v), s(s) {}

    inline Vertex(float x, int y, float z, int u, int v, float s)
      : x(x), y((float)y), z(z), u((float)u), v((float)v), s(s) {}

    inline Vertex(float x, float y, int z, int u, int v, float s)
      : x(x), y(y), z((float)z), u((float)u), v((float)v), s(s) {}

    inline Vertex(int x, float y, int z, int u, int v, float s)
      : x((float)x), y(y), z((float)z), u((float)u), v((float)v), s(s) {}

    float x;
    float y;
    float z;
    float u;
    float v;
    float s;
  };

  struct Allocator
  {
    Allocator(size_t size = 4) 
      : size(size)
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

