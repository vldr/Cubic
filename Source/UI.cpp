#include "UI.h"
#include "Game.h"
#include "Block.h"

#include <cstdio>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef EMSCRIPTEN
#include <emscripten/html5.h>
#endif

void UI::init(Game* game)
{
	this->game = game;
	this->state = State::None;
	this->mousePosition = glm::vec2();

	this->blockVertices.init();

	this->fontVertices.init();
	this->fontTexture = game->textureManager.load("Assets/font.png");

	this->interfaceVertices.init();
	this->interfaceTexture = game->textureManager.load("Assets/interface.png");
}

void UI::openMenu(UI::State newState)
{
	if (newState != State::None)
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);

#ifdef EMSCRIPTEN
		emscripten_exit_pointerlock();
#endif

		mouseState = MouseState::Up;
		state = newState;

		update();
	}
}

void UI::closeMenu()
{
	SDL_SetRelativeMouseMode(SDL_TRUE);

	mouseState = MouseState::Up;
	state = State::None;

	update();
}

bool UI::input(const SDL_Event& event)
{
	if (event.type == SDL_KEYUP)
	{
		if (event.key.keysym.sym == SDLK_b)
		{
			if (state != State::None)
			{
				closeMenu();
				return false;
			}
			else
			{
				openMenu(State::SelectBlockMenu);
				return false;
			}
		}
	}
	else if (event.type == SDL_MOUSEMOTION)
	{
		if (state != State::None)
		{
			mousePosition.x = float(event.motion.x) / float(game->scaleFactor);
			mousePosition.y = float(event.motion.y) / float(game->scaleFactor);

			update();
			return false;
		}
	}
	else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
	{
		if (state != State::None)
		{
			mouseState = MouseState::Down;

			update();
			return false;
		}
	}
	else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
	{
		if (state != State::None)
		{
			mouseState = MouseState::Up;

			update();
			return false;
		}
	}

	return true;
}

void UI::update()
{
	fontVertices.reset();
	interfaceVertices.reset();
	blockVertices.reset();

	drawHUD();

	if (state == State::SelectBlockMenu && drawSelectBlockMenu())
	{
		return;
	}

	fontVertices.update();
	interfaceVertices.update();
	blockVertices.update();
}

void UI::render()
{
	glBindTexture(GL_TEXTURE_2D, interfaceTexture);
	interfaceVertices.render();

	glBindTexture(GL_TEXTURE_2D, fontTexture);
	fontVertices.render();

	glBindTexture(GL_TEXTURE_2D, game->atlasTexture);
	blockVertices.render();
}

bool UI::drawSelectBlockMenu()
{
	float left = game->scaledWidth / 2.0f - 196.5f / 2.0f;
	float top = game->scaledHeight / 2 - 143.0f / 2.0f;

	drawInterface(left, top, 208, 143, 183, 0, 16, 16, 0.12f);

	for (unsigned char blockType = 0, selectedBlockType = 0, index = 0; blockType < std::size(Block::Definitions); blockType++)
	{
		if (!game->level.isAirTile(blockType) && !game->level.isWaterTile(blockType) && !game->level.isLavaTile(blockType))
		{
			float x = 10.0f + left + 23.0f * (index % 8);
			float y = 24.0f + top + 21.0f * (index / 8);

			float width = 23.5f;
			float height = 23.0f;

			if (drawSelectBlockButton(blockType, selectedBlockType, x, y, width, height))
			{
				game->localPlayer.inventory[game->localPlayer.inventoryIndex] = blockType;
				game->heldBlock.update();

				closeMenu();
				return true;
			}

			index++;
		}
	}

	return false;
}

bool UI::drawSelectBlockButton(unsigned char blockType, unsigned char& selectedBlockType, float x, float y, float width, float height)
{
	bool clicked = mouseState == MouseState::Down;

	float hoverX = x - 4.5f;
	float hoverY = y - 16.0f;

	bool hover = mousePosition.x >= hoverX && 
				 mousePosition.x <= hoverX + width && 
				 mousePosition.y >= hoverY && 
			     mousePosition.y <= hoverY + height;

	if (selectedBlockType == 0 && hover)
	{	
		drawInterface(hoverX, hoverY, width, height, 183, 0, 16, 16, 0.7f);
		drawBlock(blockType, x - 1.8f, y + 1.0f, 12.0f);

		selectedBlockType = blockType;
	}
	else
	{
		drawBlock(blockType, x, y, 10.0f);
	}

	return hover && clicked;
}

void UI::drawHUD()
{
	static char fps[100];
	std::snprintf(fps, sizeof(fps), "%lld fps, %lld chunk updates", game->lastFrameRate, game->lastChunkUpdates);
	drawFont(fps, 3.0f, 3.0f);

	drawInterface(game->scaledWidth / 2 - 91, game->scaledHeight - 22, 0, 0, 182, 22);
	drawInterface(game->scaledWidth / 2 - 92 + float(game->localPlayer.inventoryIndex) * 20, game->scaledHeight - 23, 0, 22, 24, 22);
	drawInterface(game->scaledWidth / 2 - 7, game->scaledHeight / 2 - 7, 24, 22, 16, 16);

	drawHotbar();
}

void UI::drawHotbar()
{
	for (int i = 0; i < game->localPlayer.inventorySize; i++)
	{
		auto blockType = game->localPlayer.inventory[i];

		float x = game->scaledWidth / 2.0f - 86.8f + i * 20.0f;
		float y = game->scaledHeight - 6.8f;

		drawBlock(blockType, x, y, 9.8f);
	}
}

void UI::drawBlock(unsigned char blockType, float x, float y, float scale)
{
	auto blockDefinition = Block::Definitions[blockType];

	float uTop = 0.0625f * (blockDefinition.topTexture % 16);
	float vTop = 0.0625f * (blockDefinition.topTexture / 16);
	float uTop2 = 0.0625f + 0.0625f * (blockDefinition.topTexture % 16);
	float vTop2 = 0.0625f + 0.0625f * (blockDefinition.topTexture / 16);

	float u = 0.0625f * (blockDefinition.sideTexture % 16);
	float v = 0.0625f * (blockDefinition.sideTexture / 16) + (0.0625f - (0.0625f * blockDefinition.height));
	float u2 = 0.0625f + 0.0625f * (blockDefinition.sideTexture % 16);
	float v2 = 0.0625f + 0.0625f * (blockDefinition.sideTexture / 16);

	VertexList::Vertex spriteVertexList[] = {
		VertexList::Vertex(0.0f, 1.0f, 1.0f, u, v, 1.0f),
		VertexList::Vertex(0.0f, 0.0f, 1.0f, u, v2, 1.0f),
		VertexList::Vertex(1.0f, 0.0f, 1.0f, u2, v2, 1.0f),

		VertexList::Vertex(0.0f, 1.0f, 1.0f, u, v, 1.0f),
		VertexList::Vertex(1.0f, 0.0f, 1.0f, u2, v2, 1.0f),
		VertexList::Vertex(1.0f, 1.0f, 1.0f, u2, v, 1.0f),
	};

	VertexList::Vertex blockVertexList[] = {
		VertexList::Vertex(0.0f, blockDefinition.height, 0.0f, uTop, vTop, 1.0f),
        VertexList::Vertex(0.0f, blockDefinition.height, 1.0f, uTop, vTop2, 1.0f),
        VertexList::Vertex(1.0f, blockDefinition.height, 1.0f, uTop2, vTop2, 1.0f),

        VertexList::Vertex(0.0f, blockDefinition.height, 0.0f, uTop, vTop, 1.0f),
        VertexList::Vertex(1.0f, blockDefinition.height, 1.0f, uTop2, vTop2, 1.0f),
        VertexList::Vertex(1.0f, blockDefinition.height, 0.0f, uTop2, vTop, 1.0f),

        VertexList::Vertex(0.0f, blockDefinition.height, 1.0f, u, v, 0.8f),
        VertexList::Vertex(0.0f, 0.0f, 1.0f, u, v2, 0.8f),
        VertexList::Vertex(1.0f, 0.0f, 1.0f, u2, v2, 0.8f),

        VertexList::Vertex(0.0f, blockDefinition.height, 1.0f, u, v, 0.8f),
        VertexList::Vertex(1.0f, 0.0f, 1.0f, u2, v2, 0.8f),
        VertexList::Vertex(1.0f, blockDefinition.height, 1.0f, u2, v, 0.8f),

        VertexList::Vertex(0.0f, blockDefinition.height, 0.0f, u, v, 0.6f),
        VertexList::Vertex(0.0f, 0.0f, 0.0f, u, v2, 0.6f),
        VertexList::Vertex(0.0f, 0.0f, 1.0f, u2, v2, 0.6f),

        VertexList::Vertex(0.0f, blockDefinition.height, 0.0f, u, v, 0.6f),
        VertexList::Vertex(0.0f, 0.0f, 1.0f, u2, v2, 0.6f),
        VertexList::Vertex(0.0f, blockDefinition.height, 1.0f, u2, v, 0.6f),
	};

	size_t length;
	VertexList::Vertex* vertices;

	if (blockDefinition.draw == Block::DrawType::DRAW_SPRITE)
	{
		vertices = spriteVertexList;
		length = std::size(spriteVertexList);
	}
	else
	{
		vertices = blockVertexList;
		length = std::size(blockVertexList);
	}

	for (int i = 0; i < length; i++)
	{
		VertexList::Vertex* vertex = &vertices[i];

		auto matrix = game->identityMatrix;
		matrix = glm::translate(matrix, glm::vec3(x, y, -15.0f));

		if (blockDefinition.draw == Block::DrawType::DRAW_SPRITE)
		{
			matrix = glm::translate(matrix, glm::vec3(2.0f, 1.0f, 0.0f));
		}
		else
		{
			matrix = glm::rotate(matrix, glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			matrix = glm::rotate(matrix, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		}

		matrix = glm::scale(matrix, glm::vec3(scale, -scale, scale));

		glm::vec4 position = matrix * glm::vec4(vertex->x, vertex->y, vertex->z, 1.0f);
		vertex->x = position.x;
		vertex->y = position.y;
		vertex->z = position.z;

		blockVertices.push(*vertex);
	}
}

void UI::drawInterface(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float shade)
{
	float size = 0.00390625;

	interfaceVertices.push(VertexList::Vertex(x0, y0, 1.0f, u0 * size, v0 * size, shade));
	interfaceVertices.push(VertexList::Vertex(x0, y0 + y1, 1.0f, u0 * size, (v0 + v1) * size, shade));
	interfaceVertices.push(VertexList::Vertex(x0 + x1, y0 + y1, 1.0f, (u0 + u1) * size, (v0 + v1) * size, shade));

	interfaceVertices.push(VertexList::Vertex(x0, y0, 1.0f, u0 * size, v0 * size, shade));
	interfaceVertices.push(VertexList::Vertex(x0 + x1, y0 + y1, 1.0f, (u0 + u1) * size, (v0 + v1) * size, shade));
	interfaceVertices.push(VertexList::Vertex(x0 + x1, y0, 1.0f, (u0 + u1) * size, v0 * size, shade));
}

void UI::drawInterface(float x0, float y0, float x1, float y1, float u, float v)
{
	float size = 0.00390625;

	interfaceVertices.push(VertexList::Vertex(x0, y0, 1.0f, x1 * size, y1 * size, 1.0f));
	interfaceVertices.push(VertexList::Vertex(x0, y0 + v, 1.0f, x1 * size, (y1 + v) * size, 1.0f));
	interfaceVertices.push(VertexList::Vertex(x0 + u, y0 + v, 1.0f, (x1 + u) * size, (y1 + v) * size, 1.0f));

	interfaceVertices.push(VertexList::Vertex(x0, y0, 1.0f, x1 * size, y1 * size, 1.0f));
	interfaceVertices.push(VertexList::Vertex(x0 + u, y0 + v, 1.0f, (x1 + u) * size, (y1 + v) * size, 1.0f));
	interfaceVertices.push(VertexList::Vertex(x0 + u, y0, 1.0f, (x1 + u) * size, y1 * size, 1.0f));
}

void UI::drawFont(const char* text, float x, float y)
{
	drawFont(text, x + 1.0f, y + 1.0f, 0.3f);
	drawFont(text, x, y, 1.0f);
}

void UI::drawCenteredFont(const char* text, float x, float y)
{
	float width = 0.0f;

	const auto length = std::strlen(text);
	for (auto i = 0; i < length; i++)
	{
		width += fontWidths[int(text[i])];
	}

	drawFont(text, x - width / 2, y);
}

void UI::drawFont(const char* text, float x, float y, float shade)
{
	float width = 0.0f;

	const auto length = std::strlen(text);
	for (int index = 0; index < length; index++)
	{
		int u = text[index] % 16 << 3;
		int v = text[index] / 16 << 3;

		float height = 7.99f;

		fontVertices.push(VertexList::Vertex(x + width, y, 1.0f, u / 128.0f, v / 128.0f, shade));
		fontVertices.push(VertexList::Vertex(x + width, y + height, 1.0f, u / 128.0f, (v + height) / 128.0f, shade));
		fontVertices.push(VertexList::Vertex(x + width + height, y + height, 1.0f, (u + height) / 128.0f, (v + height) / 128.0f, shade));

		fontVertices.push(VertexList::Vertex(x + width, y, 1.0f, u / 128.0f, v / 128.0f, shade));
		fontVertices.push(VertexList::Vertex(x + width + height, y + height, 1.0f, (u + height) / 128.0f, (v + height) / 128.0f, shade));
		fontVertices.push(VertexList::Vertex(x + width + height, y, 1.0f, (u + height) / 128.0f, v / 128.0f, shade));

		width += fontWidths[int(text[index])];
	}
}
