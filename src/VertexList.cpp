#include "VertexList.h"
#include "Game.h"

#include <glm/glm.hpp>
#include <cstdlib>

void VertexList::init(Allocator* allocator_)
{
  allocator = allocator_;
  index = 0;
  length = 0;
  bufferLength = 0;
  
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);

  glEnableVertexAttribArray(game.positionAttribute);
  glEnableVertexAttribArray(game.uvAttribute);
  glEnableVertexAttribArray(game.shadeAttribute);

  glVertexAttribPointer(game.positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexList::Vertex), 0);
  glVertexAttribPointer(game.uvAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(VertexList::Vertex), (void*)sizeof(glm::vec3));
  glVertexAttribPointer(game.shadeAttribute, 1, GL_FLOAT, GL_FALSE, sizeof(VertexList::Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
}

void VertexList::init(size_t size)
{
  init(new Allocator(size));
}

void VertexList::destroy()
{
  glDeleteBuffers(1, &buffer);
  glDeleteVertexArrays(1, &vao);

  delete allocator;
}

void VertexList::reset()
{
  index = 0;
}

void VertexList::update()
{
  length = index;

  if (length)
  {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    if (length > bufferLength)
    {
      glBufferData(GL_ARRAY_BUFFER, length * sizeof(VertexList::Vertex), allocator->data, GL_DYNAMIC_DRAW);

      bufferLength = length;
    }
    else
    {
      glBufferSubData(GL_ARRAY_BUFFER, 0, length * sizeof(VertexList::Vertex), allocator->data);
    }

    index = 0;
  }
}

void VertexList::render()
{
  if (length)
  {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)length);
  }
}