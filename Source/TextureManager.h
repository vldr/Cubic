#pragma once

#include <GL/glew.h>
#include <unordered_map>
#include <vector>

class TextureManager
{
public:
	GLuint load(const char* path);
	GLuint load(const char* path, std::vector<unsigned char>& image);
	GLuint generateSolidColor(float r, float g, float b);
	GLuint generateSolidColor(float r, float g, float b, float a);
private:
	std::unordered_map<const char*, GLuint> textures;
};

 