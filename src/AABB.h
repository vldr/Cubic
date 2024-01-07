#pragma once
#include <glm/glm.hpp>

struct AABBPosition;

class AABB
{
public:
	enum Axis { X, Y, Z };

	float x0, y0, z0;
	float x1, y1, z1;

	template<AABB::Axis AXIS>
	glm::vec3 intersection(const glm::vec3& v0, const glm::vec3& v1, float s) const;
	bool intersects(AABB aabb) const;
	bool intersectsInner(AABB aabb) const; 
	bool intersectsX(const glm::vec3& v) const;
	bool intersectsY(const glm::vec3& v) const;
	bool intersectsZ(const glm::vec3& v) const;

	AABB expand(float x, float y, float z) const;
	AABB expand(int x, int y, int z) const;
	AABB grow(float x, float y, float z) const;
	AABB grow(int x, int y, int z) const;
	AABB move(float x, float y, float z) const;
	AABB move(int x, int y, int z) const;

	AABBPosition clip(const glm::vec3& v1, const glm::vec3& v2) const;
	float clipX(AABB aabb, float xa) const;
	float clipY(AABB aabb, float ya) const;
	float clipZ(AABB aabb, float za) const;
};

