#include "Particle.h"
#include "Game.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

void Particle::init( 
  float x, float y, float z, 
  float xd, float yd, float zd, 
  unsigned char blockType
) 
{
  Entity::init();

  setSize(0.2f, 0.2f);
  setPosition(x, y, z);

  heightOffset = aabbHeight / 2.0f;

  velocity.x = xd + float(game.random.uniform() * 2.0f - 1.0f) * 0.4f;
  velocity.y = yd + float(game.random.uniform() * 2.0f - 1.0f) * 0.4f;
  velocity.z = zd + float(game.random.uniform() * 2.0f - 1.0f) * 0.4f;

  size = 0.1f * ((float)game.random.uniform() * 0.5f + 0.5f);
  maxAge = int(4.0f / (game.random.uniform() * 0.9f + 0.1f));
  age = 0;

  float speed = float(game.random.uniform() + game.random.uniform() + 1.0f) * 0.15f * 0.4f / glm::length(velocity);
  velocity.x *= speed;
  velocity.y *= speed;
  velocity.z *= speed;
  velocity.y += 0.1f;

  unsigned char texture = Block::Definitions[blockType].sideTexture;
  float u = (float)game.random.uniform() * 3.0f;
  float v = (float)game.random.uniform() * 3.0f;

  u0 = (texture % 16 + u / 4.0f) / 16.0f;
  v0 = (texture / 16 + v / 4.0f) / 16.0f;
  u1 = u0 + 0.015609375f;
  v1 = v0 + 0.015609375f;
  brightness = game.level.getTileBrightness((int)x, (int)y, (int)z);
}

void Particle::tick()
{
  Entity::tick();

  age++;
  velocity.y -= 0.04f;

  move(velocity.x, velocity.y, velocity.z);

  velocity.x *= 0.98f;
  velocity.y *= 0.98f;
  velocity.z *= 0.98f;

  if (onGround) 
  {
    velocity.x *= 0.7f;
    velocity.z *= 0.7f;
  }
}

void Particle::update(VertexList& vertexList)
{
  if (age >= maxAge)
  {
    return;
  }
  
  const auto viewPosition = oldPosition + ((position - oldPosition) * game.timer.delta);

  const float x = -glm::cos(glm::radians(game.localPlayer.rotation.x));
  const float z = -glm::sin(glm::radians(game.localPlayer.rotation.x));
  const float y = glm::cos(glm::radians(game.localPlayer.rotation.y));

  const float rotX = -z * glm::sin(glm::radians(game.localPlayer.rotation.y));
  const float rotZ = x * glm::sin(glm::radians(game.localPlayer.rotation.y));

  vertexList.push(viewPosition.x - x * size + rotX * size, viewPosition.y + y * size, viewPosition.z - z * size + rotZ * size, u0, v0, brightness);
  vertexList.push(viewPosition.x - x * size - rotX * size, viewPosition.y - y * size, viewPosition.z - z * size - rotZ * size, u0, v1, brightness);
  vertexList.push(viewPosition.x + x * size - rotX * size, viewPosition.y - y * size, viewPosition.z + z * size - rotZ * size, u1, v1, brightness);

  vertexList.push(viewPosition.x - x * size + rotX * size, viewPosition.y + y * size, viewPosition.z - z * size + rotZ * size, u0, v0, brightness);
  vertexList.push(viewPosition.x + x * size - rotX * size, viewPosition.y - y * size, viewPosition.z + z * size - rotZ * size, u1, v1, brightness);
  vertexList.push(viewPosition.x + x * size + rotX * size, viewPosition.y + y * size, viewPosition.z + z * size + rotZ * size, u1, v0, brightness);
}