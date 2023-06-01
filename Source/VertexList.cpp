#include "VertexList.h"

#include <glm/glm.hpp>
#include <cstdlib>

void VertexList::init(size_t capacity)
{
	this->index = 0;
	this->length = 0;
    this->bufferLength = 0;
    this->capacity = capacity;
    this->vertices = static_cast<VertexList::Vertex*>(
        std::malloc(this->capacity * sizeof(VertexList::Vertex))
    );

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexList::Vertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexList::Vertex), (void*)sizeof(glm::vec3));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(VertexList::Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
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
			glBufferData(GL_ARRAY_BUFFER, length * sizeof(VertexList::Vertex), vertices, GL_DYNAMIC_DRAW);

			bufferLength = length;
		}
		else
		{
			glBufferSubData(GL_ARRAY_BUFFER, 0, length * sizeof(VertexList::Vertex), vertices);
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

void VertexList::push(const VertexList::Vertex& vertex)
{
    if (index == capacity)
    {
		capacity *= 2;
        vertices = static_cast<VertexList::Vertex*>(
            std::realloc(vertices, capacity * sizeof(VertexList::Vertex))
        );
    }

    vertices[index++] = std::move(vertex);
}