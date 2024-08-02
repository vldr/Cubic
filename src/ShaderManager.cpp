#include "ShaderManager.h"

#include <cstdio>

GLuint ShaderManager::load(const char* vertexSource, const char* fragmentSource)
{
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSource, nullptr);
  glCompileShader(vertexShader);
  GLint compiled;

  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
  if (!compiled)
  {
    printf("Failed to compile vertex shader: \n");
    GLint logSize;
    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logSize);
    char* logMsg = new char[logSize];
    glGetShaderInfoLog(vertexShader, logSize, NULL, logMsg);
    printf("%s\n", logMsg);
    delete[] logMsg;

    return -1;
  }

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
  glCompileShader(fragmentShader);

  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);
  if (!compiled)
  {
    printf("Failed to compile fragment shader: \n");
    GLint logSize;
    glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logSize);
    char* logMsg = new char[logSize];
    glGetShaderInfoLog(fragmentShader, logSize, NULL, logMsg);
    printf("%s\n", logMsg);
    delete[] logMsg;

    return -1;
  }

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);

  return shaderProgram;
}