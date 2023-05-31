#pragma once

#include <GL/glew.h>
#include <unordered_map>
#include <vector>

class TextureManager
{
public:
	GLuint load(const unsigned char* data, size_t length);
	GLuint generateSolidColor(float r, float g, float b);
	GLuint generateSolidColor(float r, float g, float b, float a);
};

 