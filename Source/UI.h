#pragma once
#include "VertexList.h"

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

class Game;

class UI
{
public:
	enum class State 
	{
		None,
		SelectBlockMenu,
		StatusMenu,
	};

	enum class MouseState 
	{
		Up,
		Down
	};

	void init(Game* game);
	bool input(const SDL_Event& event);
	void update();
	void render();

	void openMenu(UI::State state);
	void closeMenu();

	UI::State state;

	UI::MouseState mouseState;
	glm::vec2 mousePosition;
private:
	struct Button 
	{
		UI::MouseState mouseState;
	};

	void drawHUD();
	void drawHotbar();
	void drawBlock(unsigned char blockType, float x, float y, float scale);

	bool drawSelectBlockMenu();
	bool drawSelectBlockButton(unsigned char blockType, unsigned char& selectedBlockType, float x, float y, float width, float height);
	bool drawButton(float x, float y, const char* text);

	void drawInterface(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float shade, float z);
	void drawInterface(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float shade);
	void drawInterface(float x0, float y0, float x1, float y1, float u, float v, float shade);
	void drawInterface(float x0, float y0, float x1, float y1, float u, float v);

	void drawFont(const char* text, float x, float y, float shade);
	void drawShadowedFont(const char* text, float x, float y, float shade);
	void drawCenteredFont(const char* text, float x, float y, float shade);

	const float fontWidths[128] = {
		1, 8, 8, 8, 8, 8, 8, 1, 8, 1, 8, 8, 1, 8, 8, 8,
		8, 8, 1, 1, 8, 8, 1, 8, 1, 1, 8, 8, 8, 8, 8, 8,
		4, 2, 5, 6, 6, 7, 7, 3, 5, 5, 8, 6, 2, 6, 2, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 2, 2, 5, 6, 5, 6,
		7, 6, 6, 6, 6, 6, 6, 6, 6, 4, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 4, 6, 4, 6, 6,
		3, 6, 6, 6, 6, 6, 5, 6, 6, 2, 6, 5, 3, 6, 6, 6,
		6, 6, 6, 6, 4, 6, 6, 6, 6, 6, 6, 5, 2, 5, 7, 6,
	};

	VertexList fontVertices;
	GLuint fontTexture;

	VertexList interfaceVertices;
	GLuint interfaceTexture;

	VertexList blockVertices;
	
	Game* game;
};

