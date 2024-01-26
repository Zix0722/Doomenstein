#pragma once
#include <string>
#include "Engine/Core/Image.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "SpawnInfo.hpp"

class Texture;
class Shader;

class MapDefinition
{
public:
	MapDefinition();
	~MapDefinition();

	bool LoadFromXmlElement(XmlElement const& element);
	bool LoadSpawnInfoFromElement(XmlElement const& element);
	static MapDefinition* GetDefinitionByName(std::string const& name);
	static std::vector<MapDefinition*> s_mapDefinitions;
public:
	std::string m_name;
	Image m_image;
	Shader* m_shader  = nullptr;
	Texture* m_spriteSheetTexture = nullptr;
	IntVec2 m_spriteSheetCellCount;
	std::vector<SpawnInfo*> m_SpawnInfos;
};