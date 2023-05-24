#include "Chunk.h"
#include "Level.h"
#include "Game.h"
#include "LocalPlayer.h"

void Chunk::init(Game* game, int x, int y, int z)
{
    this->game = game;
    this->position = glm::ivec3(x, y, z);
    this->isVisible = false;
    this->isLoaded = false;

    this->vertices.init();
    this->waterVertices.init();
}

bool Chunk::shouldRenderFace(const Block::Definition& current, const Block::Definition& neighbor, bool isBottom)
{
    return current.draw == Block::DrawType::DRAW_TRANSPARENT_THICK ||

        (neighbor.draw == Block::DrawType::DRAW_OPAQUE_SMALL && (isBottom || current.draw != Block::DrawType::DRAW_OPAQUE_SMALL)) ||
        neighbor.draw == Block::DrawType::DRAW_GAS ||
        (current.draw == Block::DrawType::DRAW_TRANSLUCENT && (neighbor.draw == Block::DrawType::DRAW_TRANSPARENT || neighbor.draw == Block::DrawType::DRAW_TRANSPARENT_THICK)) || (
            ((current.draw == Block::DrawType::DRAW_OPAQUE || current.draw == Block::DrawType::DRAW_OPAQUE_SMALL) &&
                (neighbor.draw == Block::DrawType::DRAW_SPRITE ||
                    neighbor.draw == Block::DrawType::DRAW_TRANSLUCENT ||
                    neighbor.draw == Block::DrawType::DRAW_TRANSPARENT ||
                    neighbor.draw == Block::DrawType::DRAW_TRANSPARENT_THICK))
            || (
                (current.draw == Block::DrawType::DRAW_SPRITE ||
                    neighbor.draw == Block::DrawType::DRAW_SPRITE ||

                    current.draw == Block::DrawType::DRAW_TRANSPARENT ||
                    current.draw == Block::DrawType::DRAW_TRANSPARENT_THICK) && (neighbor.draw == Block::DrawType::DRAW_SPRITE ||

                        neighbor.draw == Block::DrawType::DRAW_TRANSLUCENT ||
                        neighbor.draw == Block::DrawType::DRAW_TRANSPARENT ||
                        neighbor.draw == Block::DrawType::DRAW_TRANSPARENT_THICK) && current.draw != neighbor.draw)
            );
}

void Chunk::update()
{
    for (int x = position.x; x < (position.x + WIDTH); x++)
    {
        for (int y = position.y; y < (position.y + HEIGHT); y++)
        {
            for (int z = position.z; z < (position.z + DEPTH); z++)
            {
                auto blockType = game->level.getRenderTile(x, y, z);
                if (blockType != (unsigned char)Block::Type::BLOCK_AIR)
                {
                    Block::Definition blockDefinition = Block::Definitions[blockType];
                    auto* vertices = &this->vertices;
                     
                    float u = 0.0625f * (blockDefinition.sideTexture % 16);
                    float v = 0.0625f * (blockDefinition.sideTexture / 16) + (0.0625f - (0.0625f * blockDefinition.height));
                    float u2 = 0.0625f + 0.0625f * (blockDefinition.sideTexture % 16);
                    float v2 = 0.0625f + 0.0625f * (blockDefinition.sideTexture / 16);

                    if (blockDefinition.draw == Block::DrawType::DRAW_SPRITE)
                    {
                        vertices->push(VertexList::Vertex(x, 1.0f + y, z, u, v, 1.0f));
                        vertices->push(VertexList::Vertex(x, y, z, u, v2, 1.0f));
                        vertices->push(VertexList::Vertex(1.0f + x, y, 1.0f + z, u2, v2, 1.0f));

                        vertices->push(VertexList::Vertex(x, 1.0f + y, z, u, v, 1.0f));
                        vertices->push(VertexList::Vertex(1.0f + x, y, 1.0f + z, u2, v2, 1.0f));
                        vertices->push(VertexList::Vertex(1.0f + x, 1.0f + y, 1.0f + z, u2, v, 1.0f));

                        vertices->push(VertexList::Vertex(x, 1.0f + y, 1.0f + z, u, v, 1.0f));
                        vertices->push(VertexList::Vertex(x, y, 1.0f + z, u, v2, 1.0f));
                        vertices->push(VertexList::Vertex(1.0f + x, y, z, u2, v2, 1.0f));

                        vertices->push(VertexList::Vertex(x, 1.0f + y, 1.0f + z, u, v, 1.0f));
                        vertices->push(VertexList::Vertex(1.0f + x, y, z, u2, v2, 1.0f));
                        vertices->push(VertexList::Vertex(1.0f + x, 1.0f + y, z, u2, v, 1.0f));

                        vertices->push(VertexList::Vertex(1.0f + x, 1.0f + y, 1.0f + z, u, v, 1.0f));
                        vertices->push(VertexList::Vertex(1.0f + x, y, 1.0f + z, u, v2, 1.0f));
                        vertices->push(VertexList::Vertex(x, y, z, u2, v2, 1.0f));

                        vertices->push(VertexList::Vertex(1.0f + x, 1.0f + y, 1.0f + z, u, v, 1.0f));
                        vertices->push(VertexList::Vertex(x, y, z, u2, v2, 1.0f));
                        vertices->push(VertexList::Vertex(x, 1.0f + y, z, u2, v, 1.0f));

                        vertices->push(VertexList::Vertex(1.0f + x, 1.0f + y, z, u, v, 1.0f));
                        vertices->push(VertexList::Vertex(1.0f + x, y, z, u, v2, 1.0f));
                        vertices->push(VertexList::Vertex(x, y, 1.0f + z, u2, v2, 1.0f));

                        vertices->push(VertexList::Vertex(1.0f + x, 1.0f + y, z, u, v, 1.0f));
                        vertices->push(VertexList::Vertex(x, y, 1.0f + z, u2, v2, 1.0f));
                        vertices->push(VertexList::Vertex(x, 1.0f + y, 1.0f + z, u2, v, 1.0f));
                    }
                    else
                    {
                        if (game->level.isWaterTile(blockType))
                        {
                            vertices = &waterVertices;
                        }

                        auto shouldRenderTop = shouldRenderFace(blockDefinition, Block::Definitions[game->level.getRenderTile(x, y + 1, z)]);

                        if (!shouldRenderTop && blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID)
                        {
                            for (int xOffset = -1; xOffset < 2 && !shouldRenderTop; xOffset++)
                            {
                                for (int zOffset = -1; zOffset < 2 && !shouldRenderTop; zOffset++)
                                {
                                    if (xOffset == 0 && zOffset == 0)
                                    {
                                        continue;
                                    }

                                    if (game->level.isInBounds(x + xOffset, y, z + zOffset) && game->level.isInBounds(x + xOffset, y + 1, z + zOffset))
                                    {
                                        const auto currentBlockId = game->level.getRenderTile(x + xOffset, y, z + zOffset);
                                        const auto neighborBlockId = game->level.getRenderTile(x + xOffset, y + 1, z + zOffset);

                                        const auto currentBlockDefinition = Block::Definitions[currentBlockId];
                                        const auto neighborBlockDefinition = Block::Definitions[neighborBlockId];

                                        if (
                                            currentBlockDefinition.collide == Block::CollideType::COLLIDE_LIQUID &&
                                            neighborBlockDefinition.collide != Block::CollideType::COLLIDE_LIQUID &&
                                            shouldRenderFace(currentBlockDefinition, neighborBlockDefinition) &&
                                            game->level.getRenderTile(x, y + 1, z) != game->level.getRenderTile(x, y, z)
                                        )
                                        {
                                            shouldRenderTop = true;
                                        }
                                    }
                                }
                            }
                        }
                        else if (!shouldRenderTop && blockDefinition.height < 1.0f)
                        {
                            shouldRenderTop = true;
                        }

                        if (shouldRenderTop)
                        {
                            if (blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID)
                            {
                                blockDefinition.height = 0.9f;
                            }

                            float u = 0.0625f * (blockDefinition.topTexture % 16);
                            float v = 0.0625f * (blockDefinition.topTexture / 16);
                            float u2 = 0.0625f + 0.0625f * (blockDefinition.topTexture % 16);
                            float v2 = 0.0625f + 0.0625f * (blockDefinition.topTexture / 16);
                            float brightness = game->level.isLavaTile(blockType) ? game->level.getTileBrightness(x, y, z) : game->level.getTileBrightness(x, y + 1, z);

                            vertices->push(VertexList::Vertex(x, blockDefinition.height + y, z, u, v, brightness));
                            vertices->push(VertexList::Vertex(x, blockDefinition.height + y, 1.0f + z, u, v2, brightness));
                            vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, 1.0f + z, u2, v2, brightness));

                            vertices->push(VertexList::Vertex(x, blockDefinition.height + y, z, u, v, brightness));
                            vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, 1.0f + z, u2, v2, brightness));
                            vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, z, u2, v, brightness));

                            if (game->level.isInBounds(x, y + 1, z) && blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID)
                            {
                                vertices->push(VertexList::Vertex(x, blockDefinition.height + y, 1.0f + z, u, v, brightness));
                                vertices->push(VertexList::Vertex(x, blockDefinition.height + y, z, u, v2, brightness));
                                vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, z, u2, v2, brightness));

                                vertices->push(VertexList::Vertex(x, blockDefinition.height + y, 1.0f + z, u, v, brightness));
                                vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, z, u2, v2, brightness));
                                vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, 1.0f + z, u2, v, brightness));
                            }
                        }

                        if (shouldRenderFace(blockDefinition, Block::Definitions[game->level.getRenderTile(x, y - 1, z)], true))
                        {
                            float u = 0.0625f * (blockDefinition.bottomTexture % 16);
                            float v = 0.0625f * (blockDefinition.bottomTexture / 16);
                            float u2 = 0.0625f + 0.0625f * (blockDefinition.bottomTexture % 16);
                            float v2 = 0.0625f + 0.0625f * (blockDefinition.bottomTexture / 16);
                            float brightness = game->level.isLavaTile(blockType) ? game->level.getTileBrightness(x, y, z) : game->level.getTileBrightness(x, y - 1, z);

                            vertices->push(VertexList::Vertex(x, y, 1.0f + z, u, v, brightness * 0.5f));
                            vertices->push(VertexList::Vertex(x, y, z, u, v2, brightness * 0.5f));
                            vertices->push(VertexList::Vertex(1.0f + x, y, z, u2, v2, brightness * 0.5f));

                            vertices->push(VertexList::Vertex(x, y, 1.0f + z, u, v, brightness * 0.5f));
                            vertices->push(VertexList::Vertex(1.0f + x, y, z, u2, v2, brightness * 0.5f));
                            vertices->push(VertexList::Vertex(1.0f + x, y, 1.0f + z, u2, v, brightness * 0.5f));

                            if (game->level.isInBounds(x, y - 1, z) && blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID)
                            {
                                vertices->push(VertexList::Vertex(x, y, z, u, v, brightness));
                                vertices->push(VertexList::Vertex(x, y, 1.0f + z, u, v2, brightness));
                                vertices->push(VertexList::Vertex(1.0f + x, y, 1.0f + z, u2, v2, brightness));

                                vertices->push(VertexList::Vertex(x, y, z, u, v, brightness));
                                vertices->push(VertexList::Vertex(1.0f + x, y, 1.0f + z, u2, v2, brightness));
                                vertices->push(VertexList::Vertex(1.0f + x, y, z, u2, v, brightness));
                            }
                        }

                        if (shouldRenderFace(blockDefinition, Block::Definitions[game->level.getRenderTile(x, y, z + 1)]))
                        {
                            float blockShift = 0.0f;

                            if (
                                blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID &&
                                game->level.getRenderTile(x, y, z) == game->level.getRenderTile(x, y - 1, z + 1)
                            )
                            {
                                blockShift = -0.1f;
                            }

                            float brightness = game->level.isLavaTile(blockType) ? game->level.getTileBrightness(x, y, z) : game->level.getTileBrightness(x, y, z + 1);

                            vertices->push(VertexList::Vertex(x, blockDefinition.height + y, 1.0f + z, u, v, brightness * 0.8f));
                            vertices->push(VertexList::Vertex(x, blockShift + y, 1.0f + z, u, v2, brightness * 0.8f));
                            vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, 1.0f + z, u2, v2, brightness * 0.8f));

                            vertices->push(VertexList::Vertex(x, blockDefinition.height + y, 1.0f + z, u, v, brightness * 0.8f));
                            vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, 1.0f + z, u2, v2, brightness * 0.8f));
                            vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, 1.0f + z, u2, v, brightness * 0.8f));

                            if (game->level.isInBounds(x, y, z + 1) && blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID)
                            {
                                vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, 1.0f + z, u, v, brightness * 0.8f));
                                vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, 1.0f + z, u, v2, brightness * 0.8f));
                                vertices->push(VertexList::Vertex(x, blockShift + y, 1.0f + z, u2, v2, brightness * 0.8f));

                                vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, 1.0f + z, u, v, brightness * 0.8f));
                                vertices->push(VertexList::Vertex(x, blockShift + y, 1.0f + z, u2, v2, brightness * 0.8f));
                                vertices->push(VertexList::Vertex(x, blockDefinition.height + y, 1.0f + z, u2, v, brightness * 0.8f));
                            }
                        }

                        if (shouldRenderFace(blockDefinition, Block::Definitions[game->level.getRenderTile(x, y, z - 1)]))
                        {
                            float blockShift = 0.0f;

                            if (
                                blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID &&
                                game->level.getRenderTile(x, y, z) == game->level.getRenderTile(x, y - 1, z - 1)
                            )
                            {
                                blockShift = -0.1f;
                            }

                            float brightness = game->level.isLavaTile(blockType) ? game->level.getTileBrightness(x, y, z) : game->level.getTileBrightness(x, y, z - 1);

                            vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, z, u, v, brightness * 0.8f));
                            vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, z, u, v2, brightness * 0.8f));
                            vertices->push(VertexList::Vertex(x, blockShift + y, z, u2, v2, brightness * 0.8f));

                            vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, z, u, v, brightness * 0.8f));
                            vertices->push(VertexList::Vertex(x, blockShift + y, z, u2, v2, brightness * 0.8f));
                            vertices->push(VertexList::Vertex(x, blockDefinition.height + y, z, u2, v, brightness * 0.8f));

                            if (game->level.isInBounds(x, y, z - 1) && blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID)
                            {
                                vertices->push(VertexList::Vertex(x, blockDefinition.height + y, z, u, v, brightness * 0.8f));
                                vertices->push(VertexList::Vertex(x, blockShift + y, z, u, v2, brightness * 0.8f));
                                vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, z, u2, v2, brightness * 0.8f));

                                vertices->push(VertexList::Vertex(x, blockDefinition.height + y, z, u, v, brightness * 0.8f));
                                vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, z, u2, v2, brightness * 0.8f));
                                vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, z, u2, v, brightness * 0.8f));
                            }
                        }

                        if (shouldRenderFace(blockDefinition, Block::Definitions[game->level.getRenderTile(x + 1, y, z)]))
                        {
                            float blockShift = 0.0f;

                            if (
                                blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID &&
                                game->level.getRenderTile(x, y, z) == game->level.getRenderTile(x + 1, y - 1, z)
                            )
                            {
                                blockShift = -0.1f;
                            }

                            float brightness = game->level.isLavaTile(blockType) ? game->level.getTileBrightness(x, y, z) : game->level.getTileBrightness(x + 1, y, z);

                            vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, 1.0f + z, u, v, brightness * 0.6f));
                            vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, 1.0f + z, u, v2, brightness * 0.6f));
                            vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, z, u2, v2, brightness * 0.6f));

                            vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, 1.0f + z, u, v, brightness * 0.6f));
                            vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, z, u2, v2, brightness * 0.6f));
                            vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, z, u2, v, brightness * 0.6f));

                            if (game->level.isInBounds(x + 1, y, z) && blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID)
                            {
                                vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, z, u, v, brightness * 0.6f));
                                vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, z, u, v2, brightness * 0.6f));
                                vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, 1.0f + z, u2, v2, brightness * 0.6f));

                                vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, z, u, v, brightness * 0.6f));
                                vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, 1.0f + z, u2, v2, brightness * 0.6f));
                                vertices->push(VertexList::Vertex(1.0f + x, blockDefinition.height + y, 1.0f + z, u2, v, brightness * 0.6f));
                            }
                        }

                        if (shouldRenderFace(blockDefinition, Block::Definitions[game->level.getRenderTile(x - 1, y, z)]))
                        {
                            float blockShift = 0.0f;

                            if (
                                blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID &&
                                game->level.getRenderTile(x, y, z) == game->level.getRenderTile(x - 1, y - 1, z)
                            )
                            {
                                blockShift = -0.1f;
                            }

                            float brightness = game->level.isLavaTile(blockType) ? game->level.getTileBrightness(x, y, z) : game->level.getTileBrightness(x - 1, y, z);

                            vertices->push(VertexList::Vertex(x, blockDefinition.height + y, z, u, v, brightness * 0.6f));
                            vertices->push(VertexList::Vertex(x, blockShift + y, z, u, v2, brightness * 0.6f));
                            vertices->push(VertexList::Vertex(x, blockShift + y, 1.0f + z, u2, v2, brightness * 0.6f));

                            vertices->push(VertexList::Vertex(x, blockDefinition.height + y, z, u, v, brightness * 0.6f));
                            vertices->push(VertexList::Vertex(x, blockShift + y, 1.0f + z, u2, v2, brightness * 0.6f));
                            vertices->push(VertexList::Vertex(x, blockDefinition.height + y, 1.0f + z, u2, v, brightness * 0.6f));

                            if (game->level.isInBounds(x - 1, y, z) && blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID)
                            {
                                vertices->push(VertexList::Vertex(x, blockDefinition.height + y, 1.0f + z, u, v, brightness * 0.6f));
                                vertices->push(VertexList::Vertex(x, blockShift + y, 1.0f + z, u, v2, brightness * 0.6f));
                                vertices->push(VertexList::Vertex(x, blockShift + y, z, u2, v2, brightness * 0.6f));

                                vertices->push(VertexList::Vertex(x, blockDefinition.height + y, 1.0f + z, u, v, brightness * 0.6f));
                                vertices->push(VertexList::Vertex(x, blockShift + y, z, u2, v2, brightness * 0.6f));
                                vertices->push(VertexList::Vertex(x, blockDefinition.height + y, z, u2, v, brightness * 0.6f));
                            }
                        }
                    }
                }
            }
        }
    }

    vertices.update();
    waterVertices.update();
}

void Chunk::render()
{
    vertices.render();
}

void Chunk::renderWater()
{
    waterVertices.render();
}

float Chunk::distanceToPlayer() const
{
    float distanceX = game->localPlayer.position.x - (float)position.x;
    float distanceY = game->localPlayer.position.y - (float)position.y;
    float distanceZ = game->localPlayer.position.z - (float)position.z;

    return distanceX * distanceX + distanceY * distanceY + distanceZ * distanceZ;
}

bool Chunk::Comparator::operator()(const Chunk* a, const Chunk* b) const
{
    return a->distanceToPlayer() > b->distanceToPlayer();
}
