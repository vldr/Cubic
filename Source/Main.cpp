#include "Game.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

Game game;

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
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_SetRelativeMouseMode(SDL_TRUE);

    auto window = SDL_CreateWindow("Cubic", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        1280, 720, 
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window)
    {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

#ifdef EMSCRIPTEN
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    auto context = SDL_GL_CreateContext(window);

#ifdef EMSCRIPTEN
    if (!context)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

        context = SDL_GL_CreateContext(window);
    }
#endif

    if (!context)
    {
        printf("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

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

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}