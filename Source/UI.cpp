#include "UI.h"
#include "Game.h"
#include "Block.h"
#include "Resources.h"

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
	this->fontTexture = game->textureManager.load(fontResourceTexture, sizeof(fontResourceTexture));

	this->interfaceVertices.init();
	this->interfaceTexture = game->textureManager.load(interfaceResourceTexture, sizeof(interfaceResourceTexture));
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

void UI::openMainMenu()
{
	mainMenuCopied = false;

	openMenu(State::MainMenu);
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
		if (event.key.keysym.sym == SDLK_ESCAPE)
		{
			if (state == State::None)
			{
				openMainMenu();
				return false;
			}
			else if (state == State::SaveMenu || state == State::LoadMenu)
			{
				openMainMenu();
				return false;
			}
			else if (state == State::MainMenu)
			{
				closeMenu();
				return false;
			}
		}

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

		if (state == State::MainMenu)
		{
			if (drawMainMenu())
			{
				return;
			}
		}
	}

	fontVertices.update();
	interfaceVertices.update();
	blockVertices.update();
}

void UI::render()
{
	blockVertices.render();

	glBindTexture(GL_TEXTURE_2D, fontTexture);
	fontVertices.render();

	glBindTexture(GL_TEXTURE_2D, interfaceTexture);
	interfaceVertices.render();
}

void UI::log(const std::string& text)
{
	Log log;
	log.created = game->timer.milliTime();
	log.text = text;

	logs.push_back(log);

	update();
}

void UI::logMotd()
{
	game->ui.log("Connected! Invite friends by sharing the link.");
}

bool UI::drawStatusMenu()
{
	int x = int(glm::ceil(game->scaledWidth / 16));
	int y = int(glm::ceil(game->scaledHeight / 16));

	for (int i = 0; i < x; i++)
	{
		for (int j = 0; j < y; j++)
		{
			drawInterface(i * 16.0f, j * 16.0f, 240.0f, 0.0f, 16.0f, 16.0f, 0.25f, 0.9f);
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

bool UI::drawMainMenu() 
{
	const float offset = 73.0f;
	const float optionsOffset = 72.0f;

	drawInterface(0.0f, 0.0f, game->scaledWidth, game->scaledHeight, 183, 0, 16, 16, 0.05f, 64.0f);
	drawCenteredFont("Main Menu", game->scaledWidth / 2, game->scaledHeight / 2 - offset, 1.0f, 65.0f);

	if (drawButton(game->scaledWidth / 2 - 100, game->scaledHeight / 2 - offset + 16, 65.0f, "Back to Game"))
	{
		closeMenu();
		return true;
	}

	drawButton(game->scaledWidth / 2 - 100, game->scaledHeight / 2 - offset + 40, 65.0f, "Load", game->network.isHost() || !game->network.isConnected(), 98.0f);
	drawButton(game->scaledWidth / 2, game->scaledHeight / 2 - offset + 40, 65.0f, "Save", game->network.isHost() || !game->network.isConnected(), 100.0f);

	drawCenteredFont("Invite your friends by sharing the link", game->scaledWidth / 2, game->scaledHeight / 2 - offset + optionsOffset, 1.0f, 65.0f);
	drawButton(game->scaledWidth / 2 - 100, game->scaledHeight / 2 - offset + optionsOffset + 16, 65.0f, game->network.url.c_str(), 0);

	if (drawButton(game->scaledWidth / 2 - 100, game->scaledHeight / 2 - offset + optionsOffset + 24 + 16, 65.0f, mainMenuCopied ? "Copied!" : "Copy"))
	{
		SDL_SetClipboardText(game->network.url.c_str());
		mainMenuCopied = true;

		return true;
	}

	return false;
}

bool UI::drawSelectBlockMenu() 
{ 
	float left = game->scaledWidth / 2.0f - 196.0f / 2.0f;
	float top = game->scaledHeight / 2.0f - 143.0f / 2.0f;

	drawInterface(left - 4, top - 3, 0, 106, 204, 149.9f, 1.0f, 2.0f);

	for (unsigned char blockType = 0, selectedBlockType = 0, index = 0; blockType < std::size(Block::Definitions); blockType++)
	{
		if (!game->level.isAirTile(blockType) && !game->level.isWaterTile(blockType) && !game->level.isLavaTile(blockType))
		{
			float x = 10.0f + left + 23.0f * (index % 8);
			float y = 23.5f + top + 21.0f * (index / 8);
			 
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
		drawInterface(hoverX, hoverY, width, height, 183, 0, 16, 16, 0.7f, 2.0f);
		drawBlock(blockType, x - 1.2f, y + 1.0f, 12.0f);

		selectedBlockType = blockType;
	}
	else
	{
		drawBlock(blockType, x, y, 10.0f);
	}

	return hover && clicked;
}

bool UI::drawButton(float x, float y, float z, const char* text, int state, float width)
{
	float height = 20;

	bool clicked = mouseState == MouseState::Down;

	float hoverX = x;
	float hoverY = y;

	bool hover = mousePosition.x >= hoverX &&
		mousePosition.x <= hoverX + width &&
		mousePosition.y >= hoverY &&
		mousePosition.y <= hoverY + height;

	if (state && hover) { state = 2; }

	drawInterface(x, y, 0.0f, 46.0f + state * 19.99f, width / 2, height, 1.0f, z);
	drawInterface(x + width / 2, y, 200 - width / 2, 46.0f + state * 19.99f, width / 2, height, 1.0f, z);

	if (state)
	{
		drawCenteredFont(text, x + width / 2, y + (height - 8) / 2, 1.0f, z + 100.0f);
	}
	else
	{
		float size = 0.0f;
		int index = 0;

		const auto length = std::strlen(text);
		for (auto i = 0; i < length; i++)
		{
			size += FONT_WIDTHS[int(text[i])];
			index = i;

			if (size > width - 15.0f)
			{
				break;
			}
		}

		auto truncatedText = std::string(text);
		if (index < length - 1)
		{
			truncatedText = truncatedText.substr(0, index) + "...";
		}

		drawCenteredFont(truncatedText.c_str(), x + width / 2, y + (height - 8) / 2, 0.7f, z + 100.0f);
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

	drawInterface(x, y, 0.0f, 46.0f + state * 19.99f, width / 2, height, 1.0f, 0.9f);
	drawInterface(x + width / 2, y, 200 - width / 2, 46.0f + state * 19.99f, width / 2, height, 1.0f, 0.9f);
	drawCenteredFont(text, x + width / 2, y + (height - 8) / 2, 1.0f);

	return hover && clicked;
}

void UI::drawHUD()
{
	static char info[255];
	std::snprintf(info, sizeof(info), "%lld fps, %lld chunk updates", game->lastFrameRate, game->lastChunkUpdates);

	drawShadowedFont(info, 3.0f, 3.0f, 1.0f);

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
			float maxWidth = game->scaledWidth / 2 + 18.0f;
			float width = 0.0f;

			for (auto i = 0; i < log->text.length(); i++)
			{
				width += FONT_WIDTHS[int(log->text[i])];
			}

			if (width > maxWidth) 
			{
				maxWidth = width + 4.0f;
			}

			drawInterface(0.0f, game->scaledHeight - 35.0f - index * 10.0f, maxWidth, 10.0f, 183, 0, 16, 16, 0.12f);
			drawFont(log->text.c_str(), 1.8f, game->scaledHeight - 33.8f - index * 10.0f, 1.0f, 1.1f);

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

		auto matrix = game->IDENTITY_MATRIX;
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

void UI::drawInterface(float x0, float y0, float x1, float y1, float u, float v, float shade, float z)
{
	float size = 0.00390625;

	interfaceVertices.push(VertexList::Vertex(x0, y0, z, x1 * size, y1 * size, shade));
	interfaceVertices.push(VertexList::Vertex(x0, y0 + v, z, x1 * size, (y1 + v) * size, shade));
	interfaceVertices.push(VertexList::Vertex(x0 + u, y0 + v, z, (x1 + u) * size, (y1 + v) * size, shade));

	interfaceVertices.push(VertexList::Vertex(x0, y0, z, x1 * size, y1 * size, shade));
	interfaceVertices.push(VertexList::Vertex(x0 + u, y0 + v, z, (x1 + u) * size, (y1 + v) * size, shade));
	interfaceVertices.push(VertexList::Vertex(x0 + u, y0, z, (x1 + u) * size, y1 * size, shade));
}

void UI::drawInterface(float x0, float y0, float x1, float y1, float u, float v, float shade)
{
	drawInterface(x0, y0, x1, y1, u, v, shade, 1.0f);
}

void UI::drawInterface(float x0, float y0, float x1, float y1, float u, float v)
{
	drawInterface(x0, y0, x1, y1, u, v, 1.0f);
}

void UI::drawShadowedFont(const char* text, float x, float y, float shade, float z)
{
	drawFont(text, x + 1.0f, y + 1.0f, 0.3f * shade, z);
	drawFont(text, x, y, shade, z);
}

void UI::drawShadowedFont(const char* text, float x, float y, float shade)
{
	drawShadowedFont(text, x, y, shade, 1.0f);
}

void UI::drawCenteredFont(const char* text, float x, float y, float shade, float z)
{
	float width = 0.0f;

	const auto length = std::strlen(text);
	for (auto i = 0; i < length; i++)
	{
		width += FONT_WIDTHS[int(text[i])];
	}

	drawShadowedFont(text, x - width / 2, y, shade, z);
}

void UI::drawCenteredFont(const char* text, float x, float y, float shade)
{
	drawCenteredFont(text, x, y, shade, 1.0f);
}

void UI::drawFont(const char* text, float x, float y, float shade, float z)
{
	float width = 0.0f;

	const auto length = std::strlen(text);
	for (int index = 0; index < length; index++)
	{
		float u = float(text[index] % 16 << 3);
		float v = float(text[index] / 16 << 3);

		float height = 7.98f;

		fontVertices.push(VertexList::Vertex(x + width, y, z, u / 128.0f, v / 128.0f, shade));
		fontVertices.push(VertexList::Vertex(x + width, y + height, z, u / 128.0f, (v + height) / 128.0f, shade));
		fontVertices.push(VertexList::Vertex(x + width + height, y + height, z, (u + height) / 128.0f, (v + height) / 128.0f, shade));

		fontVertices.push(VertexList::Vertex(x + width, y, z, u / 128.0f, v / 128.0f, shade));
		fontVertices.push(VertexList::Vertex(x + width + height, y + height, z, (u + height) / 128.0f, (v + height) / 128.0f, shade));
		fontVertices.push(VertexList::Vertex(x + width + height, y, z, (u + height) / 128.0f, v / 128.0f, shade));

		width += FONT_WIDTHS[int(text[index])];
	}
}
