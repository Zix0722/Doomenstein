#include "TileDefinition.hpp"

std::vector<TileDefinition*> TileDefinition::s_tileDefinitions;

TileDefinition* TileDefinition::GetTileDefByTexelColor(Rgba8 texelColor)
{
	for (int defIndex = 0; defIndex < s_tileDefinitions.size(); ++defIndex)
	{
		if (s_tileDefinitions[defIndex]->m_mapImagePixelColor == texelColor)
		{
			return s_tileDefinitions[defIndex];
		}
	}
	return nullptr;
}

TileDefinition::TileDefinition()
{

}

TileDefinition::~TileDefinition()
{

}

bool TileDefinition::LoadFromXmlElement(XmlElement const& element)
{
	m_name = ParseXMLAttribute(element, "name", m_name);
	m_isSolid = ParseXMLAttribute(element, "isSolid", m_isSolid);
	m_mapImagePixelColor = ParseXMLAttribute(element, "mapImagePixelColor", m_mapImagePixelColor);
	m_floorSpriteCoords = ParseXMLAttribute(element, "floorSpriteCoords", m_floorSpriteCoords);
	m_ceillingSpriteCoords = ParseXMLAttribute(element, "ceilingSpriteCoords", m_ceillingSpriteCoords);
	m_wallSpriteCoords = ParseXMLAttribute(element, "wallSpriteCoords", m_wallSpriteCoords);
	return true;
}

