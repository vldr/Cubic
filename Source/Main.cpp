#include "Game.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

Game game;

#ifdef EMSCRIPTEN
static EM_BOOL resize(int eventType, const EmscriptenUiEvent* uiEvent, void* userData)
{
	game.resize();

	return true;
}

static EM_BOOL pointer_lock_change(int eventType, const EmscriptenPointerlockChangeEvent* pointerlockChangeEvent, void* userData)
{
	static bool previousIsActive = false;

	if (!pointerlockChangeEvent->isActive && pointerlockChangeEvent->isActive != previousIsActive)
	{
		SDL_Event event;
		event.type = SDL_WINDOWEVENT;
		event.window.event = SDL_WINDOWEVENT_FOCUS_LOST;

		game.input(event);
	}

	previousIsActive = pointerlockChangeEvent->isActive;

	return true;
}
#endif

void loop()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		game.input(event);
	}

	game.run();

	SDL_GL_SwapWindow(game.window);
}

int main(int argc, char** argv)
{
#ifdef _WIN32
	SetProcessDPIAware();
#endif

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL_Init failed: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	auto window = SDL_CreateWindow("Cubic",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1280, 720, 
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN 
#ifndef EMSCRIPTEN
		| SDL_WINDOW_RESIZABLE
#endif
	);

	if (!window)
	{
		printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

#ifdef EMSCRIPTEN
	EmscriptenWebGLContextAttributes attributes;
	emscripten_webgl_init_context_attributes(&attributes);
	attributes.alpha = false;
	attributes.stencil = false;
	attributes.depth = true;
	attributes.antialias = false;
	attributes.majorVersion = 2;
	attributes.powerPreference = EM_WEBGL_POWER_PREFERENCE_HIGH_PERFORMANCE;

	auto context = emscripten_webgl_create_context("#canvas", &attributes);
	if (!context)
	{
		attributes.majorVersion = 1;

		context = emscripten_webgl_create_context("#canvas", &attributes);
	}

	if (!context)
	{
		printf("emscripten_webgl_create_context failed\n");
		return EXIT_FAILURE;
	}

	emscripten_webgl_make_context_current(context);
	emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, false, pointer_lock_change);
	emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, false, resize);

	EM_ASM(
		FS.mkdir('/saves');
		FS.mount(IDBFS, {}, '/saves');
		FS.syncfs(true, function(err) {
			if (err)
			{
				console.log(err);
			}
		});
	);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	auto context = SDL_GL_CreateContext(window);

	if (!context)
	{
		printf("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}
#endif

	glewInit();
	game.init(window);

#ifdef EMSCRIPTEN
	emscripten_set_main_loop(loop, 0, true);
#else
	for (;;)
	{
		loop();
	}
#endif

	return 0;
}