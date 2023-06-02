#include "Game.h"
#include "LocalPlayer.h"
#include "LevelRenderer.h"
#include "LevelGenerator.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "Random.h"
#include "Timer.h"
#include "Resources.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <ctime>
#include <cstdio>

static const GLchar* fragmentSource = R""""(#version 100
    precision mediump float;

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
        vec4 color = texture2D(TextureSample, fragmentTextureCoordinate + FragmentOffset);
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
    this->random.init(std::time(nullptr));
    this->timer.init(this, TICK_RATE);
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
    this->shader = shaderManager.load(vertexSource, fragmentSource);

    SDL_GetWindowSize(window, &width, &height);
    resize();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(shader);
    fragmentOffsetUniform = glGetUniformLocation(shader, "FragmentOffset");
    playerPositionUniform = glGetUniformLocation(shader, "PlayerPosition");
    fogEnableUniform = glGetUniformLocation(shader, "FogEnable");
    fogDistanceUniform = glGetUniformLocation(shader, "FogDistance");
    fogColorUniform = glGetUniformLocation(shader, "FogColor");
    projectionMatrixUniform = glGetUniformLocation(shader, "Projection");
    viewMatrixUniform = glGetUniformLocation(shader, "View");
    modelMatrixUniform = glGetUniformLocation(shader, "Model");
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(IDENTITY_MATRIX));

    levelGenerator.generate();
    network.connect();
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
            width = event.window.data1;
            height = event.window.data2;

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
    glViewport(0, 0, width, height);

    scaledWidth = float(width);
    scaledHeight = float(height);

    scaleFactor = 1;
    scaleCount = 3;

    while (scaleFactor < scaleCount && scaledWidth / (scaleFactor + 1) >= 320 && scaledHeight / (scaleFactor + 1) >= 240)
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