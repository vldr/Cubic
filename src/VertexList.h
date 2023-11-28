#pragma once

#include <vector>
#include <GL/glew.h>

class Game;

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

	void init(Game* game, size_t capacity = 4);
	void destroy();
	void update();
	void render();
	void reset();
	void push(const VertexList::Vertex& vertex);

private:
	Vertex* vertices;
	GLuint vao;
	GLuint buffer;

	size_t bufferLength;
	size_t length;
	size_t index;
	size_t capacity;
};

