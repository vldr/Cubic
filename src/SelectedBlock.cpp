#include "SelectedBlock.h"
#include "Game.h"
#include "LocalPlayer.h"

void SelectedBlock::init()
{
  texture = game.textureManager.loadColor(0.0f, 0.0f, 0.0f);

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, BUFFER_SIZE * sizeof(glm::vec3), NULL, GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(game.positionAttribute);

  glVertexAttribPointer(game.positionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glVertexAttrib2f(game.uvAttribute, 0.0f, 1.0f);
  glVertexAttrib1f(game.shadeAttribute, 0.0f);
}

void SelectedBlock::renderPost()
{
  if (game.localPlayer.selected.isValid)
  {
    auto blockType = game.level.getTile(game.localPlayer.selected.x, game.localPlayer.selected.y, game.localPlayer.selected.z);

    if (blockType != (unsigned char)Block::Type::BLOCK_AIR)
    {
      glBindTexture(GL_TEXTURE_2D, texture);
      glBindVertexArray(vao);

      if (game.localPlayer.selectedIndex != game.localPlayer.selected.index)
      {
        AABB aabb = Block::Definitions[blockType].boundingBox;
        aabb = aabb.move((float)game.localPlayer.selected.x, (float)game.localPlayer.selected.y, (float)game.localPlayer.selected.z);
        aabb = aabb.grow(0.002f, 0.002f, 0.002f);

        float vertices[BUFFER_SIZE * 3] =
        {
          aabb.x0, aabb.y0, aabb.z0,
          aabb.x0, aabb.y0, aabb.z1,
          aabb.x1, aabb.y0, aabb.z0,
          aabb.x1, aabb.y0, aabb.z1,
          aabb.x0, aabb.y1, aabb.z0,
          aabb.x0, aabb.y1, aabb.z1,
          aabb.x1, aabb.y1, aabb.z0,
          aabb.x1, aabb.y1, aabb.z1,
          aabb.x0, aabb.y0, aabb.z0,
          aabb.x1, aabb.y0, aabb.z0,
          aabb.x0, aabb.y0, aabb.z1,
          aabb.x1, aabb.y0, aabb.z1,
          aabb.x0, aabb.y1, aabb.z0,
          aabb.x1, aabb.y1, aabb.z0,
          aabb.x0, aabb.y1, aabb.z1,
          aabb.x1, aabb.y1, aabb.z1,
          aabb.x0, aabb.y0, aabb.z0,
          aabb.x0, aabb.y1, aabb.z0,
          aabb.x1, aabb.y0, aabb.z0,
          aabb.x1, aabb.y1, aabb.z0,
          aabb.x1, aabb.y0, aabb.z1,
          aabb.x1, aabb.y1, aabb.z1,
          aabb.x0, aabb.y0, aabb.z1,
          aabb.x0, aabb.y1, aabb.z1,
        };

        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        game.localPlayer.selectedIndex = game.localPlayer.selected.index;
      }

      glDrawArrays(GL_LINES, 0, (GLsizei)BUFFER_SIZE);
    }
  }
}
