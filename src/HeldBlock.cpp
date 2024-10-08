#include "HeldBlock.h"
#include "Block.h"
#include "Game.h"
#include "LocalPlayer.h"
#include "VertexList.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void HeldBlock::init()
{
  swingOffset = 0;
  isSwinging = false;
  height = 1.0f;
  vertices.init(36);

  update();
}

void HeldBlock::update()
{
  auto blockType = game.localPlayer.inventory[game.localPlayer.inventoryIndex];
  auto blockDefinition = Block::Definitions[blockType];

  float uTop = 0.0625f * (blockDefinition.topTexture % 16);
  float vTop = 0.0625f * (blockDefinition.topTexture / 16);
  float uTop2 = 0.0625f + 0.0625f * (blockDefinition.topTexture % 16);
  float vTop2 = 0.0625f + 0.0625f * (blockDefinition.topTexture / 16);

  float uBottom = 0.0625f * (blockDefinition.bottomTexture % 16);
  float vBottom = 0.0625f * (blockDefinition.bottomTexture / 16);
  float uBottom2 = 0.0625f + 0.0625f * (blockDefinition.bottomTexture % 16);
  float vBottom2 = 0.0625f + 0.0625f * (blockDefinition.bottomTexture / 16);

  float u = 0.0625f * (blockDefinition.sideTexture % 16);
  float v = 0.0625f * (blockDefinition.sideTexture / 16) + (0.0625f - (0.0625f * blockDefinition.height));
  float u2 = 0.0625f + 0.0625f * (blockDefinition.sideTexture % 16);
  float v2 = 0.0625f + 0.0625f * (blockDefinition.sideTexture / 16);

  if (blockDefinition.draw == Block::DrawType::DRAW_SPRITE)
  {
    vertices.push(0.0f, 1.5f, 0.0f, u, v, 1.0f);
    vertices.push(0.0f, 0.5f, 0.0f, u, v2, 1.0f);
    vertices.push(1.0f, 0.5f, 1.0f, u2, v2, 1.0f);
                         
    vertices.push(0.0f, 1.5f, 0.0f, u, v, 1.0f);
    vertices.push(1.0f, 0.5f, 1.0f, u2, v2, 1.0f);
    vertices.push(1.0f, 1.5f, 1.0f, u2, v, 1.0f);
                        
    vertices.push(0.0f, 1.5f, 1.0f, u, v, 1.0f);
    vertices.push(0.0f, 0.5f, 1.0f, u, v2, 1.0f);
    vertices.push(1.0f, 0.5f, 0.0f, u2, v2, 1.0f);
                         
    vertices.push(0.0f, 1.5f, 1.0f, u, v, 1.0f);
    vertices.push(1.0f, 0.5f, 0.0f, u2, v2, 1.0f);
    vertices.push(1.0f, 1.5f, 0.0f, u2, v, 1.0f);
                          
    vertices.push(1.0f, 1.5f, 1.0f, u, v, 1.0f);
    vertices.push(1.0f, 0.5f, 1.0f, u, v2, 1.0f);
    vertices.push(0.0f, 0.5f, 0.0f, u2, v2, 1.0f);
                         
    vertices.push(1.0f, 1.5f, 1.0f, u, v, 1.0f);
    vertices.push(0.0f, 0.5f, 0.0f, u2, v2, 1.0f);
    vertices.push(0.0f, 1.5f, 0.0f, u2, v, 1.0f);
                        
    vertices.push(1.0f, 1.5f, 0.0f, u, v, 1.0f);
    vertices.push(1.0f, 0.5f, 0.0f, u, v2, 1.0f);
    vertices.push(0.0f, 0.5f, 1.0f, u2, v2, 1.0f);
                         
    vertices.push(1.0f, 1.5f, 0.0f, u, v, 1.0f);
    vertices.push(0.0f, 0.5f, 1.0f, u2, v2, 1.0f);
    vertices.push(0.0f, 1.5f, 1.0f, u2, v, 1.0f);
  }
  else
  {
    vertices.push(0.0f, blockDefinition.height, 0.0f, uTop, vTop, 1.0f);
    vertices.push(0.0f, blockDefinition.height, 1.0f, uTop, vTop2, 1.0f);
    vertices.push(1.0f, blockDefinition.height, 1.0f, uTop2, vTop2, 1.0f);

    vertices.push(0.0f, blockDefinition.height, 0.0f, uTop, vTop, 1.0f);
    vertices.push(1.0f, blockDefinition.height, 1.0f, uTop2, vTop2, 1.0f);
    vertices.push(1.0f, blockDefinition.height, 0.0f, uTop2, vTop, 1.0f);

    vertices.push(0.0f, 0.0f, 1.0f, uBottom, vBottom, 0.5f);
    vertices.push(0.0f, 0.0f, 0.0f, uBottom, vBottom2, 0.5f);
    vertices.push(1.0f, 0.0f, 0.0f, uBottom2, vBottom2, 0.5f);

    vertices.push(0.0f, 0.0f, 1.0f, uBottom, vBottom, 0.5f);
    vertices.push(1.0f, 0.0f, 0.0f, uBottom2, vBottom2, 0.5f);
    vertices.push(1.0f, 0.0f, 1.0f, uBottom2, vBottom, 0.5f);

    vertices.push(0.0f, blockDefinition.height, 1.0f, u, v, 0.8f);
    vertices.push(0.0f, 0.0f, 1.0f, u, v2, 0.8f);
    vertices.push(1.0f, 0.0f, 1.0f, u2, v2, 0.8f);

    vertices.push(0.0f, blockDefinition.height, 1.0f, u, v, 0.8f);
    vertices.push(1.0f, 0.0f, 1.0f, u2, v2, 0.8f);
    vertices.push(1.0f, blockDefinition.height, 1.0f, u2, v, 0.8f);

    vertices.push(1.0f, blockDefinition.height, 0.0f, u, v, 0.8f);
    vertices.push(1.0f, 0.0f, 0.0f, u, v2, 0.8f);
    vertices.push(0.0f, 0.0f, 0.0f, u2, v2, 0.8f);

    vertices.push(1.0f, blockDefinition.height, 0.0f, u, v, 0.8f);
    vertices.push(0.0f, 0.0f, 0.0f, u2, v2, 0.8f);
    vertices.push(0.0f, blockDefinition.height, 0.0f, u2, v, 0.8f);

    vertices.push(1.0f, blockDefinition.height, 1.0f, u, v, 0.6f);
    vertices.push(1.0f, 0.0f, 1.0f, u, v2, 0.6f);
    vertices.push(1.0f, 0.0f, 0.0f, u2, v2, 0.6f);

    vertices.push(1.0f, blockDefinition.height, 1.0f, u, v, 0.6f);
    vertices.push(1.0f, 0.0f, 0.0f, u2, v2, 0.6f);
    vertices.push(1.0f, blockDefinition.height, 0.0f, u2, v, 0.6f);

    vertices.push(0.0f, blockDefinition.height, 0.0f, u, v, 0.6f);
    vertices.push(0.0f, 0.0f, 0.0f, u, v2, 0.6f);
    vertices.push(0.0f, 0.0f, 1.0f, u2, v2, 0.6f);

    vertices.push(0.0f, blockDefinition.height, 0.0f, u, v, 0.6f);
    vertices.push(0.0f, 0.0f, 1.0f, u2, v2, 0.6f);
    vertices.push(0.0f, blockDefinition.height, 1.0f, u2, v, 0.6f);
  }

  vertices.update();

  reset();
}

void HeldBlock::swing()
{
  swingOffset = 0;
  isSwinging = true;
}

void HeldBlock::reset()
{
  height = 0.0f;
}

void HeldBlock::tick()
{
  lastHeight = height;

  float heightDelta = 1.0f - height;
  if (heightDelta < -0.4f) { heightDelta = -0.4f; }
  if (heightDelta > 0.4f) { heightDelta = 0.4f; }
  height += heightDelta;

  if (isSwinging) 
  {
    swingOffset++;

    if (swingOffset == 7) 
    {
      swingOffset = 0;
      isSwinging = false;
    }
  }
}

void HeldBlock::render()
{
  float height = this->lastHeight + (this->height - this->lastHeight) * game.timer.delta;
  float walk = game.localPlayer.walkDistance + (game.localPlayer.walkDistance - game.localPlayer.oldWalkDistance) * game.timer.delta;
  float bob = game.localPlayer.oldBobbing + (game.localPlayer.bobbing - game.localPlayer.oldBobbing) * game.timer.delta;
  float tilt = game.localPlayer.oldTilt + (game.localPlayer.tilt - game.localPlayer.oldTilt) * game.timer.delta;
  
  auto matrix = game.IDENTITY_MATRIX;
  matrix = glm::translate(matrix, glm::vec3(0.45f, -0.55f - (0.25f * (1.0f - height)), -0.8f));
  matrix = glm::translate(matrix, glm::vec3(glm::sin(walk * M_PI) * bob * 0.5, -glm::abs(glm::cos(walk * M_PI) * bob), 0.0));
  matrix = glm::translate(matrix, glm::vec3(0, glm::radians(tilt), 0));

  if (isSwinging)
  {
    float swingOffsetDelta = (swingOffset + game.timer.delta) / 7.0f;
    matrix = glm::translate(matrix, glm::vec3(-glm::sin(glm::sqrt(swingOffsetDelta) * M_PI) * 0.4, glm::sin(glm::sqrt(swingOffsetDelta) * M_PI * 2.0) * 0.2, -glm::sin(swingOffsetDelta * M_PI) * 0.2));
  }

  matrix = glm::scale(matrix, glm::vec3(0.3f, 0.3f, 0.3f));
  matrix = glm::rotate(matrix, glm::radians(-40.0f), glm::vec3(0, 1, 0));
  matrix = glm::rotate(matrix, glm::radians(glm::sin(walk * (float)M_PI) * bob * 3.0f), glm::vec3(0.0, 0.0, 1.0));
  matrix = glm::rotate(matrix, glm::radians(glm::abs(glm::cos(walk * (float)M_PI + 0.2f) * bob) * 5.0f), glm::vec3(1.0, 0.0, 0.0));

  if (isSwinging) 
  {
    float swingOffsetDelta = (swingOffset + game.timer.delta) / 7.0f;
    matrix = glm::rotate(matrix, glm::radians(glm::sin(glm::sqrt(swingOffsetDelta) * (float)M_PI) * 80.0f), glm::vec3(0.0, 1.0, 0.0));
    matrix = glm::rotate(matrix, glm::radians(-glm::sin(swingOffsetDelta * swingOffsetDelta * (float)M_PI)), glm::vec3(1.0, 0.0, 0.0));
  }

  glUniformMatrix4fv(game.modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(matrix));

  glBindTexture(GL_TEXTURE_2D, game.atlasTexture);
  vertices.render();

  glUniformMatrix4fv(game.modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(game.IDENTITY_MATRIX));
}
