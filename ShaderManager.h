#pragma once
#include <GL/glew.h>

class ShaderManager
{
public:
	GLuint load(const char* vertexSource, const char* fragmentSource);
};

