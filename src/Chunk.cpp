#include "Chunk.h"
#include "Level.h"
#include "Game.h"
#include "LocalPlayer.h"

Chunk::Face Chunk::topFaces[Chunk::SIZE * Chunk::SIZE * Chunk::SIZE];
Chunk::Face Chunk::bottomFaces[Chunk::SIZE * Chunk::SIZE * Chunk::SIZE];
Chunk::Face Chunk::leftFaces[Chunk::SIZE * Chunk::SIZE * Chunk::SIZE];
Chunk::Face Chunk::rightFaces[Chunk::SIZE * Chunk::SIZE * Chunk::SIZE];
Chunk::Face Chunk::frontFaces[Chunk::SIZE * Chunk::SIZE * Chunk::SIZE];
Chunk::Face Chunk::backFaces[Chunk::SIZE * Chunk::SIZE * Chunk::SIZE];

void Chunk::init(Game* game, int x, int y, int z)
{
    this->game = game;
    this->position = glm::ivec3(x, y, z);
    this->isVisible = false;
    this->isLoaded = false;

    this->vertices.init(game);
    this->waterVertices.init(game);
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

bool Chunk::shouldRenderTopFace(const Block::Definition& blockDefinition, int x, int y, int z)
{
    bool shouldRenderTop = shouldRenderFace(blockDefinition, Block::Definitions[game->level.getRenderTile(x, y + 1, z)]);
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

    return shouldRenderTop;
}

inline Chunk::Face& Chunk::getFace(Chunk::Face* faces, int x, int y, int z)
{
   return faces[(z * Chunk::SIZE + y) * Chunk::SIZE + x];
}

template <Chunk::FaceType faceType>
inline void Chunk::generateMesh(Face* faces)
{
    for (int slice = 0; slice < Chunk::SIZE; slice++)
    {
        for (int column = 0; column < Chunk::SIZE; column++)
        {
            for (int row = 0; row < Chunk::SIZE; row++)
            {
                Face face;

                if constexpr (faceType == FaceType::Top || faceType == FaceType::Bottom)
                {
                    face = getFace(faces, column, slice, row);
                }
                else if constexpr (faceType == FaceType::Front || faceType == FaceType::Back)
                {
                    face = getFace(faces, column, row, slice);
                }
                else if constexpr (faceType == FaceType::Left || faceType == FaceType::Right)
                {
                    face = getFace(faces, slice, row, column);
                }
                
                if (!face.valid)
                {
                    continue;
                }

                int width = 1;
                int height = 1;

                for (int innerRow = row + 1; innerRow < Chunk::SIZE; innerRow++)
                {
                    Face previousFace;
                    Face face;

                    if constexpr (faceType == FaceType::Top || faceType == FaceType::Bottom)
                    {
                        previousFace = getFace(faces, column, slice, innerRow - 1);
                        face = getFace(faces, column, slice, innerRow);
                    }
                    else if constexpr (faceType == FaceType::Front || faceType == FaceType::Back)
                    {
                        previousFace = getFace(faces, column, innerRow - 1, slice);
                        face = getFace(faces, column, innerRow, slice);
                    }
                    else if constexpr (faceType == FaceType::Left || faceType == FaceType::Right)
                    {
                        previousFace = getFace(faces, slice, innerRow - 1, column);
                        face = getFace(faces, slice, innerRow, column);
                    }

                    if (previousFace == face)
                    {
                        if constexpr (faceType == FaceType::Front || faceType == FaceType::Back || faceType == FaceType::Left || faceType == FaceType::Right)
                        {
                            if (face.blockDefinition.height != 1.0f)
                            {
                                break;
                            }
                        }

                        width++;
                    }
                    else
                    {
                        break;
                    }
                }

                for (int innerColumn = column + 1; innerColumn < Chunk::SIZE; innerColumn++)
                {
                    int innerWidth = 0;

                    Face innerFace;

                    if constexpr (faceType == FaceType::Top || faceType == FaceType::Bottom)
                    {
                        innerFace = getFace(faces, innerColumn, slice, row);
                    }
                    else if constexpr (faceType == FaceType::Front || faceType == FaceType::Back)
                    {
                        innerFace = getFace(faces, innerColumn, row, slice);
                    }
                    else if constexpr (faceType == FaceType::Left || faceType == FaceType::Right)
                    {
                        innerFace = getFace(faces, slice, row, innerColumn);
                    }

                    if (face == innerFace)
                    {
                        innerWidth = 1;

                        for (int innerRow = row + 1; innerRow < row + width; innerRow++)
                        {
                            Face previousFace;
                            Face face;

                            if constexpr (faceType == FaceType::Top || faceType == FaceType::Bottom)
                            {
                                previousFace = getFace(faces, innerColumn, slice, innerRow - 1);
                                face = getFace(faces, innerColumn, slice, innerRow);
                            }
                            else if constexpr (faceType == FaceType::Front || faceType == FaceType::Back)
                            {
                                previousFace = getFace(faces, innerColumn, innerRow - 1, slice);
                                face = getFace(faces, innerColumn, innerRow, slice);
                            }
                            else if constexpr (faceType == FaceType::Left || faceType == FaceType::Right)
                            {
                                previousFace = getFace(faces, slice, innerRow - 1, innerColumn);
                                face = getFace(faces, slice, innerRow, innerColumn);
                            }

                            if (previousFace == face)
                            {
                                innerWidth++;
                            }
                            else
                            {
                                break;
                            }
                        }
                    }

                    if (innerWidth == width)
                    {
                        height++;
                    }
                    else
                    {
                        break;

                    }
                }

                for (int innerColumn = column; innerColumn < column + height; innerColumn++)
                {
                    for (int innerRow = row; innerRow < row + width; innerRow++)
                    {
                        if constexpr (faceType == FaceType::Top || faceType == FaceType::Bottom)
                        {
                            getFace(faces, innerColumn, slice, innerRow).valid = false;
                        }
                        else if constexpr (faceType == FaceType::Front || faceType == FaceType::Back)
                        {
                            getFace(faces, innerColumn, innerRow, slice).valid = false;
                        }
                        else if constexpr (faceType == FaceType::Left || faceType == FaceType::Right)
                        {
                            getFace(faces, slice, innerRow, innerColumn).valid = false;
                        }
                    }
                }

                const auto& blockType = face.blockType;
                const auto& blockDefinition = face.blockDefinition;
                const auto& brightness = face.brightness;
                const auto& blockShift = face.blockShift;
                const auto& mirror = face.mirror;

                VertexList* vertices;
                if (game->level.isWaterTile(blockType))
                {
                    vertices = &this->waterVertices;
                }
                else
                {
                    vertices = &this->vertices;
                }

                if constexpr (faceType == FaceType::Top || faceType == FaceType::Bottom)
                {
                    int x = position.x + column;
                    int y = position.y + slice;
                    int z = position.z + row;

                    if constexpr (faceType == FaceType::Top)
                    {
                        float u = height + 0.0625f * (blockDefinition.topTexture % 16);
                        float v = width + 0.0625f * (blockDefinition.topTexture / 16);
                        float u2 = 0.0625f + u;
                        float v2 = 0.0625f + v;

                        vertices->push(VertexList::Vertex(x, blockDefinition.height + y, z, u, v, brightness));
                        vertices->push(VertexList::Vertex(x, blockDefinition.height + y, width + z, u, v2, brightness));
                        vertices->push(VertexList::Vertex(height + x, blockDefinition.height + y, width + z, u2, v2, brightness));

                        vertices->push(VertexList::Vertex(x, blockDefinition.height + y, z, u, v, brightness));
                        vertices->push(VertexList::Vertex(height + x, blockDefinition.height + y, width + z, u2, v2, brightness));
                        vertices->push(VertexList::Vertex(height + x, blockDefinition.height + y, z, u2, v, brightness));

                        if (mirror)
                        {
                            vertices->push(VertexList::Vertex(x, blockDefinition.height + y, width + z, u, v, brightness));
                            vertices->push(VertexList::Vertex(x, blockDefinition.height + y, z, u, v2, brightness));
                            vertices->push(VertexList::Vertex(height + x, blockDefinition.height + y, z, u2, v2, brightness));

                            vertices->push(VertexList::Vertex(x, blockDefinition.height + y, width + z, u, v, brightness));
                            vertices->push(VertexList::Vertex(height + x, blockDefinition.height + y, z, u2, v2, brightness));
                            vertices->push(VertexList::Vertex(height + x, blockDefinition.height + y, width + z, u2, v, brightness));
                        }
                    }
                    else if constexpr (faceType == FaceType::Bottom)
                    {
                        float u = height + 0.0625f * (blockDefinition.bottomTexture % 16);
                        float v = width + 0.0625f * (blockDefinition.bottomTexture / 16);
                        float u2 = 0.0625f + u;
                        float v2 = 0.0625f + v;

                        vertices->push(VertexList::Vertex(x, y, width + z, u, v, brightness * 0.5f));
                        vertices->push(VertexList::Vertex(x, y, z, u, v2, brightness * 0.5f));
                        vertices->push(VertexList::Vertex(height + x, y, z, u2, v2, brightness * 0.5f));

                        vertices->push(VertexList::Vertex(x, y, width + z, u, v, brightness * 0.5f));
                        vertices->push(VertexList::Vertex(height + x, y, z, u2, v2, brightness * 0.5f));
                        vertices->push(VertexList::Vertex(height + x, y, width + z, u2, v, brightness * 0.5f));

                        if (mirror)
                        {
                            vertices->push(VertexList::Vertex(x, y, z, u, v, brightness));
                            vertices->push(VertexList::Vertex(x, y, width + z, u, v2, brightness));
                            vertices->push(VertexList::Vertex(height + x, y, width + z, u2, v2, brightness));

                            vertices->push(VertexList::Vertex(x, y, z, u, v, brightness));
                            vertices->push(VertexList::Vertex(height + x, y, width + z, u2, v2, brightness));
                            vertices->push(VertexList::Vertex(height + x, y, z, u2, v, brightness));
                        }
                    }      
                }
                else if constexpr (faceType == FaceType::Front || faceType == FaceType::Back)
                {
                    float u = height + 0.0625f * (blockDefinition.sideTexture % 16);
                    float v = width + 0.0625f * (blockDefinition.sideTexture / 16);
                    float u2 = 0.0625f + u;
                    float v2 = blockDefinition.height * 0.0625f + v;

                    int x = position.x + column;
                    int y = position.y + row;
                    int z = position.z + slice;

                    if constexpr (faceType == FaceType::Front)
                    {
                        vertices->push(VertexList::Vertex(x, width * blockDefinition.height + y, 1.0f + z, u, v, brightness * 0.8f));
                        vertices->push(VertexList::Vertex(x, blockShift + y, 1.0f + z, u, v2, brightness * 0.8f));
                        vertices->push(VertexList::Vertex(height + x, blockShift + y, 1.0f + z, u2, v2, brightness * 0.8f));

                        vertices->push(VertexList::Vertex(x, width * blockDefinition.height + y, 1.0f + z, u, v, brightness * 0.8f));
                        vertices->push(VertexList::Vertex(height + x, blockShift + y, 1.0f + z, u2, v2, brightness * 0.8f));
                        vertices->push(VertexList::Vertex(height + x, width * blockDefinition.height + y, 1.0f + z, u2, v, brightness * 0.8f));

                        if (mirror)
                        {
                            vertices->push(VertexList::Vertex(height + x, width * blockDefinition.height + y, 1.0f + z, u, v, brightness * 0.8f));
                            vertices->push(VertexList::Vertex(height + x, blockShift + y, 1.0f + z, u, v2, brightness * 0.8f));
                            vertices->push(VertexList::Vertex(x, blockShift + y, 1.0f + z, u2, v2, brightness * 0.8f));

                            vertices->push(VertexList::Vertex(height + x, width * blockDefinition.height + y, 1.0f + z, u, v, brightness * 0.8f));
                            vertices->push(VertexList::Vertex(x, blockShift + y, 1.0f + z, u2, v2, brightness * 0.8f));
                            vertices->push(VertexList::Vertex(x, width * blockDefinition.height + y, 1.0f + z, u2, v, brightness * 0.8f));
                        }
                    }
                    else if constexpr (faceType == FaceType::Back)
                    {
                        vertices->push(VertexList::Vertex(height + x, width * blockDefinition.height + y, z, u, v, brightness * 0.8f));
                        vertices->push(VertexList::Vertex(height + x, blockShift + y, z, u, v2, brightness * 0.8f));
                        vertices->push(VertexList::Vertex(x, blockShift + y, z, u2, v2, brightness * 0.8f));

                        vertices->push(VertexList::Vertex(height + x, width * blockDefinition.height + y, z, u, v, brightness * 0.8f));
                        vertices->push(VertexList::Vertex(x, blockShift + y, z, u2, v2, brightness * 0.8f));
                        vertices->push(VertexList::Vertex(x, width * blockDefinition.height + y, z, u2, v, brightness * 0.8f));
                        
                        if (mirror)
                        {
                            vertices->push(VertexList::Vertex(x, width * blockDefinition.height + y, z, u, v, brightness * 0.8f));
                            vertices->push(VertexList::Vertex(x, blockShift + y, z, u, v2, brightness * 0.8f));
                            vertices->push(VertexList::Vertex(height + x, blockShift + y, z, u2, v2, brightness * 0.8f));

                            vertices->push(VertexList::Vertex(x, width * blockDefinition.height + y, z, u, v, brightness * 0.8f));
                            vertices->push(VertexList::Vertex(height + x, blockShift + y, z, u2, v2, brightness * 0.8f));
                            vertices->push(VertexList::Vertex(height + x, width * blockDefinition.height + y, z, u2, v, brightness * 0.8f));
                        }
                    }
                }
                else if constexpr (faceType == FaceType::Left || faceType == FaceType::Right)
                {
                    float u = height + 0.0625f * (blockDefinition.sideTexture % 16);
                    float v = width + 0.0625f * (blockDefinition.sideTexture / 16);
                    float u2 = 0.0625f + u;
                    float v2 = blockDefinition.height * 0.0625f + v;

                    int x = position.x + slice;
                    int y = position.y + row;
                    int z = position.z + column;

                    if constexpr (faceType == FaceType::Right)
                    {
                        vertices->push(VertexList::Vertex(1.0f + x, width * blockDefinition.height + y, height + z, u, v, brightness * 0.6f));
                        vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, height + z, u, v2, brightness * 0.6f));
                        vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, z, u2, v2, brightness * 0.6f));

                        vertices->push(VertexList::Vertex(1.0f + x, width * blockDefinition.height + y, height + z, u, v, brightness * 0.6f));
                        vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, z, u2, v2, brightness * 0.6f));
                        vertices->push(VertexList::Vertex(1.0f + x, width * blockDefinition.height + y, z, u2, v, brightness * 0.6f));

                        if (mirror)
                        {
                            vertices->push(VertexList::Vertex(1.0f + x, width * blockDefinition.height + y, z, u, v, brightness * 0.6f));
                            vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, z, u, v2, brightness * 0.6f));
                            vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, height + z, u2, v2, brightness * 0.6f));

                            vertices->push(VertexList::Vertex(1.0f + x, width * blockDefinition.height + y, z, u, v, brightness * 0.6f));
                            vertices->push(VertexList::Vertex(1.0f + x, blockShift + y, height + z, u2, v2, brightness * 0.6f));
                            vertices->push(VertexList::Vertex(1.0f + x, width * blockDefinition.height + y, height + z, u2, v, brightness * 0.6f));
                        }
                    }
                    else if constexpr (faceType == FaceType::Left)
                    {
                        vertices->push(VertexList::Vertex(x, width * blockDefinition.height + y, z, u, v, brightness * 0.6f));
                        vertices->push(VertexList::Vertex(x, blockShift + y, z, u, v2, brightness * 0.6f));
                        vertices->push(VertexList::Vertex(x, blockShift + y, height + z, u2, v2, brightness * 0.6f));
                                
                        vertices->push(VertexList::Vertex(x, width * blockDefinition.height + y, z, u, v, brightness * 0.6f));
                        vertices->push(VertexList::Vertex(x, blockShift + y, height + z, u2, v2, brightness * 0.6f));
                        vertices->push(VertexList::Vertex(x, width * blockDefinition.height + y, height + z, u2, v, brightness * 0.6f));

                        if (mirror)
                        {
                            vertices->push(VertexList::Vertex(x, width * blockDefinition.height + y, height + z, u, v, brightness * 0.6f));
                            vertices->push(VertexList::Vertex(x, blockShift + y, height + z, u, v2, brightness * 0.6f));
                            vertices->push(VertexList::Vertex(x, blockShift + y, z, u2, v2, brightness * 0.6f));

                            vertices->push(VertexList::Vertex(x, width * blockDefinition.height + y, height + z, u, v, brightness * 0.6f));
                            vertices->push(VertexList::Vertex(x, blockShift + y, z, u2, v2, brightness * 0.6f));
                            vertices->push(VertexList::Vertex(x, width * blockDefinition.height + y, z, u2, v, brightness * 0.6f));
                        }
                    }
                }
            }
        }
    }
}

inline void Chunk::generateFaces()
{
    for (int x = position.x; x < (position.x + SIZE); x++)
    {
        for (int y = position.y; y < (position.y + SIZE); y++)
        {
            for (int z = position.z; z < (position.z + SIZE); z++)
            {
                auto index = ((z - position.z) * SIZE + (y - position.y)) * SIZE + (x - position.x);

                topFaces[index].valid = false;
                bottomFaces[index].valid = false;
                leftFaces[index].valid = false;
                rightFaces[index].valid = false;
                frontFaces[index].valid = false;
                backFaces[index].valid = false;

                auto blockType = game->level.getRenderTile(x, y, z);
                if (!blockType)
                {
                    continue;
                }

                Block::Definition blockDefinition = Block::Definitions[blockType];
                if (blockDefinition.draw == Block::DrawType::DRAW_SPRITE)
                {
                    float u = 0.0625f * (blockDefinition.sideTexture % 16);
                    float v = 0.0625f * (blockDefinition.sideTexture / 16) + (0.0625f - (0.0625f * blockDefinition.height));
                    float u2 = 0.0625f + u;
                    float v2 = 0.0625f + v;

                    vertices.push(VertexList::Vertex(x, 1.0f + y, z, u, v, 1.0f));
                    vertices.push(VertexList::Vertex(x, y, z, u, v2, 1.0f));
                    vertices.push(VertexList::Vertex(1.0f + x, y, 1.0f + z, u2, v2, 1.0f));

                    vertices.push(VertexList::Vertex(x, 1.0f + y, z, u, v, 1.0f));
                    vertices.push(VertexList::Vertex(1.0f + x, y, 1.0f + z, u2, v2, 1.0f));
                    vertices.push(VertexList::Vertex(1.0f + x, 1.0f + y, 1.0f + z, u2, v, 1.0f));

                    vertices.push(VertexList::Vertex(x, 1.0f + y, 1.0f + z, u, v, 1.0f));
                    vertices.push(VertexList::Vertex(x, y, 1.0f + z, u, v2, 1.0f));
                    vertices.push(VertexList::Vertex(1.0f + x, y, z, u2, v2, 1.0f));

                    vertices.push(VertexList::Vertex(x, 1.0f + y, 1.0f + z, u, v, 1.0f));
                    vertices.push(VertexList::Vertex(1.0f + x, y, z, u2, v2, 1.0f));
                    vertices.push(VertexList::Vertex(1.0f + x, 1.0f + y, z, u2, v, 1.0f));

                    vertices.push(VertexList::Vertex(1.0f + x, 1.0f + y, 1.0f + z, u, v, 1.0f));
                    vertices.push(VertexList::Vertex(1.0f + x, y, 1.0f + z, u, v2, 1.0f));
                    vertices.push(VertexList::Vertex(x, y, z, u2, v2, 1.0f));

                    vertices.push(VertexList::Vertex(1.0f + x, 1.0f + y, 1.0f + z, u, v, 1.0f));
                    vertices.push(VertexList::Vertex(x, y, z, u2, v2, 1.0f));
                    vertices.push(VertexList::Vertex(x, 1.0f + y, z, u2, v, 1.0f));

                    vertices.push(VertexList::Vertex(1.0f + x, 1.0f + y, z, u, v, 1.0f));
                    vertices.push(VertexList::Vertex(1.0f + x, y, z, u, v2, 1.0f));
                    vertices.push(VertexList::Vertex(x, y, 1.0f + z, u2, v2, 1.0f));

                    vertices.push(VertexList::Vertex(1.0f + x, 1.0f + y, z, u, v, 1.0f));
                    vertices.push(VertexList::Vertex(x, y, 1.0f + z, u2, v2, 1.0f));
                    vertices.push(VertexList::Vertex(x, 1.0f + y, 1.0f + z, u2, v, 1.0f));
                }
                else
                {
                    if (shouldRenderTopFace(blockDefinition, x, y, z))
                    {
                        if (blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID)
                        {
                            blockDefinition.height = 0.9f;
                        }

                        float blockShift = 0.0f;
                        float brightness = game->level.isLavaTile(blockType) ? game->level.getTileBrightness(x, y, z) : game->level.getTileBrightness(x, y + 1, z);

                        bool mirror = game->level.isInBounds(x, y + 1, z) && blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID;

                        topFaces[index] = {
                            true,
                            blockType,
                            blockDefinition,
                            brightness,
                            blockShift,
                            mirror
                        };
                    }

                    if (shouldRenderFace(blockDefinition, Block::Definitions[game->level.getRenderTile(x, y - 1, z)], true))
                    {
                        float blockShift = 0.0f;
                        float brightness = game->level.isLavaTile(blockType) ? game->level.getTileBrightness(x, y, z) : game->level.getTileBrightness(x, y - 1, z);

                        bool mirror = game->level.isInBounds(x, y - 1, z) && blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID;

                        bottomFaces[index] = {
                            true,
                            blockType,
                            blockDefinition,
                            brightness,
                            blockShift,
                            mirror
                        };
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

                        bool mirror = game->level.isInBounds(x, y, z + 1) && blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID;

                        frontFaces[index] = {
                            true,
                            blockType,
                            blockDefinition,
                            brightness,
                            blockShift,
                            mirror
                        };
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

                        bool mirror = game->level.isInBounds(x, y, z - 1) && blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID;

                        backFaces[index] = {
                            true,
                            blockType,
                            blockDefinition,
                            brightness,
                            blockShift,
                            mirror
                        };
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

                        bool mirror = game->level.isInBounds(x + 1, y, z) && blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID;

                        rightFaces[index] = {
                            true,
                            blockType,
                            blockDefinition,
                            brightness,
                            blockShift,
                            mirror
                        };
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

                        bool mirror = game->level.isInBounds(x - 1, y, z) && blockDefinition.collide == Block::CollideType::COLLIDE_LIQUID;

                        leftFaces[index] = {
                            true,
                            blockType,
                            blockDefinition,
                            brightness,
                            blockShift,
                            mirror
                        };
                    }
                }
            }
        }
    }
}

void Chunk::update()
{ 
    generateFaces();

    generateMesh<FaceType::Top>(topFaces);
    generateMesh<FaceType::Bottom>(bottomFaces);
    generateMesh<FaceType::Front>(frontFaces);
    generateMesh<FaceType::Back>(backFaces);
    generateMesh<FaceType::Left>(leftFaces);
    generateMesh<FaceType::Right>(rightFaces);

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
