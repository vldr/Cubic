#include "Game.h"
#include "LocalPlayer.h"
#include "LevelRenderer.h"
#include "LevelGenerator.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "Random.h"
#include "Timer.h"
#include "Resources.h"
#include "VertexList.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <ctime>
#include <cstdio>
#include <sstream>

#ifdef EMSCRIPTEN
#include <emscripten/html5.h>

EM_JS(bool, is_fullscreen, (), {
  return window.fullScreen ||
   (window.innerWidth == screen.width && window.innerHeight == screen.height);
});
#endif

static const GLchar* fragmentSource = R""""(#version 100
    precision highp float;

    uniform sampler2D TextureSample;
    uniform vec2 FragmentOffset;
    uniform vec3 PlayerPosition;

    uniform float FogEnable;
    uniform float FogDistance;
    uniform vec4 FogColor;

    varying vec3 fragmentPosition;
    varying vec2 fragmentTextureCoordinate;
    varying float fragmentShade;

    void main() 
    {
        vec2 textureCoordinate = fragmentTextureCoordinate;

	    vec2 size = floor(textureCoordinate);
        if (size != vec2(0, 0))
        {
            vec2 position = fract(textureCoordinate);
            vec2 flooredPosition = floor(position * 16.0) / 16.0;
        
	        textureCoordinate = flooredPosition + mod(((textureCoordinate - flooredPosition) * size) * 16.0, 1.0) / 16.0;
        }

        vec4 color = texture2D(TextureSample, textureCoordinate + FragmentOffset);
        color.rgb *= fragmentShade;

        if (color.a == 0.0)
        {
            discard;
        }

        float distance = length(fragmentPosition - PlayerPosition);

        float factor = (FogDistance - distance) / FogDistance;
        factor = max(FogEnable, clamp(factor, 0.0, 1.0));

        gl_FragColor = mix(FogColor, color, factor);
    }
)"""";

static const GLchar* vertexSource = R""""(#version 100
    uniform mat4 View, Projection, Model;

    attribute vec3 position;
    attribute vec2 uv;
    attribute float shade;

    varying vec3 fragmentPosition;
    varying vec2 fragmentTextureCoordinate;
    varying float fragmentShade;

    void main()
    {
        fragmentPosition = (Model * vec4(position, 1.0)).xyz;
        fragmentTextureCoordinate = uv;
        fragmentShade = shade;

        gl_Position = Projection * View * Model * vec4(position, 1.0);
    }
)"""";

void Game::init(SDL_Window* sdlWindow)
{
    this->window = sdlWindow;
    this->shader = shaderManager.load(vertexSource, fragmentSource);

    glUseProgram(shader);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    this->positionAttribute = glGetAttribLocation(shader, "position");
    this->uvAttribute = glGetAttribLocation(shader, "uv");
    this->shadeAttribute = glGetAttribLocation(shader, "shade");

    this->fragmentOffsetUniform = glGetUniformLocation(shader, "FragmentOffset");
    this->playerPositionUniform = glGetUniformLocation(shader, "PlayerPosition");
    this->fogEnableUniform = glGetUniformLocation(shader, "FogEnable");
    this->fogDistanceUniform = glGetUniformLocation(shader, "FogDistance");
    this->fogColorUniform = glGetUniformLocation(shader, "FogColor");
    this->projectionMatrixUniform = glGetUniformLocation(shader, "Projection");
    this->viewMatrixUniform = glGetUniformLocation(shader, "View");
    this->modelMatrixUniform = glGetUniformLocation(shader, "Model");

    this->random.init(std::time(nullptr));
    this->timer.init(TICK_RATE);
    this->localPlayer.init(this);
    this->frustum.init(this);
    this->network.init(this);
    this->ui.init(this);
    this->heldBlock.init(this);
    this->selectedBlock.init(this);
    this->particleManager.init(this);
    this->levelGenerator.init(this);
    this->levelRenderer.init(this);
    this->lastTick = timer.milliTime();
    this->frameRate = 0;
    this->fullscreen = false;
    this->atlasTexture = textureManager.load(terrainResourceTexture, sizeof(terrainResourceTexture));

    resize();
}

void Game::run()
{
    timer.update();

    for (int i = 0; i < timer.elapsedTicks; i++)
    {
        localPlayer.tick();
        particleManager.tick();
        level.tick();
        levelRenderer.tick();
        heldBlock.tick();
        network.tick();

        timer.tick();
    }

    glClearColor(fogColor.r, fogColor.g, fogColor.b, fogColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(perspectiveProjectionMatrix));
    glUniform3fv(playerPositionUniform, 1, glm::value_ptr(localPlayer.position));

    glUniform1f(fogEnableUniform, 0.0f);
    glUniform1f(fogDistanceUniform, fogDistance);
    glUniform4fv(fogColorUniform, 1, glm::value_ptr(fogColor));

    levelGenerator.update();
    localPlayer.update();
    frustum.update();

    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewMatrix));
   
    levelRenderer.render();
    particleManager.render();
    network.render();

    selectedBlock.renderPost();
    levelRenderer.renderPost();

    glClear(GL_DEPTH_BUFFER_BIT);
    glUniform1f(fogEnableUniform, 1.0f);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, glm::value_ptr(IDENTITY_MATRIX));

    heldBlock.render();

    glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(orthographicProjectionMatrix));

    ui.render();

    frameRate++;
    if (timer.milliTime() - lastTick > 1000.0f)
    {
        lastFrameRate = frameRate;
        lastChunkUpdates = chunkUpdates;

        frameRate = 0;
        chunkUpdates = 0;
        lastTick = timer.milliTime();

        ui.update();
    }
}

void Game::input(const SDL_Event& event)
{
    if (event.type == SDL_WINDOWEVENT)
    {
        if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        {
            resize();
        }
        else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
        {
            if (ui.state == UI::State::None)
            {
                ui.openMainMenu();
            }
        }
    }
    else if (event.type == SDL_KEYUP)
    {
#ifndef EMSCRIPTEN
        if (event.key.keysym.sym == SDLK_F1)
        {
            static bool state = false;

            glPolygonMode(GL_FRONT_AND_BACK, state ? GL_FILL : GL_LINE);

            state = !state;
        }
#endif

        if (event.key.keysym.sym == SDLK_F2)
        {
            ui.log("Players: " + std::to_string(network.count()));
        }

        if (event.key.keysym.sym == SDLK_F3)
        {
            auto crc32 = [](unsigned char* data, size_t length) {
                unsigned int crc;
                crc = 0xFFFFFFFFu;

                for (size_t i = 0; i < length; i++)
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

            auto hash = crc32(level.blocks.get(), Level::WIDTH * Level::HEIGHT * Level::DEPTH);
            ui.log("CRC32 checksum: " + std::to_string(hash));
        }

        if (event.key.keysym.sym == SDLK_F4)
        {
            std::stringstream log{};
            log << "Build date: ";
            log << __DATE__;
            log << " ";
            log << __TIME__;

            ui.log(log.str());
        }

        if (event.key.keysym.sym == SDLK_F5)
        {
            ui.isTouch = !ui.isTouch;
            ui.update();
        }
    }
    else if (event.type == SDL_QUIT)
    {
        exit(0);
    }

    if (ui.input(event))
    {
        localPlayer.input(event);
    }
}

void Game::resize()
{
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

#ifdef EMSCRIPTEN
    width = std::lround(windowWidth * emscripten_get_device_pixel_ratio());
    height = std::lround(windowHeight * emscripten_get_device_pixel_ratio());
#else
    SDL_GL_GetDrawableSize(window, &width, &height);
#endif

    glViewport(0, 0, width, height);

    scaledWidth = float(width);
    scaledHeight = float(height);

    int maxScaleFactor = 3;
    int scaleFactor = 1;

#ifdef EMSCRIPTEN
    maxScaleFactor *= emscripten_get_device_pixel_ratio();
    fullscreen = is_fullscreen();
#endif

    while (scaleFactor < maxScaleFactor && scaledWidth / (scaleFactor + 1) >= 320 && scaledHeight / (scaleFactor + 1) >= 240)
    {
        scaleFactor++;
    }

    scaledWidth /= scaleFactor;
    scaledHeight /= scaleFactor;

    orthographicProjectionMatrix = glm::ortho(0.0f, scaledWidth, scaledHeight, 0.0f, -1000.0f, 1000.0f);
    perspectiveProjectionMatrix = glm::perspective(
        glm::radians(FIELD_OF_VIEW),
        GLfloat(width) / GLfloat(height),
        NEAR_PLANE,
        FAR_PLANE
    );

    ui.update();
}