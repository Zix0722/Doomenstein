#pragma once
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/IntVec2.hpp"

class TileDefinition;
class Tile
{
public:
	Tile();
	~Tile();
public:
	AABB3 m_bounds;
	TileDefinition* m_def;
};