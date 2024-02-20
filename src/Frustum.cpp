#include "Frustum.h"
#include "Chunk.h"
#include "Game.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

void Frustum::update()
{
	float* projection = glm::value_ptr(game.perspectiveProjectionMatrix);
	float* view = glm::value_ptr(game.viewMatrix);

	clip[0] = view[0] * projection[0] + view[1] * projection[4] + view[2] * projection[8] + view[3] * projection[12];
	clip[1] = view[0] * projection[1] + view[1] * projection[5] + view[2] * projection[9] + view[3] * projection[13];
	clip[2] = view[0] * projection[2] + view[1] * projection[6] + view[2] * projection[10] + view[3] * projection[14];
	clip[3] = view[0] * projection[3] + view[1] * projection[7] + view[2] * projection[11] + view[3] * projection[15];
	clip[4] = view[4] * projection[0] + view[5] * projection[4] + view[6] * projection[8] + view[7] * projection[12];
	clip[5] = view[4] * projection[1] + view[5] * projection[5] + view[6] * projection[9] + view[7] * projection[13];
	clip[6] = view[4] * projection[2] + view[5] * projection[6] + view[6] * projection[10] + view[7] * projection[14];
	clip[7] = view[4] * projection[3] + view[5] * projection[7] + view[6] * projection[11] + view[7] * projection[15];
	clip[8] = view[8] * projection[0] + view[9] * projection[4] + view[10] * projection[8] + view[11] * projection[12];
	clip[9] = view[8] * projection[1] + view[9] * projection[5] + view[10] * projection[9] + view[11] * projection[13];
	clip[10] = view[8] * projection[2] + view[9] * projection[6] + view[10] * projection[10] + view[11] * projection[14];
	clip[11] = view[8] * projection[3] + view[9] * projection[7] + view[10] * projection[11] + view[11] * projection[15];
	clip[12] = view[12] * projection[0] + view[13] * projection[4] + view[14] * projection[8] + view[15] * projection[12];
	clip[13] = view[12] * projection[1] + view[13] * projection[5] + view[14] * projection[9] + view[15] * projection[13];
	clip[14] = view[12] * projection[2] + view[13] * projection[6] + view[14] * projection[10] + view[15] * projection[14];
	clip[15] = view[12] * projection[3] + view[13] * projection[7] + view[14] * projection[11] + view[15] * projection[15];

	planes[0][0] = clip[0x3] - clip[0x0];
	planes[0][1] = clip[0x7] - clip[0x4];
	planes[0][2] = clip[0xB] - clip[0x8];
	planes[0][3] = clip[0xF] - clip[0xC];
	normalizePlane(0);

	planes[1][0] = clip[0x3] + clip[0x0];
	planes[1][1] = clip[0x7] + clip[0x4];
	planes[1][2] = clip[0xB] + clip[0x8];
	planes[1][3] = clip[0xF] + clip[0xC];
	normalizePlane(1);

	planes[2][0] = clip[0x3] - clip[0x1];
	planes[2][1] = clip[0x7] - clip[0x5];
	planes[2][2] = clip[0xB] - clip[0x9];
	planes[2][3] = clip[0xF] - clip[0xD];
	normalizePlane(2);

	planes[3][0] = clip[0x3] + clip[0x1];
	planes[3][1] = clip[0x7] + clip[0x5];
	planes[3][2] = clip[0xB] + clip[0x9];
	planes[3][3] = clip[0xF] + clip[0xD];
	normalizePlane(3);

	planes[4][0] = clip[0x3] - clip[0x2];
	planes[4][1] = clip[0x7] - clip[0x6];
	planes[4][2] = clip[0xB] - clip[0xA];
	planes[4][3] = clip[0xF] - clip[0xE];
	normalizePlane(4);

	planes[5][0] = clip[0x3] + clip[0x2];
	planes[5][1] = clip[0x7] + clip[0x6];
	planes[5][2] = clip[0xB] + clip[0xA];
	planes[5][3] = clip[0xF] + clip[0xE];
	normalizePlane(5);
}

bool Frustum::contains(Chunk* chunk)
{
	float startX = float(chunk->position.x); 
	float startY = float(chunk->position.y);
	float startZ = float(chunk->position.z);
	float endX = float(chunk->position.x + chunk->SIZE);
	float endY = float(chunk->position.y + chunk->SIZE);
	float endZ = float(chunk->position.z + chunk->SIZE);

	for (int plane = 0; plane < 6; plane++) 
	{
		bool outside = true;
		outside = outside && planes[plane][0] * startX + planes[plane][1] * startY + planes[plane][2] * startZ + planes[plane][3] <= 0.0f;
		outside = outside && planes[plane][0] * endX + planes[plane][1] * startY + planes[plane][2] * startZ + planes[plane][3] <= 0.0f;
		outside = outside && planes[plane][0] * startX + planes[plane][1] * endY + planes[plane][2] * startZ + planes[plane][3] <= 0.0f;
		outside = outside && planes[plane][0] * endX + planes[plane][1] * endY + planes[plane][2] * startZ + planes[plane][3] <= 0.0f;
		outside = outside && planes[plane][0] * startX + planes[plane][1] * startY + planes[plane][2] * endZ + planes[plane][3] <= 0.0f;
		outside = outside && planes[plane][0] * endX + planes[plane][1] * startY + planes[plane][2] * endZ + planes[plane][3] <= 0.0f;
		outside = outside && planes[plane][0] * startX + planes[plane][1] * endY + planes[plane][2] * endZ + planes[plane][3] <= 0.0f;
		outside = outside && planes[plane][0] * endX + planes[plane][1] * endY + planes[plane][2] * endZ + planes[plane][3] <= 0.0f;
		
		if (outside) 
		{ 
			return false; 
		}
	}

	return true;
}

void Frustum::normalizePlane(int plane)
{
	float length = glm::sqrt(
		planes[plane][0] * planes[plane][0] + 
		planes[plane][1] * planes[plane][1] + 
		planes[plane][2] * planes[plane][2]
	);

	planes[plane][0] /= length;
	planes[plane][1] /= length;
	planes[plane][2] /= length;
	planes[plane][3] /= length;
}
