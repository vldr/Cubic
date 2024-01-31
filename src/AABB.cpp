#include "AABB.h"
#include "AABBPosition.h"

static const auto MAX_VECTOR = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
static const auto EPSILON = 1.0E-7f;

AABB AABB::expand(float x, float y, float z) const
{
	AABB aabb = *this;
	aabb.x0 += x < 0.0f ? x : 0.0f;
	aabb.y0 += y < 0.0f ? y : 0.0f;
	aabb.z0 += z < 0.0f ? z : 0.0f;
	aabb.x1 += x > 0.0f ? x : 0.0f;
	aabb.y1 += y > 0.0f ? y : 0.0f;
	aabb.z1 += z > 0.0f ? z : 0.0f;

	return aabb;
}

AABB AABB::expand(int x, int y, int z) const
{
	return expand((float)x, (float)y, (float)z);
}

AABB AABB::grow(float x, float y, float z) const
{
	AABB aabb = *this;
	aabb.x0 = x0 - x;
	aabb.y0 = y0 - y;
	aabb.z0 = z0 - z;
	aabb.x1 = x1 + x;
	aabb.y1 = y1 + y;
	aabb.z1 = z1 + z;

	return aabb;
}

AABB AABB::grow(int x, int y, int z) const
{
	return grow((float)x, (float)y, (float)z);
}

AABB AABB::move(float x, float y, float z) const
{
	AABB aabb = *this;
	aabb.x0 = x0 + x;
	aabb.y0 = y0 + y;
	aabb.z0 = z0 + z;
	aabb.x1 = x1 + x;
	aabb.y1 = y1 + y;
	aabb.z1 = z1 + z;

	return aabb;
}

AABB AABB::move(int x, int y, int z) const
{
	return move((float)x, (float)y, (float)z);
}

float AABB::clipX(AABB aabb, float velocityX) const
{
	if (aabb.y1 <= y0 || aabb.y0 >= y1 || aabb.z1 <= z0 || aabb.z0 >= z1)
	{
		return velocityX;
	}

	float max = x0 - aabb.x1 - EPSILON;
	if (velocityX > 0.0 && aabb.x1 <= x0 && max < velocityX) 
	{ 
		velocityX = max; 
	}

	max = x1 - aabb.x0 + EPSILON;
	if (velocityX < 0.0 && aabb.x0 >= x1 && max > velocityX) 
	{ 
		velocityX = max; 
	}

	return velocityX;
}

float AABB::clipY(AABB aabb, float velocityY) const
{
	if (aabb.x1 <= x0 || aabb.x0 >= x1 || aabb.z1 <= z0 || aabb.z0 >= z1)
	{
		return velocityY;
	}

	float max = y0 - aabb.y1 - EPSILON;
	if (velocityY > 0.0 && aabb.y1 <= y0 && max < velocityY) 
	{ 
		velocityY = max; 
	}

	max = y1 - aabb.y0 + EPSILON;
	if (velocityY < 0.0 && aabb.y0 >= y1 && max > velocityY) 
	{ 
		velocityY = max; 
	}

	return velocityY;
}

float AABB::clipZ(AABB aabb, float velocityZ) const
{
	if (aabb.x1 <= x0 || aabb.x0 >= x1 || aabb.y1 <= y0 || aabb.y0 >= y1)
	{
		return velocityZ;
	}

	float max = z0 - aabb.z1 - EPSILON;
	if (velocityZ > 0.0 && aabb.z1 <= z0 && max < velocityZ) 
	{ 
		velocityZ = max; 
	}

	max = z1 - aabb.z0 + EPSILON;
	if (velocityZ < 0.0 && aabb.z0 >= z1 && max > velocityZ) 
	{ 
		velocityZ = max; 
	}

	return velocityZ;
}

bool AABB::intersects(AABB aabb) const
{
	return aabb.x1 > x0 && aabb.x0 < x1 ? (aabb.y1 > y0 && aabb.y0 < y1 ? aabb.z1 > z0 && aabb.z0 < z1 : false) : false;
}

bool AABB::intersectsInner(AABB aabb) const
{
	return aabb.x1 >= x0 && aabb.x0 <= x1 ? (aabb.y1 >= y0 && aabb.y0 <= y1 ? aabb.z1 >= z0 && aabb.z0 <= z1 : false) : false;
}

bool AABB::intersectsX(const glm::vec3& v) const 
{
	return v.y >= y0 && v.y <= y1 && v.z >= z0 && v.z <= z1;
}

bool AABB::intersectsY(const glm::vec3& v) const
{
	return v.x >= x0 && v.x <= x1 && v.z >= z0 && v.z <= z1;
}

bool AABB::intersectsZ(const glm::vec3& v) const
{
	return v.x >= x0 && v.x <= x1 && v.y >= y0 && v.y <= y1;
}

template <AABB::Axis AXIS>
glm::vec3 AABB::intersection(const glm::vec3& start, const glm::vec3& end, float s) const
{
	auto d = end - start;
	auto da = d[AXIS];

	s = (s - start[AXIS]) / da;

	if (glm::abs(da) < EPSILON || s < 0.0f || s > 1.0f)
	{
		return MAX_VECTOR;
	}
	else
	{
		return glm::mix(start, end, s);
	}
}

AABBPosition AABB::clip(const glm::vec3& start, const glm::vec3& end) const
{
	auto minX = intersection<AABB::Axis::X>(start, end, x0);
	auto maxX = intersection<AABB::Axis::X>(start, end, x1);
	auto minY = intersection<AABB::Axis::Y>(start, end, y0);
	auto maxY = intersection<AABB::Axis::Y>(start, end, y1);
	auto minZ = intersection<AABB::Axis::Z>(start, end, z0);
	auto maxZ = intersection<AABB::Axis::Z>(start, end, z1);

	if (!intersectsX(minX)) { minX = MAX_VECTOR; }
	if (!intersectsX(maxX)) { maxX = MAX_VECTOR; }
	if (!intersectsY(minY)) { minY = MAX_VECTOR; }
	if (!intersectsY(maxY)) { maxY = MAX_VECTOR; }
	if (!intersectsZ(minZ)) { minZ = MAX_VECTOR; }
	if (!intersectsZ(maxZ)) { maxZ = MAX_VECTOR; }

	auto intersection = MAX_VECTOR;
	if (minX != MAX_VECTOR) { intersection = minX; }
	if (maxX != MAX_VECTOR && (intersection == MAX_VECTOR || glm::distance(start, maxX) < glm::distance(start, intersection))) { intersection = maxX; }
	if (minY != MAX_VECTOR && (intersection == MAX_VECTOR || glm::distance(start, minY) < glm::distance(start, intersection))) { intersection = minY; }
	if (maxY != MAX_VECTOR && (intersection == MAX_VECTOR || glm::distance(start, maxY) < glm::distance(start, intersection))) { intersection = maxY; }
	if (minZ != MAX_VECTOR && (intersection == MAX_VECTOR || glm::distance(start, minZ) < glm::distance(start, intersection))) { intersection = minZ; }
	if (maxZ != MAX_VECTOR && (intersection == MAX_VECTOR || glm::distance(start, maxZ) < glm::distance(start, intersection))) { intersection = maxZ; }
	if (intersection == MAX_VECTOR)
	{ 
		AABBPosition aabbPosition;
		aabbPosition.isValid = false;

		return aabbPosition;
	}

	int face = -1;
	if (intersection.x == minX.x && intersection.y == minX.y && intersection.z == minX.z) { face = 4; }
	if (intersection.x == maxX.x && intersection.y == maxX.y && intersection.z == maxX.z) { face = 5; }
	if (intersection.x == minY.x && intersection.y == minY.y && intersection.z == minY.z) { face = 0; }
	if (intersection.x == maxY.x && intersection.y == maxY.y && intersection.z == maxY.z) { face = 1; }
	if (intersection.x == minZ.x && intersection.y == minZ.y && intersection.z == minZ.z) { face = 2; }
	if (intersection.x == maxZ.x && intersection.y == maxZ.y && intersection.z == maxZ.z) { face = 3; }

	AABBPosition aabbPosition;
	aabbPosition.isValid = true;
	aabbPosition.face = face;
	aabbPosition.vector = intersection;

	return aabbPosition;
}
