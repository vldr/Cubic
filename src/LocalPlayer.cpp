#include "LocalPlayer.h"
#include "Game.h"
#include "Level.h"
#include "Timer.h"
#include "AABBPosition.h"
#include "Particle.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void LocalPlayer::init()
{
  Entity::init();

  footSize = 0.5f;
  heightOffset = 1.62f;
  lastClick = 0;
  selectedIndex = 0;
}

void LocalPlayer::update()
{
  viewAngles.y = glm::radians(rotation.y);
  viewAngles.x = glm::radians(-rotation.x);

  lookAt.x = glm::sin(viewAngles.x) * glm::cos(viewAngles.y);
  lookAt.y = glm::sin(viewAngles.y);
  lookAt.z = glm::cos(viewAngles.x) * glm::cos(viewAngles.y);

  viewPosition = oldPosition + ((position - oldPosition) * game.timer.delta);

  game.viewMatrix = glm::lookAt(viewPosition, viewPosition + lookAt, UP);

  if (game.level.isRenderWaterTile(viewPosition.x, viewPosition.y + CAMERA_OFFSET, viewPosition.z))
  {
    game.fogColor.r = 0.02f;
    game.fogColor.g = 0.02f;
    game.fogColor.b = 0.2f;
    game.fogColor.a = 1.0f;
    game.fogDistance = 20.f;
  }
  else if (game.level.isLavaTile(viewPosition.x, viewPosition.y + CAMERA_OFFSET, viewPosition.z))
  {
    game.fogColor.r = 0.6f;
    game.fogColor.g = 0.0f;
    game.fogColor.b = 0.1f;
    game.fogColor.a = 1.0f;
    game.fogDistance = 1.f;
  }
  else
  {
    game.fogColor.r = 0.87450f;
    game.fogColor.g = 0.93725f;
    game.fogColor.b = 1.0f;
    game.fogColor.a = 1.0f; 
    game.fogDistance = 1000.0f;
  }

  interact();
}

void LocalPlayer::interact()
{
  selected = game.level.clip(viewPosition, viewPosition + lookAt * REACH);

  if (!selected.isValid && onGround)
  {
    const auto ground = glm::ivec3(viewPosition.x, viewPosition.y - 2, viewPosition.z);
    const auto groundBlockType = game.level.getTile(ground.x, ground.y, ground.z);

    if (
      !game.level.isAirTile(groundBlockType) &&
      !game.level.isWaterTile(groundBlockType) &&
      !game.level.isLavaTile(groundBlockType)
    )
    {
      selected = game.level.clip(viewPosition, viewPosition + lookAt * REACH, &ground);
    }
  }

  bool interactLeft = false;
  bool interactMiddle = false;
  bool interactRight = false;

  if (game.ui.state == UI::State::None)
  {
    if (interactState & (unsigned int)Interact::Left) { interactLeft = true; }
    if (interactState & (unsigned int)Interact::Middle) { interactMiddle = true; }
    if (interactState & (unsigned int)Interact::Right) { interactRight = true; }

    if ( 
      glm::abs(controllerState.x) > CONTROLLER_DEAD_ZONE || 
      glm::abs(controllerState.y) > CONTROLLER_DEAD_ZONE
    ) 
    {
      float adjustedControllerSpeed = 144.0f / std::max(game.lastFrameRate, uint64_t(1)) * CONTROLLER_SPEED;

      turn(controllerState.x * adjustedControllerSpeed, controllerState.y * adjustedControllerSpeed);
    }
  }

  if (game.ui.isTouch)
  {
    if (interactLeft && game.timer.ticks - lastClick < game.timer.ticksPerSecond / BUILD_SPEED * 1.5)
    {
      return;
    }
  }
  else
  {
    if (game.timer.ticks - lastClick < game.timer.ticksPerSecond / BUILD_SPEED)
    {
      return;
    }
  }

  if (interactLeft)
  {
    game.heldBlock.swing();

    lastClick = game.timer.ticks;
  }

  if (selected.isValid)
  {
    int vx = selected.x;
    int vy = selected.y;
    int vz = selected.z;

    if (interactLeft)
    {
      auto blockType = game.level.getTile(vx, vy, vz);

      if (
        selected.destructible &&
        !game.level.isAirTile(blockType)
      )
      {
        game.level.setTileWithNeighborChange(vx, vy, vz, (unsigned char)Block::Type::BLOCK_AIR, true);
        game.particleManager.spawn((float)vx, (float)vy, (float)vz, blockType);

        if (game.network.isConnected() && !game.network.isHost())
        {
          game.network.sendSetBlock(vx, vy, vz, (unsigned char)Block::Type::BLOCK_AIR);
        }
      }
    }
    else if (interactRight)
    {
      if (selected.face == 0) { vy--; }
      if (selected.face == 1) { vy++; }
      if (selected.face == 2) { vz--; }
      if (selected.face == 3) { vz++; }
      if (selected.face == 4) { vx--; }
      if (selected.face == 5) { vx++; }

      auto blockType = game.level.getTile(vx, vy, vz);

      auto heldBlockType = inventory[inventoryIndex];
      auto heldBlockDefinition = Block::Definitions[heldBlockType];
      auto heldBlockAABB = heldBlockDefinition.boundingBox.move(vx, vy, vz);

      if (
        blockType == (unsigned char)Block::Type::BLOCK_AIR ||
        game.level.isWaterTile(blockType) ||
        game.level.isLavaTile(blockType)
      )
      {
        if (!aabb.intersects(heldBlockAABB))
        {
          game.level.setTileWithNeighborChange(vx, vy, vz, heldBlockType);
          game.heldBlock.reset();

          if (game.network.isConnected() && !game.network.isHost())
          {
            game.network.sendSetBlock(vx, vy, vz, heldBlockType);
          }

          selectedIndex = 0;
        }
      }

      lastClick = game.timer.ticks;
    }
    else if (interactMiddle)
    {
      auto heldBlockType = inventory[inventoryIndex];
      auto blockType = game.level.getTile(vx, vy, vz);

      if (
        blockType != heldBlockType &&
        blockType != (unsigned char)Block::Type::BLOCK_AIR &&
        !game.level.isWaterTile(blockType) &&
        !game.level.isLavaTile(blockType)
      )
      {
        inventory[inventoryIndex] = blockType;

        game.heldBlock.update();
        game.ui.update();
      }

      lastClick = game.timer.ticks;
    }
  }
   
}

void LocalPlayer::tick()
{
  Entity::tick(); 
  
  oldBobbing = bobbing;
  oldTilt = tilt;
   
  float moveX = 0.0f;
  float moveZ = 0.0f;
  bool jumping = false;
  bool sprinting = false;

  if (game.ui.state == UI::State::None)
  {
    if (moveState & (unsigned int)Move::Backward) { moveZ = -0.98f; }
    if (moveState & (unsigned int)Move::Forward) { moveZ = 0.98f; }
    if (moveState & (unsigned int)Move::Left) { moveX = 0.98f; }
    if (moveState & (unsigned int)Move::Right) { moveX = -0.98f; }
    if (moveState & (unsigned int)Move::Jump) { jumping = true; }
    if (moveState & (unsigned int)Move::Sprint) { sprinting = true; }
  }

  if (noPhysics)
  {
    float speed = 1.0f;
    if (sprinting) { speed *= 4; }

    velocity = moveZ * speed * lookAt;
    velocity -= moveX * speed * glm::normalize(glm::cross(lookAt, UP));

    if (jumping) { velocity.y = speed; }

    move(velocity.x, velocity.y, velocity.z);

    bobbing = 0.0f;
    tilt = 0.0f;
  }
  else
  {
    if (jumping)
    {
      if (isInWater()) { velocity.y += 0.04f; }
      else if (isInLava()) { velocity.y += 0.04f; }
      else if (onGround) { velocity.y = 0.42f; }
    }

    if (isInWater() || isInLava())
    {
      moveRelative(moveX, moveZ, 0.02f);
      move(velocity.x, velocity.y, velocity.z);

      float speed;

      if (isInWater())
      {
        speed = 0.8f;
      }
      else
      {
        speed = 0.5f;
      }

      velocity.x *= speed;
      velocity.y *= speed;
      velocity.z *= speed;
      velocity.y -= 0.02f;

      if (jumping && horizontalCollision && isFree(velocity.x, velocity.y + 0.6f, velocity.z))
      {
        velocity.y = 0.3f;
      }
    }
    else
    {
      moveRelative(moveX, moveZ, onGround ? 0.1f : 0.02f);
      move(velocity.x, velocity.y, velocity.z);

      velocity.x *= 0.91f;
      velocity.y *= 0.98f;
      velocity.z *= 0.91f;
      velocity.y -= 0.08f;

      if (onGround)
      {
        velocity.x *= 0.6f;
        velocity.z *= 0.6f;
      }
    }

    float bob = glm::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
    float tilt = glm::atan(-velocity.y * 0.2f) * 15.0f;
    if (bob > 0.1f) { bob = 0.1f; }

    if (!onGround) { bob = 0.0; }
    if (onGround) { tilt = 0.0; }

    this->bobbing += (bob - this->bobbing) * 0.4f;
    this->tilt += (tilt - this->tilt) * 0.8f;
  }
}

void LocalPlayer::input(const SDL_Event& event)
{
  if (event.type == SDL_KEYDOWN)
  {
    if (game.ui.state == UI::State::None)
    {
      if (event.key.keysym.sym == SDLK_r)
      {
        setPosition(
          game.level.spawn.x,
          game.level.spawn.y,
          game.level.spawn.z
        );
      }

      if (event.key.keysym.sym == SDLK_RETURN)
      {
        game.level.spawn = position;
      }

      if (event.key.keysym.sym == SDLK_v)
      {
        noPhysics = !noPhysics;
      }
    }

    if (event.key.keysym.sym >= SDLK_1 && event.key.keysym.sym <= SDLK_9)
    {
      const auto newInventoryIndex = event.key.keysym.sym - SDLK_1;

      if (inventoryIndex != newInventoryIndex)
      {
        inventoryIndex = newInventoryIndex;

        game.heldBlock.update();
        game.ui.update();
      }
    }

    if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_LEFT)
      moveState |= (unsigned int)Move::Left;

    if (event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_RIGHT)
      moveState |= (unsigned int)Move::Right;

    if (event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDLK_UP)
      moveState |= (unsigned int)Move::Forward;

    if (event.key.keysym.sym == SDLK_s || event.key.keysym.sym == SDLK_DOWN)
      moveState |= (unsigned int)Move::Backward;

    if (event.key.keysym.sym == SDLK_SPACE)
      moveState |= (unsigned int)Move::Jump;

    if (event.key.keysym.sym == SDLK_LSHIFT)
      moveState |= (unsigned int)Move::Sprint;
  }
  else if (event.type == SDL_KEYUP)
  {
    if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_LEFT)
      moveState &= ~(unsigned int)Move::Left;

    if (event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_RIGHT)
      moveState &= ~(unsigned int)Move::Right;

    if (event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDLK_UP)
      moveState &= ~(unsigned int)Move::Forward;

    if (event.key.keysym.sym == SDLK_s || event.key.keysym.sym == SDLK_DOWN)
      moveState &= ~(unsigned int)Move::Backward;

    if (event.key.keysym.sym == SDLK_SPACE)
      moveState &= ~(unsigned int)Move::Jump;

    if (event.key.keysym.sym == SDLK_LSHIFT)
      moveState &= ~(unsigned int)Move::Sprint;
  }
  else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.which != SDL_TOUCH_MOUSEID)
  {
    if (event.button.button == SDL_BUTTON_LEFT)
      interactState |= (unsigned int)Interact::Left;

    if (event.button.button == SDL_BUTTON_MIDDLE)
      interactState |= (unsigned int)Interact::Middle;

    if (event.button.button == SDL_BUTTON_RIGHT)
      interactState |= (unsigned int)Interact::Right;
  }
  else if (event.type == SDL_MOUSEBUTTONUP && event.button.which != SDL_TOUCH_MOUSEID)
  {
    if (event.button.button == SDL_BUTTON_LEFT)
      interactState &= ~(unsigned int)Interact::Left;

    if (event.button.button == SDL_BUTTON_MIDDLE)
      interactState &= ~(unsigned int)Interact::Middle;

    if (event.button.button == SDL_BUTTON_RIGHT)
      interactState &= ~(unsigned int)Interact::Right;
  }
  else if (event.type == SDL_CONTROLLERBUTTONDOWN)
  {
    if (event.jbutton.button == SDL_CONTROLLER_BUTTON_A)
    {
      moveState |= (unsigned int)Move::Jump;
    }
    else if (event.jbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
    {
      nextInventorySlot();
    }
    else if (event.jbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER)
    {
      previousInventorySlot();
    }
    else if (event.jbutton.button == SDL_CONTROLLER_BUTTON_LEFTSTICK)
    {
      noPhysics = !noPhysics;
    }
    else if (event.jbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSTICK)
    {
      interactState |= (unsigned int)Interact::Middle;
    }
  }
  else if (event.type == SDL_CONTROLLERBUTTONUP)
  {
    if (event.jbutton.button == SDL_CONTROLLER_BUTTON_A)
    {
      moveState &= ~(unsigned int)Move::Jump;
    }
    else if (event.jbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSTICK)
    {
      interactState &= ~(unsigned int)Interact::Middle;
    }
  }
  else if (event.type == SDL_CONTROLLERAXISMOTION)
  {
    switch (event.caxis.axis)
    {
    case SDL_CONTROLLER_AXIS_LEFTY:
      if (event.caxis.value < -CONTROLLER_Y_OFFSET)
      {
        moveState |= (unsigned int)Move::Forward;
      }
      else if (event.caxis.value > CONTROLLER_Y_OFFSET)
      {
        moveState |= (unsigned int)Move::Backward;
      }
      else
      {
        moveState &= ~((unsigned int)Move::Forward | (unsigned int)Move::Backward);
      }

      break;
    case SDL_CONTROLLER_AXIS_LEFTX:
      if (event.caxis.value < -CONTROLLER_X_OFFSET)
      {
        moveState |= (unsigned int)Move::Left;

      }
      else if (event.caxis.value > CONTROLLER_X_OFFSET)
      {
        moveState |= (unsigned int)Move::Right;
      }
      else
      {
        moveState &= ~((unsigned int)Move::Left | (unsigned int)Move::Right);
      }

      break;
    case SDL_CONTROLLER_AXIS_RIGHTY:
      controllerState.y = float(event.caxis.value) / float(SHRT_MAX);
      break;
    case SDL_CONTROLLER_AXIS_RIGHTX:
      controllerState.x = float(event.caxis.value) / float(SHRT_MAX);
      break;
    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
      if (event.caxis.value > CONTROLLER_TRIGGER_OFFSET)
      {
        interactState |= (unsigned int)Interact::Right;
      }
      else
      {
        interactState &= ~(unsigned int)Interact::Right;
      }

      break;
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
      if (event.caxis.value > CONTROLLER_TRIGGER_OFFSET)
      {
        interactState |= (unsigned int)Interact::Left;
      }
      else
      {
        interactState &= ~(unsigned int)Interact::Left;
      }

      break;
    }
  }
  else if (event.type == SDL_MOUSEWHEEL)
  {
    if (event.wheel.y < 0)
    {
      previousInventorySlot();
    }
    else if (event.wheel.y > 0)
    {
      nextInventorySlot();
    }
  }
  else if (event.type == SDL_MOUSEMOTION)
  {
    turn(
      (float)event.motion.xrel * 0.15f,
      (float)event.motion.yrel * 0.15f
    );
  }
}

void LocalPlayer::turn(float rx, float ry)
{
  if (game.ui.state != UI::State::None)
  {
    return;
  }

  Entity::turn(rx, ry);
}

void LocalPlayer::previousInventorySlot()
{
  if (game.ui.state != UI::State::None)
  {
    return;
  }

  inventoryIndex--;
  if (inventoryIndex < 0)
  {
    inventoryIndex = LocalPlayer::INVENTORY_SIZE - 1;
  }

  game.heldBlock.update();
  game.ui.update();
}

void LocalPlayer::nextInventorySlot()
{
  if (game.ui.state != UI::State::None)
  {
    return;
  }

  inventoryIndex = (inventoryIndex + 1) % LocalPlayer::INVENTORY_SIZE;

  game.heldBlock.update();
  game.ui.update();
}

void LocalPlayer::setPosition(float x, float y, float z)
{
  Entity::setPosition(x, y, z);
  Entity::move(0.0f, -heightOffset, 0.0f);
}