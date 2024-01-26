#include "MapDefinition.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "GameCommon.hpp"

std::vector<MapDefinition*> MapDefinition::s_mapDefinitions;
MapDefinition::MapDefinition()
{

}

MapDefinition::~MapDefinition()
{
	m_SpawnInfos.clear();
}

bool MapDefinition::LoadFromXmlElement(XmlElement const& element)
{
	m_name = ParseXMLAttribute(element, "name" , m_name);
	std::string str;
	m_image = Image(ParseXMLAttribute(element, "image", str).c_str());
	std::string shader;
	shader = ParseXMLAttribute(element, "shader", str).c_str();
	if (shader == "Default")
	{
		m_shader = g_theRenderer->GetDefalutShader();
	}
	else
	{
		m_shader = g_theRenderer->CreateShader(shader.c_str());
		//m_shader = g_theRenderer->GetDefalutShader();
	}
	Image textureImage = Image(ParseXMLAttribute(element, "spriteSheetTexture", str).c_str());
	m_spriteSheetTexture = g_theRenderer->CreateTextureFromImage(textureImage);
	m_spriteSheetCellCount = ParseXMLAttribute(element, "spriteSheetCellCount", m_spriteSheetCellCount);
	return true;
}

bool MapDefinition::LoadSpawnInfoFromElement(XmlElement const& element)
{
	SpawnInfo* newInfo = new SpawnInfo();
	newInfo->m_actor = ParseXMLAttribute(element, "actor", newInfo->m_actor);
	newInfo->m_position = ParseXMLAttribute(element, "position", newInfo->m_position);
	newInfo->m_orientation = ParseXMLAttribute(element, "orientation", newInfo->m_orientation);
	m_SpawnInfos.push_back(newInfo);
	return true;
}

MapDefinition* MapDefinition::GetDefinitionByName(std::string const& name)
{
	for (int defIndex = 0; defIndex < s_mapDefinitions.size(); ++defIndex)
	{
		if (s_mapDefinitions[defIndex]->m_name == name)
		{
			return s_mapDefinitions[defIndex];
		}
	}
	return nullptr;
}

