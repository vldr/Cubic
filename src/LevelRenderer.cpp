#include "LevelRenderer.h"
#include "Game.h"
#include "Level.h"
#include "Chunk.h"
#include "Skybox.h"
#include "Random.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

void LevelRenderer::init()
{
    skybox.init();

    xChunks = Level::WIDTH / Chunk::SIZE;
    yChunks = Level::HEIGHT / Chunk::SIZE;
    zChunks = Level::DEPTH / Chunk::SIZE;
    chunks = std::make_unique<Chunk[]>(xChunks * yChunks * zChunks);

    for (int x = 0; x < xChunks; x++)
    {
        for (int y = 0; y < yChunks; y++)
        {
            for (int z = 0; z < zChunks; z++)
            {
                auto chunk = getChunk(x, y, z);
                chunk->init(Chunk::SIZE * x, Chunk::SIZE * y, Chunk::SIZE * z);
            }
        }
    }
}

void LevelRenderer::render()
{
    int chunkUpdates = 0;
    while (chunkUpdates < MAX_CHUNK_UPDATES && !chunkQueue.empty())
    {
        Chunk* chunk = chunkQueue.top();
        chunk->isLoaded = true;
        chunk->update();

        chunkUpdates++;
        chunkQueue.pop();
    }

    game.chunkUpdates += chunkUpdates;

    glBindTexture(GL_TEXTURE_2D, game.atlasTexture);
    
    for (int i = 0; i < xChunks * yChunks * zChunks; i++)
    {
        Chunk* chunk = &chunks[i];
        chunk->isVisible = game.frustum.contains(chunk);

        if (chunk->isVisible)
        {
            chunk->render();
        }
    }

    skybox.renderBedrock();
    skybox.renderSky();
    skybox.renderClouds();
}

void LevelRenderer::renderPost()
{
    glBindTexture(GL_TEXTURE_2D, game.atlasTexture);
    glColorMask(false, false, false, false);

    for (int i = 0; i < xChunks * yChunks * zChunks; i++)
    {
        Chunk* chunk = &chunks[i];

        if (chunk->isVisible)
        {
            chunks[i].renderWater();
        }
    }

    skybox.renderWater();

    glBindTexture(GL_TEXTURE_2D, game.atlasTexture);
    glColorMask(true, true, true, true);

    for (int i = 0; i < xChunks * yChunks * zChunks; i++)
    {
        Chunk* chunk = &chunks[i];

        if (chunk->isVisible)
        {
            chunks[i].renderWater();
        }
    }

    skybox.renderWater();
}

void LevelRenderer::tick()
{
    glBindTexture(GL_TEXTURE_2D, game.atlasTexture);

    updateWaterTexture();
    updateLavaTexture();

    skybox.updateWater(waterTextureData);
}

void LevelRenderer::updateWaterTexture()
{
    for (int x = 0; x < 16; x++) 
    {
        for (int y = 0; y < 16; y++) 
        {
            float v = 0.0;
            for (int i = x - 1; i <= x + 1; i++) 
            {
                int a = i & 15;
                int b = y & 15;
                v += waterTextureRed[a + (b << 4)];
            }

            waterTextureBlue[x + (y << 4)] = v / 3.3f + waterTextureGreen[x + (y << 4)] * 0.8f;
        }
    }

    for (int x = 0; x < 16; x++) 
    {
        for (int y = 0; y < 16; y++) 
        {
            waterTextureGreen[x + (y << 4)] += waterTextureAlpha[x + (y << 4)] * 0.05f;
            if (waterTextureGreen[x + (y << 4)] < 0.0) { waterTextureGreen[x + (y << 4)] = 0.0; }
            waterTextureAlpha[x + (y << 4)] -= 0.1f;
            if (game.random.uniform() < 0.05) { waterTextureAlpha[x + (y << 4)] = 0.5; }
        }
    }

    for (int i = 0; i < 256; i++) 
    {
        float bluePrevious = waterTextureBlue[i];
        waterTextureBlue[i] = waterTextureRed[i];
        waterTextureRed[i] = bluePrevious;
        float v = waterTextureRed[i];
        if (v > 1.0) { v = 1.0; }
        if (v < 0.0) { v = 0.0; }
        int a = (int)(32.0f + v * v * 32.0f);
        int b = (int)(50.0f + v * v * 64.0f);
        int c = (int)255;
        int d = (int)(180.0f + v * v * 45.0f);

        waterTextureData[i << 2] = a;
        waterTextureData[(i << 2) + 1] = b;
        waterTextureData[(i << 2) + 2] = c;
        waterTextureData[(i << 2) + 3] = d;
    }

    const auto texture = Block::Definitions[(unsigned char)Block::Type::BLOCK_WATER].topTexture;
    glTexSubImage2D(GL_TEXTURE_2D, 0, texture % 16 << 4, texture / 16 << 4, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, waterTextureData);
}

void LevelRenderer::updateLavaTexture()
{
    for (int x = 0; x < 16; x++) 
    {
        for (int y = 0; y < 16; y++) 
        {
            int sy = (int)(glm::sin(y * M_PI / 8.0) * 1.2);
            int sx = (int)(glm::sin(x * M_PI / 8.0) * 1.2);
            float v = 0.0;
            for (int i = x - 1; i <= x + 1; i++) {
                for (int j = y - 1; j <= y + 1; j++) {
                    int a = i + sy & 15;
                    int b = j + sx & 15;
                    v += lavaTextureRed[a + (b << 4)];
                }
            }
            lavaTextureGreen[x + (y << 4)] = v / 10.0f + (lavaTextureBlue[(x & 15) + ((y & 15) << 4)] + lavaTextureBlue[(x + 1 & 15) + ((y & 15) << 4)]
                + lavaTextureBlue[(x + 1 & 15) + ((y + 1 & 15) << 4)] + lavaTextureBlue[(x & 15) + ((y + 1 & 15) << 4)]) / 4.0f * 0.8f;
            lavaTextureBlue[x + (y << 4)] += lavaTextureAlpha[x + (y << 4)] * 0.01f;
            if (lavaTextureBlue[x + (y << 4)] < 0.0) { lavaTextureBlue[x + (y << 4)] = 0.0; }
            lavaTextureAlpha[x + (y << 4)] -= 0.06f;
            if (game.random.uniform() < 0.005) { lavaTextureAlpha[x + (y << 4)] = 1.5; }
        }
    }

    for (int i = 0; i < 256; i++) 
    {
        float red = lavaTextureRed[i];
        lavaTextureRed[i] = lavaTextureGreen[i];
        lavaTextureGreen[i] = red;
        float v = lavaTextureRed[i] * 2.0f;
        if (v > 1.0) { v = 1.0f; }
        if (v < 0.0) { v = 0.0f; }
        int a = (int)(v * 100.0f + 155.0f);
        int b = (int)(v * v * 255.0f);
        int c = (int)(v * v * v * v * 128.0f);

        lavaTextureData[i << 2] = a;
        lavaTextureData[(i << 2) + 1] = b;
        lavaTextureData[(i << 2) + 2] = c;
        lavaTextureData[(i << 2) + 3] = -1;
    }

    const auto texture = Block::Definitions[(unsigned char)Block::Type::BLOCK_LAVA].topTexture;
    glTexSubImage2D(GL_TEXTURE_2D, 0, texture % 16 << 4, texture / 16 << 4, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, lavaTextureData);
}

void LevelRenderer::initChunks()
{
    for (int i = 0; i < xChunks * yChunks * zChunks; i++)
    {
        Chunk* chunk = &chunks[i];
		chunk->isLoaded = false;

		chunkQueue.push(chunk);
    }	
}

void LevelRenderer::loadChunks(int x0, int y0, int z0, int x1, int y1, int z1)
{
    x0 /= Chunk::SIZE;
    y0 /= Chunk::SIZE;
    z0 /= Chunk::SIZE;

    x1 /= Chunk::SIZE;
    y1 /= Chunk::SIZE;
    z1 /= Chunk::SIZE;

    if (x0 < 0)
    {
        x0 = 0;
    }

    if (y0 < 0)
    {
        y0 = 0;
    }

    if (z0 < 0)
    {
        z0 = 0;
    }

    if (x1 > xChunks - 1)
    {
        x1 = xChunks - 1;
    }

    if (y1 > yChunks - 1)
    {
        y1 = yChunks - 1;
    }

    if (z1 > zChunks - 1)
    {
        z1 = zChunks - 1;
    }

    for (int x = x0; x <= x1; x++)
    {
        for (int y = y0; y <= y1; y++)
        {
            for (int z = z0; z <= z1; z++)
            {
                Chunk* chunk = getChunk(x, y, z);

                if (chunk->isLoaded)
                {
                    chunk->isLoaded = false;

                    chunkQueue.push(chunk);
                }
            }
        }
    }
}

Chunk* LevelRenderer::getChunk(int x, int y, int z)
{
    return &chunks[(z * yChunks + y) * xChunks + x];
}
