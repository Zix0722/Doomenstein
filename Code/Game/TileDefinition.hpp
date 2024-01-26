#pragma once
#include <string>
#include "Engine/Core/Image.hpp"
#include "Engine/Core/XmlUtils.hpp"

class TileDefinition
{
public:
	TileDefinition();
	~TileDefinition();

	bool LoadFromXmlElement(XmlElement const& element);
	static std::vector<TileDefinition*> s_tileDefinitions;

	static TileDefinition* GetTileDefByTexelColor(Rgba8 texelColor);
public:
	std::string m_name;
	bool m_isSolid = false;
	Rgba8 m_mapImagePixelColor;
	IntVec2 m_floorSpriteCoords = IntVec2(-1,-1);
	IntVec2 m_ceillingSpriteCoords = IntVec2(-1, -1);
	IntVec2 m_wallSpriteCoords = IntVec2(-1, -1);
};