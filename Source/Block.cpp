#include "Block.h"

static const auto DEFAULT_BOUNDING_BOX = AABB{ 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };

const Block::Definition Block::Definitions[] = {
	{  0,  0,  0, 1.0f,  DEFAULT_BOUNDING_BOX, false, Block::DrawType::DRAW_GAS,    Block::CollideType::COLLIDE_NONE,  }, /* AIR */
	{  1,  1,  1, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* STONE */
	{  0,  3,  2, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* GRASS */
	{  2,  2,  2, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* DIRT */
	{ 16, 16, 16, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* COBBLE */
	{  4,  4,  4, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* WOOD */
	{ 15, 15, 15, 1.0f,  DEFAULT_BOUNDING_BOX, false, Block::DrawType::DRAW_SPRITE, Block::CollideType::COLLIDE_NONE,  }, /* SAPLING */
	{ 17, 17, 17, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* BEDROCK */
	{ 14, 14, 14, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_TRANSLUCENT, Block::CollideType::COLLIDE_LIQUID, },/* WATER */
	{ 14, 14, 14, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_TRANSLUCENT, Block::CollideType::COLLIDE_LIQUID, },/* STILL_WATER */
	{ 30, 30, 30, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_TRANSLUCENT, Block::CollideType::COLLIDE_LIQUID, }, /* LAVA */
	{ 30, 30, 30, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_TRANSLUCENT, Block::CollideType::COLLIDE_LIQUID, }, /* STILL_LAVA */
	{ 18, 18, 18, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* SAND */
	{ 19, 19, 19, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* GRAVEL */
	{ 32, 32, 32, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* GOLD_ORE */
	{ 33, 33, 33, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* IRON_ORE */
	{ 34, 34, 34, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* COAL_ORE */
	{ 21, 20, 21, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* LOG */
	{ 22, 22, 22, 1.0f,  DEFAULT_BOUNDING_BOX, false, Block::DrawType::DRAW_TRANSPARENT_THICK, Block::CollideType::COLLIDE_SOLID, }, /* LEAVES */
	{ 48, 48, 48, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* SPONGE */
	{ 49, 49, 49, 1.0f,  DEFAULT_BOUNDING_BOX, false, Block::DrawType::DRAW_TRANSPARENT, Block::CollideType::COLLIDE_SOLID, },/* GLASS */
	{ 64, 64, 64, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* RED */
	{ 65, 65, 65, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* ORANGE */
	{ 66, 66, 66, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* YELLOW */
	{ 67, 67, 67, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* LIME */
	{ 68, 68, 68, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* GREEN */
	{ 69, 69, 69, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* TEAL */
	{ 70, 70, 70, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* AQUA */
	{ 71, 71, 71, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* CYAN */
	{ 72, 72, 72, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* BLUE */
	{ 73, 73, 73, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* INDIGO */
	{ 74, 74, 74, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* VIOLET */
	{ 75, 75, 75, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* MAGNETA */
	{ 76, 76, 76, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* PINK */
	{ 77, 77, 77, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* BLACK */
	{ 78, 78, 78, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* GRAY */
	{ 79, 79, 79, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* WHITE */
	{ 13, 13, 13, 1.0f,  AABB{0.30f, 0.0f, 0.30f, 0.70f, 0.55f, 0.70f}, false, Block::DrawType::DRAW_SPRITE, Block::CollideType::COLLIDE_NONE, }, /* DANDELION */
	{ 12, 12, 12, 1.0f,  AABB{0.30f, 0.0f, 0.30f, 0.70f, 0.70f, 0.70f}, false, Block::DrawType::DRAW_SPRITE, Block::CollideType::COLLIDE_NONE, }, /* ROSE */
	{ 29, 29, 29, 1.0f,  AABB{0.30f, 0.0f, 0.30f, 0.70f, 0.45f, 0.70f}, false, Block::DrawType::DRAW_SPRITE, Block::CollideType::COLLIDE_NONE, }, /* BROWN_SHROOM */
	{ 28, 28, 28, 1.0f,  AABB{0.23f, 0.0f, 0.23f, 0.77f, 0.43f, 0.77f}, false, Block::DrawType::DRAW_SPRITE, Block::CollideType::COLLIDE_NONE, }, /* RED_SHROOM */
	{ 24, 40, 56, 1.0f,  DEFAULT_BOUNDING_BOX, true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* GOLD */
	{ 23, 39, 55, 1.0f,  DEFAULT_BOUNDING_BOX, true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* IRON */
	{  6,  5,  6, 1.0f,  DEFAULT_BOUNDING_BOX, true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* DOUBLE_SLAB */
	{  6,  5,  6, 0.5f,  AABB{0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f}, true, Block::DrawType::DRAW_OPAQUE_SMALL, Block::CollideType::COLLIDE_SOLID, }, /* SLAB */
	{  7,  7,  7, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* BRICK */
	{  9,  8, 10, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* TNT */
	{  4, 35,  4, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* BOOKSHELF */
	{ 36, 36, 36, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* MOSSY_ROCKS */
	{ 37, 37, 37, 1.0f,  DEFAULT_BOUNDING_BOX,  true, Block::DrawType::DRAW_OPAQUE, Block::CollideType::COLLIDE_SOLID, }, /* OBSIDIAN */
};