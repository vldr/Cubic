#pragma once
#include "LocalPlayer.h"
#include "LevelRenderer.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "LevelGenerator.h"
#include "Level.h"
#include "Random.h"
#include "Timer.h"
#include "ParticleManager.h"
#include "HeldBlock.h"
#include "SelectedBlock.h"
#include "UI.h"
#include "Frustum.h"
#include "Network.h"
#include "VertexList.h"

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

extern class Game {
public:
	void init(SDL_Window* window);
	void input(const SDL_Event& event);
	void render();
	void resize();

	SDL_Window* window;
	SDL_GameController* controller;

	TextureManager textureManager;
	ShaderManager shaderManager;
	ParticleManager particleManager;
	LocalPlayer localPlayer;
	HeldBlock heldBlock;
	SelectedBlock selectedBlock;
	LevelRenderer levelRenderer;
	LevelGenerator levelGenerator;
	Level level;
	Random random;
	Timer timer;
	UI ui;
	Frustum frustum;
	Network network;

	GLuint shader;
	GLuint atlasTexture;

	GLuint projectionMatrixUniform;
	GLuint viewMatrixUniform;
	GLuint modelMatrixUniform;

	GLuint playerPositionUniform;
	GLuint fragmentOffsetUniform;

	GLuint fogEnableUniform;
	GLuint fogDistanceUniform;
	GLuint fogColorUniform;

	GLuint positionAttribute;
	GLuint uvAttribute;
	GLuint shadeAttribute;

	int height;
	int width;
	bool fullscreen;

	int windowHeight;
	int windowWidth;

	float scaledHeight;
	float scaledWidth;

	uint64_t lastTick;
	uint64_t lastChunkUpdates;
	uint64_t lastFrameRate;
	uint64_t frameRate;
	size_t chunkUpdates;

	float fogDistance;
	glm::vec4 fogColor;

	glm::mat4 orthographicProjectionMatrix;
	glm::mat4 perspectiveProjectionMatrix;
	glm::mat4 viewMatrix;

	const glm::mat4 IDENTITY_MATRIX = glm::mat4(1.0f);

	const float FIELD_OF_VIEW = 70.0f;
	const float NEAR_PLANE = 0.01f;
	const float FAR_PLANE = 1000.0f;
	const float TICK_RATE = 20.0f;
} game;