#pragma once

class Chunk;
class Game;

class Frustum 
{
public:
	void init(Game* game);
	void update();
	bool contains(Chunk* chunk);
private:
	void normalizePlane(int plane);

	float planes[6][16];
	float clip[16];

	Game* game;
};