#include "LocalPlayer.h"
#include "Game.h"
#include "Level.h"
#include "Timer.h"
#include "AABBPosition.h"
#include "Particle.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void LocalPlayer::init(Game* game)
{
    Entity::init(game);

    this->footSize = 0.5f;
    this->heightOffset = 1.62f;
    this->lastClick = 0;
    this->selectedIndex = 0;
}

void LocalPlayer::update()
{
    viewAngles.y = glm::radians(rotation.x);
    viewAngles.x = glm::radians(-rotation.y);

    lookAt.x = glm::sin(viewAngles.x) * glm::cos(viewAngles.y);
    lookAt.y = glm::sin(viewAngles.y);
    lookAt.z = glm::cos(viewAngles.x) * glm::cos(viewAngles.y);

    viewPosition = oldPosition + ((position - oldPosition) * game->timer.delta);

    game->viewMatrix = glm::lookAt(viewPosition, viewPosition + lookAt, UP);

    /////////////////////////////////////////////////////

    if (game->level.isRenderWaterTile(viewPosition.x, viewPosition.y + CAMERA_OFFSET, viewPosition.z))
    {
        game->fogColor.r = 0.02f;
        game->fogColor.g = 0.02f;
        game->fogColor.b = 0.2f;
        game->fogColor.a = 1.0f;
        game->fogDistance = 20.f;
    }
    else if (game->level.isLavaTile(viewPosition.x, viewPosition.y + CAMERA_OFFSET, viewPosition.z))
    {
        game->fogColor.r = 0.6f;
        game->fogColor.g = 0.0f;
        game->fogColor.b = 0.1f;
        game->fogColor.a = 1.0f;
        game->fogDistance = 1.f;
    }
    else
    {
        game->fogColor.r = 0.87450f;
        game->fogColor.g = 0.93725f;
        game->fogColor.b = 1.0f;
        game->fogColor.a = 1.0f; 
        game->fogDistance = 1000.0f;
    }

    /////////////////////////////////////////////////////
     
    selected = game->level.clip(viewPosition, viewPosition + lookAt * REACH); 
 
    if (!selected.isValid && onGround)
    {
        const auto ground = glm::ivec3(viewPosition.x, viewPosition.y - 2, viewPosition.z);
        const auto groundBlockType = game->level.getTile(ground.x, ground.y, ground.z);

        if (
            !game->level.isAirTile(groundBlockType) && 
            !game->level.isWaterTile(groundBlockType) && 
            !game->level.isLavaTile(groundBlockType)
        )
        {
            selected = game->level.clip(viewPosition, viewPosition + lookAt * REACH, &ground);
        }
    }

    bool interactLeft = false;
    bool interactMiddle = false;
    bool interactRight = false;

    if (game->ui.state == UI::State::None)
    {
        if (interactState & Interact::Interact_Left) { interactLeft = true; }
        if (interactState & Interact::Interact_Middle) { interactMiddle = true; }
        if (interactState & Interact::Interact_Right) { interactRight = true; }
    }

    if (game->timer.ticks - lastClick >= game->timer.ticksPerSecond / BUILD_SPEED)
    {
        if (interactLeft)
        {
            game->heldBlock.swing();

            lastClick = game->timer.ticks;
        }

        if (selected.isValid)
        {
            int vx = selected.x;
            int vy = selected.y;
            int vz = selected.z;

            if (interactLeft)
            {
                auto blockType = game->level.getTile(vx, vy, vz);

                if (
                    selected.destructible &&
                    !game->level.isAirTile(blockType)
                )
                {
                    game->level.setTileWithNeighborChange(vx, vy, vz, (unsigned char)Block::Type::BLOCK_AIR, true);
                    game->particleManager.spawn((float)vx, (float)vy, (float)vz, blockType);

                    if (game->network.isConnected() && !game->network.isHost())
                    {
                        game->network.sendSetBlock(vx, vy, vz, (unsigned char)Block::Type::BLOCK_AIR);
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

                auto blockType = game->level.getTile(vx, vy, vz);

                auto heldBlockType = inventory[inventoryIndex];
                auto heldBlockDefinition = Block::Definitions[heldBlockType];
                auto heldBlockAABB = heldBlockDefinition.boundingBox.move(vx, vy, vz);

                if (
                    blockType == (unsigned char)Block::Type::BLOCK_AIR ||
                    game->level.isWaterTile(blockType) ||
                    game->level.isLavaTile(blockType)
                )
                {
                    if (!aabb.intersects(heldBlockAABB))
                    {
                        game->level.setTileWithNeighborChange(vx, vy, vz, heldBlockType);
                        game->heldBlock.reset();

                        if (game->network.isConnected() && !game->network.isHost())
                        {
                            game->network.sendSetBlock(vx, vy, vz, heldBlockType);
                        }

                        selectedIndex = 0;
                    }
                }

                lastClick = game->timer.ticks;
            }
            else if (interactMiddle)
            {
                auto heldBlockType = inventory[inventoryIndex];
                auto blockType = game->level.getTile(vx, vy, vz);

                if (
                    blockType != heldBlockType &&
                    blockType != (unsigned char)Block::Type::BLOCK_AIR &&
                    !game->level.isWaterTile(blockType) &&
                    !game->level.isLavaTile(blockType) 
                )
                {
                    inventory[inventoryIndex] = blockType;

                    game->heldBlock.update();
                    game->ui.update();
                }

                lastClick = game->timer.ticks;
            }
        }
    }
}

void LocalPlayer::tick()
{
    Entity::tick(); 
    
    oldBobbing = bobbing;
    oldTilt = tilt;
     
    float moveX = 0.0f;
    float moveY = 0.0f;
    bool jumping = false;
    bool sprinting = false;

    if (game->ui.state == UI::State::None)
    {
        if (moveState & Move::Move_Backward) { moveY = -0.98f; }
        if (moveState & Move::Move_Forward) { moveY = 0.98f; }
        if (moveState & Move::Move_Left) { moveX = 0.98f; }
        if (moveState & Move::Move_Right) { moveX = -0.98f; }
        if (moveState & Move::Move_Jump) { jumping = true; }
        if (moveState & Move::Move_Sprint) { sprinting = true; }
    }

    if (noPhysics)
    {
        float speed = 1.0f;
        if (sprinting) { speed *= 4; }

        velocity = moveY * speed * lookAt;
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

        if (isInWater())
        {
            moveRelative(moveX, moveY, 0.02f);
            move(velocity.x, velocity.y, velocity.z);

            velocity.x *= 0.8f;
            velocity.y *= 0.8f;
            velocity.z *= 0.8f;
            velocity.y -= 0.02f;

            if (horizontalCollision && isFree(velocity.x, velocity.y + 0.6f, velocity.z))
            {
                velocity.y = 0.3f;
            }
        }
        else if (isInLava())
        {
            moveRelative(moveX, moveY, 0.02f);
            move(velocity.x, velocity.y, velocity.z);

            velocity.x *= 0.5f;
            velocity.y *= 0.5f;
            velocity.z *= 0.5f;
            velocity.y -= 0.02f;

            if (horizontalCollision && isFree(velocity.x, velocity.y + 0.6f, velocity.z))
            {
                velocity.y = 0.3f;
            }
        }
        else
        {
            moveRelative(moveX, moveY, onGround ? 0.1f : 0.02f);
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
        if (game->ui.state == UI::State::None)
        {
            if (event.key.keysym.sym == SDLK_r)
            {
                setPosition(
                    game->level.spawn.x,
                    game->level.spawn.y,
                    game->level.spawn.z
                );
            }

            if (event.key.keysym.sym == SDLK_RETURN)
            {
                game->level.spawn = position;
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

                game->heldBlock.update();
                game->ui.update();
            }
        }

        if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_LEFT)
            moveState |= Move::Move_Left;

        if (event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_RIGHT)
            moveState |= Move::Move_Right;

        if (event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDLK_UP)
            moveState |= Move::Move_Forward;

        if (event.key.keysym.sym == SDLK_s || event.key.keysym.sym == SDLK_DOWN)
            moveState |= Move::Move_Backward;

        if (event.key.keysym.sym == SDLK_SPACE)
            moveState |= Move::Move_Jump;

        if (event.key.keysym.sym == SDLK_LSHIFT)
            moveState |= Move::Move_Sprint;
    }
    else if (event.type == SDL_KEYUP)
    {
        if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_LEFT)
            moveState &= ~Move::Move_Left;

        if (event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_RIGHT)
            moveState &= ~Move::Move_Right;

        if (event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDLK_UP)
            moveState &= ~Move::Move_Forward;

        if (event.key.keysym.sym == SDLK_s || event.key.keysym.sym == SDLK_DOWN)
            moveState &= ~Move::Move_Backward;

        if (event.key.keysym.sym == SDLK_SPACE)
            moveState &= ~Move::Move_Jump;

        if (event.key.keysym.sym == SDLK_LSHIFT)
            moveState &= ~Move::Move_Sprint;

        if (event.key.keysym.sym == SDLK_F3)
        {
            auto crc32 = [](unsigned char* data, size_t length)
            {
                unsigned int crc;
                crc = 0xFFFFFFFFu; 

                for (int i = 0; i < length; i++)
                {
                    crc ^= (data[i] << 24u);

                    for (int j = 0; j < 8; j++)
                    {
                        unsigned int msb = crc >> 31u;
                        crc <<= 1u;
                        crc ^= (0u - msb) & 0x04C11DB7u;
                    }
                }

                return crc;
            };

            auto hash = crc32(game->level.blocks.get(), game->level.width * game->level.height * game->level.depth);
            game->ui.log("CRC32 checksum: " + std::to_string(hash));
        }     
    }
    else if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        if (event.button.button == SDL_BUTTON_LEFT)
            interactState |= Interact::Interact_Left;

        if (event.button.button == SDL_BUTTON_MIDDLE)
            interactState |= Interact::Interact_Middle;

        if (event.button.button == SDL_BUTTON_RIGHT)
            interactState |= Interact::Interact_Right;
    }
    else if (event.type == SDL_MOUSEBUTTONUP)
    {
        if (event.button.button == SDL_BUTTON_LEFT)
            interactState &= ~Interact::Interact_Left;

        if (event.button.button == SDL_BUTTON_MIDDLE)
            interactState &= ~Interact::Interact_Middle;

        if (event.button.button == SDL_BUTTON_RIGHT)
            interactState &= ~Interact::Interact_Right;  
    }
    else if (game->ui.state == UI::State::None)
    {
        if (event.type == SDL_MOUSEWHEEL)
        {
            if (event.wheel.y < 0)
            {
                inventoryIndex = (inventoryIndex + 1) % inventorySize;

                game->heldBlock.update();
                game->ui.update();
            }
            else if (event.wheel.y > 0)
            {
                inventoryIndex--;
                if (inventoryIndex < 0)
                {
                    inventoryIndex = inventorySize - 1;
                }

                game->heldBlock.update();
                game->ui.update();
            }
        }
        else if (event.type == SDL_MOUSEMOTION)
        {
            turn(
                (float)event.motion.xrel,
                (float)event.motion.yrel
            );
        }
    }
}

void LocalPlayer::setPosition(float x, float y, float z)
{
    Entity::setPosition(x, y, z);
    Entity::move(0.0f, -heightOffset, 0.0f);
}