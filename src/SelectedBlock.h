#pragma once
#include <GL/glew.h>

class SelectedBlock
{
public:
	void init();
	void renderPost();

private:
	const static int BUFFER_SIZE = 24;

	GLuint vao;
	GLuint buffer;
	GLuint texture;
};

