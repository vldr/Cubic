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

#if defined(EMSCRIPTEN)
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
        vec2 position = fract(fragmentTextureCoordinate) * 16.0;
        vec2 size = floor(fragmentTextureCoordinate);
        vec2 textureCoordinate = mix(
            fragmentTextureCoordinate, 
            floor(position) / 16.0 + mod(position * size, 1.0) / 16.0, 
            float(size.x > 1.0 || size.y > 1.0)
        );

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
    this->atlasTexture = textureManager.load(terrainResourceTexture, sizeof(terrainResourceTexture));

#if defined(ANDROID)
    this->fullscreen = true;
#else
    this->fullscreen = false;
#endif

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
   
    network.render();
    levelRenderer.render();
    particleManager.render();

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
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
        if (event.key.keysym.sym == SDLK_F1)
        {
            static bool state = false;

            glPolygonMode(GL_FRONT_AND_BACK, state ? GL_FILL : GL_LINE);

            state = !state;
        }
#endif

        if (event.key.keysym.sym == SDLK_F2)
        {
            ui.log("Players: %d", network.count());
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
            ui.log("CRC32 checksum: %X", hash);
        }

        if (event.key.keysym.sym == SDLK_F4)
        {
            ui.log("Build date: %s %s", __DATE__, __TIME__);
        }

        if (event.key.keysym.sym == SDLK_F5)
        {
            ui.isTouch = !ui.isTouch;
            ui.update();

            resize();
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

#if defined(EMSCRIPTEN)
    fullscreen = is_fullscreen();
    width = std::lround(windowWidth * emscripten_get_device_pixel_ratio());
    height = std::lround(windowHeight * emscripten_get_device_pixel_ratio());
#else
    SDL_GL_GetDrawableSize(window, &width, &height);
#endif

    glViewport(0, 0, width, height);

    int scaleFactor = 1;
    int maxScaleFactor = ui.isTouch ? 6 : 3;

    while (scaleFactor < maxScaleFactor && width / (scaleFactor + 1) >= 280 && height / (scaleFactor + 1) >= 200)
        scaleFactor++;

    scaledWidth = float(width) / float(scaleFactor);
    scaledHeight = float(height) / float(scaleFactor);

    orthographicProjectionMatrix = glm::ortho(0.0f, scaledWidth, scaledHeight, 0.0f, -1000.0f, 1000.0f);
    perspectiveProjectionMatrix = glm::perspective(
        glm::radians(FIELD_OF_VIEW),
        GLfloat(width) / GLfloat(height),
        NEAR_PLANE,
        FAR_PLANE
    );

    ui.update();
}