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

void UI::openStatusMenu(const char* title, const char* description, bool closeable)
{
	statusTitle = title;
	statusDescription = description;
	statusCloseable = closeable;

	openMenu(State::StatusMenu);
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
		if (event.key.keysym.sym == SDLK_b || event.key.keysym.sym == SDLK_e)
		{
			if (state == State::SelectBlockMenu)
			{
				closeMenu();
				return false;
			}
			else if (state == State::None)
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
	
	if (state == State::StatusMenu)
	{
		if (drawStatusMenu())
		{
			return;
		}
	}
	else
	{
		drawHUD();

		if (state == State::SelectBlockMenu && drawSelectBlockMenu())
		{
			return;
		}
	}

	fontVertices.update();
	interfaceVertices.update();
	blockVertices.update();
}

void UI::render()
{
	blockVertices.render();

	glBindTexture(GL_TEXTURE_2D, interfaceTexture);
	interfaceVertices.render();

	glBindTexture(GL_TEXTURE_2D, fontTexture);
	fontVertices.render();
}

void UI::log(const std::string& text)
{
	Log log;
	log.created = game->timer.milliTime();
	log.text = text;

	logs.push_back(log);
}

bool UI::drawStatusMenu()
{
	int x = int(glm::ceil(game->scaledWidth / 16));
	int y = int(glm::ceil(game->scaledHeight / 16));

	for (int i = 0; i < x; i++)
	{
		for (int j = 0; j < y; j++)
		{
			drawInterface(i * 16.0f, j * 16.0f, 240.0f, 0.0f, 16.0f, 16.0f, 0.25f);
		}
	}

	if (statusCloseable)
	{
		drawCenteredFont(statusTitle.c_str(), game->scaledWidth / 2, game->scaledHeight / 2 - 25.0f, 0.6f);
		drawCenteredFont(statusDescription.c_str(), game->scaledWidth / 2, game->scaledHeight / 2 - 12.0f, 1.0f);

		if (drawButton(game->scaledWidth / 2 - 100, game->scaledHeight / 2 + 5.0f, game->network.isConnected() ? "Create a new room" : "Play offline"))
		{
			game->network.create();

			closeMenu();
			return true;
		}
	}
	else
	{
		drawCenteredFont(statusTitle.c_str(), game->scaledWidth / 2, game->scaledHeight / 2 - 13.0f, 0.6f);
		drawCenteredFont(statusDescription.c_str(), game->scaledWidth / 2, game->scaledHeight / 2, 1.0f);
	}

	return false;
}

bool UI::drawSelectBlockMenu()
{
	float left = game->scaledWidth / 2.0f - 196.0f / 2.0f;
	float top = game->scaledHeight / 2.0f - 143.0f / 2.0f;

	drawInterface(left, top, 196, 143, 183, 0, 16, 16, 0.12f);

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
		drawBlock(blockType, x - 1.2f, y + 1.0f, 12.0f);

		selectedBlockType = blockType;
	}
	else
	{
		drawBlock(blockType, x, y, 10.0f);
	}

	return hover && clicked;
}

bool UI::drawButton(float x, float y, const char* text)
{
	float width = 200; 
	float height = 20;

	bool clicked = mouseState == MouseState::Down;

	float hoverX = x;
	float hoverY = y;

	bool hover = mousePosition.x >= hoverX &&
		mousePosition.x <= hoverX + width &&
		mousePosition.y >= hoverY &&
		mousePosition.y <= hoverY + height;

	int state = 1;
	if (hover) { state = 2; }

	drawInterface(x, y, 0.0f, 46.0f + state * 19.99f, width / 2, height);
	drawInterface(x + width / 2, y, 200 - width / 2, 46.0f + state * 19.99f, width / 2, height);
	drawCenteredFont(text, x + width / 2, y + (height - 8) / 2, 1.0f);

	return hover && clicked;
}

void UI::drawHUD()
{
	static char fps[100];
	std::snprintf(fps, sizeof(fps), "%lld fps, %lld chunk updates", game->lastFrameRate, game->lastChunkUpdates);
	drawShadowedFont(fps, 3.0f, 3.0f, 1.0f);

	drawInterface(game->scaledWidth / 2 - 91, game->scaledHeight - 22, 0, 0, 182, 22);
	drawInterface(game->scaledWidth / 2 - 92 + float(game->localPlayer.inventoryIndex) * 20, game->scaledHeight - 23, 0, 22, 24, 22);
	drawInterface(game->scaledWidth / 2 - 7, game->scaledHeight / 2 - 7, 211, 0, 16, 16);

	drawHotbar();

	for (auto log = logs.begin(); log != logs.end();)
	{
		auto index = logs.end() - log - 1;

		if (game->timer.milliTime() - log->created > 5000)
		{
			log = logs.erase(log);
		}
		else
		{
			drawInterface(1.4f, game->scaledHeight - 35.0f - index * 10.0f, game->scaledWidth / 2 + 18.0f, 10.0f, 183, 0, 16, 16, 0.12f);
			drawInterface(0, game->scaledHeight - 35.0f - index * 10.0f, 1.5f, 10.0f, 183, 0, 16, 16, 1.0f);
			drawFont(log->text.c_str(), 3.30f, game->scaledHeight - 33.8f - index * 10.0f, 1.0f);

			log++;
		}
	}
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
		matrix = glm::translate(matrix, glm::vec3(x, y, 15.0f));

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

void UI::drawInterface(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float shade, float z)
{
	float size = 0.00390625f;

	interfaceVertices.push(VertexList::Vertex(x0, y0, z, u0 * size, v0 * size, shade));
	interfaceVertices.push(VertexList::Vertex(x0, y0 + y1, z, u0 * size, (v0 + v1) * size, shade));
	interfaceVertices.push(VertexList::Vertex(x0 + x1, y0 + y1, z, (u0 + u1) * size, (v0 + v1) * size, shade));

	interfaceVertices.push(VertexList::Vertex(x0, y0, z, u0 * size, v0 * size, shade));
	interfaceVertices.push(VertexList::Vertex(x0 + x1, y0 + y1, z, (u0 + u1) * size, (v0 + v1) * size, shade));
	interfaceVertices.push(VertexList::Vertex(x0 + x1, y0, z, (u0 + u1) * size, v0 * size, shade));
}

void UI::drawInterface(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float shade)
{
	drawInterface(x0, y0, x1, y1, u0, v0, u1, v1, shade, 1.0f);
}

void UI::drawInterface(float x0, float y0, float x1, float y1, float u, float v, float shade)
{
	float size = 0.00390625;

	interfaceVertices.push(VertexList::Vertex(x0, y0, 1.0f, x1 * size, y1 * size, shade));
	interfaceVertices.push(VertexList::Vertex(x0, y0 + v, 1.0f, x1 * size, (y1 + v) * size, shade));
	interfaceVertices.push(VertexList::Vertex(x0 + u, y0 + v, 1.0f, (x1 + u) * size, (y1 + v) * size, shade));

	interfaceVertices.push(VertexList::Vertex(x0, y0, 1.0f, x1 * size, y1 * size, shade));
	interfaceVertices.push(VertexList::Vertex(x0 + u, y0 + v, 1.0f, (x1 + u) * size, (y1 + v) * size, shade));
	interfaceVertices.push(VertexList::Vertex(x0 + u, y0, 1.0f, (x1 + u) * size, y1 * size, shade));
}

void UI::drawInterface(float x0, float y0, float x1, float y1, float u, float v)
{
	drawInterface(x0, y0, x1, y1, u, v, 1.0f);
}

void UI::drawShadowedFont(const char* text, float x, float y, float shade)
{
	drawFont(text, x + 1.0f, y + 1.0f, 0.3f * shade);
	drawFont(text, x, y, shade);
}

void UI::drawCenteredFont(const char* text, float x, float y, float shade)
{
	float width = 0.0f;

	const auto length = std::strlen(text);
	for (auto i = 0; i < length; i++)
	{
		width += fontWidths[int(text[i])];
	}

	drawShadowedFont(text, x - width / 2, y, shade);
}

void UI::drawFont(const char* text, float x, float y, float shade)
{
	float width = 0.0f;

	const auto length = std::strlen(text);
	for (int index = 0; index < length; index++)
	{
		float u = float(text[index] % 16 << 3);
		float v = float(text[index] / 16 << 3);

		float height = 7.98f;

		fontVertices.push(VertexList::Vertex(x + width, y, 1.0f, u / 128.0f, v / 128.0f, shade));
		fontVertices.push(VertexList::Vertex(x + width, y + height, 1.0f, u / 128.0f, (v + height) / 128.0f, shade));
		fontVertices.push(VertexList::Vertex(x + width + height, y + height, 1.0f, (u + height) / 128.0f, (v + height) / 128.0f, shade));

		fontVertices.push(VertexList::Vertex(x + width, y, 1.0f, u / 128.0f, v / 128.0f, shade));
		fontVertices.push(VertexList::Vertex(x + width + height, y + height, 1.0f, (u + height) / 128.0f, (v + height) / 128.0f, shade));
		fontVertices.push(VertexList::Vertex(x + width + height, y, 1.0f, (u + height) / 128.0f, v / 128.0f, shade));

		width += fontWidths[int(text[index])];
	}
}
