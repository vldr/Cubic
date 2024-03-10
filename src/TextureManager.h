#pragma once
#include <GL/glew.h>

class TextureManager
{
public:
	GLuint load(const unsigned char* data, size_t length);
	GLuint load(const unsigned char* data, size_t length, unsigned int left, unsigned int top, unsigned int right, unsigned int bottom);
	GLuint loadColor(float r, float g, float b);
};

 