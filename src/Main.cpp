#include "Game.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(EMSCRIPTEN)
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

Game game;

#if defined(EMSCRIPTEN)
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

	game.render();

	SDL_GL_SwapWindow(game.window);
}

int main(int argc, char** argv)
{
#if defined(_WIN32)
	SetProcessDPIAware();
#endif

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
	{
		printf("SDL_Init failed: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

#if defined(ANDROID) || TARGET_OS_IPHONE
	SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	unsigned int flags = {};
	flags |= SDL_WINDOW_OPENGL;
	flags |= SDL_WINDOW_SHOWN;
	flags |= SDL_WINDOW_RESIZABLE;
	flags |= SDL_WINDOW_ALLOW_HIGHDPI;

#if defined(ANDROID) || TARGET_OS_IPHONE
	flags |= SDL_WINDOW_FULLSCREEN;
#endif

	auto window = SDL_CreateWindow("Cubic",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1280, 720,
		flags
	);

	if (!window)
	{
		printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

#if defined(EMSCRIPTEN)
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
	auto context = SDL_GL_CreateContext(window);
	if (!context)
	{
		printf("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	glewInit();
	
	SDL_GL_SetSwapInterval(1);
#endif

	game.init(window);
	
#if defined(EMSCRIPTEN)
	emscripten_set_main_loop(loop, 0, true);
#else
	for (;;)
	{
		loop();
	}
#endif

	return 0;
}
