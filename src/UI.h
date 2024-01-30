#pragma once
#include "VertexList.h"

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class UI
{
public:
	enum class State
	{
		None,
		StatusMenu,
		SelectBlockMenu,
		MainMenu,
		SaveMenu,
		LoadMenu,
	};

	void init();
	bool input(const SDL_Event& event);
	void update();
	void render();

	void openMenu(UI::State state, bool shouldUpdate = true);
	void openStatusMenu(const char* title, const char* description, bool closeable = false);
	void openMainMenu();
	void closeMenu();

	void log(const char* format, ...);

	UI::State state;
	bool isTouch;

private:
	enum class MouseState
	{
		Up,
		Down
	};

	enum class TouchState
	{
		None = 0,
		Up = 1 << 0,
		Down = 1 << 1,
		Left = 1 << 2,
		Right = 1 << 3,
		Middle = 1 << 4,
		Jump = 1 << 5,
		Fullscreen = 1 << 6,
		Menu = 1 << 7,
	};

	enum class Cancellable
	{
		None = 0,
		Hold = 1 << 0,
		Swipe = 1 << 1,
	};

	struct Log
	{
		uint64_t created;
		std::string text;
	};

	struct Save
	{
		std::string path;
		std::string name;
	};

	struct TouchPosition 
	{
		int64_t id;
		float x;
		float y;

		uint64_t startTime;

		bool swipe;
		bool hold;
		bool isHolding;
	};

	void think();
	void refresh();
	void load(size_t index);
	void save(size_t index);

	void drawHUD();
	void drawFPS();
	void drawCrosshair();
	void drawLogs();
	void drawHotbar();
	bool drawTouchControls(bool invisible = false);
	
	bool drawStatusMenu();
	bool drawLoadMenu();
	bool drawSaveMenu();
	bool drawMainMenu();
	bool drawSelectBlockMenu();

	bool drawSelectBlockButton(unsigned char blockType, unsigned char& selectedBlockType, float x, float y, float width, float height);
	bool drawTouchButton(unsigned int flag, float x, float y, float z, const char* text, float width = 200.0f, float height = 20.0f, bool multiTouch = true, bool invisible = false);
	bool drawButton(float x, float y, float z, const char* text, int state = 1, float width = 200.0f, float height = 20.0f);

	void drawBlock(unsigned char blockType, float x, float y, float scale);

	void drawInterface(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float shade, float z);
	void drawInterface(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float shade);
	void drawInterface(float x0, float y0, float x1, float y1, float u, float v, float shade, float z);
	void drawInterface(float x0, float y0, float x1, float y1, float u, float v, float shade);
	void drawInterface(float x0, float y0, float x1, float y1, float u, float v);

	void drawFont(const char* text, float x, float y, float shade, float z);
	void drawShadowedFont(const char* text, float x, float y, float shade, float z);
	void drawShadowedFont(const char* text, float x, float y, float shade);
	void drawCenteredFont(const char* text, float x, float y, float shade, float z);
	void drawCenteredFont(const char* text, float x, float y, float shade);

	const float FONT_WIDTHS[256] = {
		1, 8, 8, 8, 8, 8, 8, 9, 9, 1, 8, 8, 1, 8, 8, 8,
		8, 8, 1, 1, 8, 8, 8, 8, 1, 1, 8, 8, 8, 8, 8, 8,
		4, 2, 5, 6, 6, 7, 7, 3, 5, 5, 8, 6, 2, 6, 2, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 2, 2, 5, 6, 5, 6,
		7, 6, 6, 6, 6, 6, 6, 6, 6, 4, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 4, 6, 4, 6, 6,
		3, 6, 6, 6, 6, 6, 5, 6, 6, 2, 6, 5, 3, 6, 6, 6,
		6, 6, 6, 6, 4, 6, 6, 6, 6, 6, 6, 5, 2, 5, 7, 6,

		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		6, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	};

	glm::vec2 mousePosition;
	UI::MouseState mouseState;
	unsigned int touchState;

	std::vector<glm::vec2> buttonPositions;
	std::vector<TouchPosition> touchPositions;
	std::vector<Log> logs;

	size_t page;
	std::vector<Save> saves;

	std::string statusTitle;
	std::string statusDescription;
	bool statusCloseable;

	uint64_t mainMenuLastCopy;

	VertexList fontVertices;
	GLuint fontTexture;

	VertexList interfaceVertices;
	GLuint interfaceTexture;

	VertexList blockVertices;
};

