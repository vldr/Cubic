#pragma once

class Chunk;

class Frustum 
{
public:
	void update();
	bool contains(Chunk* chunk);
private:
	void normalizePlane(int plane);

	float planes[6][16];
	float clip[16];
};