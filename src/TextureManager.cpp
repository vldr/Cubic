#include "TextureManager.h"
#include "PNG.h"

GLuint TextureManager::load(const unsigned char* data, size_t length)
{
    upng_t* upng = upng_new_from_bytes(data, (unsigned long)length);
    upng_decode(upng);

    auto width = upng_get_width(upng);
    auto height = upng_get_height(upng);
    auto format = upng_get_components(upng) > 3 ? GL_RGBA : GL_RGB;
    
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, upng_get_buffer(upng));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    upng_free(upng);
    
    return texture;
}

GLuint TextureManager::load(const unsigned char* data, size_t length, unsigned int left, unsigned int top, unsigned int right, unsigned int bottom)
{
    upng_t* upng = upng_new_from_bytes(data, (unsigned long)length);
    upng_decode(upng);

    auto width = upng_get_width(upng);
    auto height = upng_get_height(upng);
    auto components = upng_get_components(upng);
    auto format = components > 3 ? GL_RGBA : GL_RGB;
    auto buffer = upng_get_buffer(upng);

    std::vector<unsigned char> image(components * right * bottom);

    for (unsigned int i = 0; i < width * height; i++)
    {
        const unsigned int row = i % width;
        const unsigned int column = i / width;

        if (row < left || column < top || row >= left + right || column >= top + bottom)
        {
            continue;
        }

        const unsigned int offset = (row - left) + bottom * (column - top);

        for (unsigned int j = 0; j < components; j++)
        {
            image[offset * components + j] = buffer[i * components + j];
        }
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, right, bottom, 0, format, GL_UNSIGNED_BYTE, &image[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    upng_free(upng);

    return texture;
}

GLuint TextureManager::loadColor(float r, float g, float b)
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