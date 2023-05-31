#include "TextureManager.h"
#include "Lodepng.h"

GLuint TextureManager::load(const unsigned char* data, size_t length)
{
    std::vector<unsigned char> image;

    unsigned width, height;
    unsigned error = lodepng::decode(image, width, height, data, length);

    if (error)
    {
        printf("Failed to load texture: %d\n", error);
        exit(0);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return texture;
}

GLuint TextureManager::generateSolidColor(float r, float g, float b)
{
    const auto size = 1;
    unsigned char data[3 * size * size * sizeof(unsigned char)];

    for (unsigned int i = 0; i < size * size; i++)
    {
        data[i * 3] = (unsigned char)(r * 255.0f);
        data[i * 3 + 1] = (unsigned char)(g * 255.0f);
        data[i * 3 + 2] = (unsigned char)(b * 255.0f);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);;

    return texture;
}

GLuint TextureManager::generateSolidColor(float r, float g, float b, float a)
{
    const auto size = 1;
    unsigned char data[4 * size * size * sizeof(unsigned char)];

    for (unsigned int i = 0; i < size * size; i++)
    {
        data[i * 3] = (unsigned char)(r * 255.0f);
        data[i * 3 + 1] = (unsigned char)(g * 255.0f);
        data[i * 3 + 2] = (unsigned char)(b * 255.0f);
        data[i * 3 + 3] = (unsigned char)(a * 255.0f);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);;

    return texture;
}
