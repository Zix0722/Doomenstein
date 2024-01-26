#include "WeaponDefinition.hpp"
#include "GameCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


std::vector<WeaponDefinition*> WeaponDefinition::s_weaponDefinitions;

WeaponDefinition::WeaponDefinition()
{

}

WeaponDefinition::~WeaponDefinition()
{

}

WeaponDefinition* WeaponDefinition::GetWeaponDefinitionByName(std::string name)
{
	for (int defIndex = 0; defIndex < s_weaponDefinitions.size(); ++defIndex)
	{
		if (WeaponDefinition::s_weaponDefinitions[defIndex]->m_name == name)
		{
  			return s_weaponDefinitions[defIndex];
		}
	}

	return nullptr;
}

bool WeaponDefinition::LoadFromXmlElement(XmlElement const& element)
{
	m_name = ParseXMLAttribute(element, "name", m_name);
	m_refireTime = ParseXMLAttribute(element, "refireTime", m_refireTime);
	m_rayCount = ParseXMLAttribute(element, "rayCount", m_rayCount);
	m_rayCone = ParseXMLAttribute(element, "rayCone", m_rayCone);
	m_rayRange = ParseXMLAttribute(element, "rayRange", m_rayRange);
	m_rayDamage = ParseXMlAttribute(element, "rayDamage", m_rayDamage, '~');
	m_rayImpulse = ParseXMLAttribute(element, "rayImpulse", m_rayImpulse);
	m_projectileCount = ParseXMLAttribute(element, "projectileCount", m_projectileCount);
	m_projectileCone = ParseXMLAttribute(element, "projectileCone", m_projectileCone);
	m_projectileSpeed = ParseXMLAttribute(element, "projectileSpeed", m_projectileSpeed);
	m_projectileActor = ParseXMLAttribute(element, "projectileActor", m_projectileActor);
	m_meleeCount = ParseXMLAttribute(element, "meleeCount", m_meleeCount);
	m_meleeRange = ParseXMLAttribute(element, "meleeRange", m_meleeRange);
	m_meleeArc = ParseXMLAttribute(element, "meleeArc", m_meleeArc);
	m_meleeDamage = ParseXMlAttribute(element, "meleeDamage", m_meleeDamage, '~'); 
	m_meleeImpulse = ParseXMLAttribute(element, "meleeImpulse", m_meleeImpulse);
	XmlElement const* HUDElement = element.FirstChildElement("HUD");
	if (HUDElement)
	{
		std::string shaderHUD;
		shaderHUD = ParseXMLAttribute(*HUDElement, "shader", shaderHUD);
		if (shaderHUD != "")
		{
			m_UIshader = g_theRenderer->CreateShader(shaderHUD.c_str());
		}
		std::string baseTexture;
		baseTexture = ParseXMLAttribute(*HUDElement, "baseTexture", baseTexture);
		m_baseTexture = g_theRenderer->CreateOrGetTextureFromFile(baseTexture.c_str());
		std::string reticleTexure;
		reticleTexure = ParseXMLAttribute(*HUDElement, "reticleTexture", reticleTexure);
		m_reticleTexture = g_theRenderer->CreateOrGetTextureFromFile(reticleTexure.c_str());
		m_reticleSize = ParseXMLAttribute(*HUDElement, "reticleSize", m_reticleSize);
		m_spriteSize = ParseXMLAttribute(*HUDElement, "spriteSize", m_spriteSize);
		m_spritePivot = ParseXMLAttribute(*HUDElement, "spritePivot", m_spritePivot);

		XmlElement const* AnimElement = HUDElement->FirstChildElement("Animation");
		if (AnimElement)
		{
			std::string shader;
			shader = ParseXMLAttribute(*AnimElement, "shader", shader);
			if (shader != "")
			{
				m_shaderAnim = g_theRenderer->CreateShader(shader.c_str());
			}
			std::string name;
			name = ParseXMLAttribute(*AnimElement, "name", name);
			m_animNames.push_back(name);
			std::string spriteSheet;
			spriteSheet = ParseXMLAttribute(*AnimElement, "spriteSheet", spriteSheet);
			IntVec2 cellCount;
			cellCount = ParseXMLAttribute(*AnimElement, "cellCount", cellCount);

			Texture* usingTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheet.c_str());
			m_spriteSheet = new SpriteSheet(*usingTexture, cellCount);

			int startIndex = 0, endIndex = 0;
			startIndex = ParseXMLAttribute(*AnimElement, "startFrame", startIndex);
			endIndex = ParseXMLAttribute(*AnimElement, "endFrame", endIndex);
			float secPerFrame = 0.f;
			secPerFrame = ParseXMLAttribute(*AnimElement, "secondsPerFrame", secPerFrame);
			SpriteAnimDefinition* newAnim = new SpriteAnimDefinition(*m_spriteSheet, startIndex, endIndex, 1.f / secPerFrame, SpriteAnimPlaybackType::ONCE);
			m_animsDef.push_back(newAnim);
			AnimElement = AnimElement->NextSiblingElement("Animation");
			if (AnimElement)
			{
				startIndex = ParseXMLAttribute(*AnimElement, "startFrame", startIndex);
				endIndex = ParseXMLAttribute(*AnimElement, "endFrame", endIndex);
				secPerFrame = ParseXMLAttribute(*AnimElement, "secondsPerFrame", secPerFrame);
				newAnim = new SpriteAnimDefinition(*m_spriteSheet, startIndex, endIndex, 1.f / secPerFrame, SpriteAnimPlaybackType::ONCE);
				m_animsDef.push_back(newAnim);
			}
		}
	}
	
	XmlElement const* soundsElement = element.FirstChildElement("Sounds");
	XmlElement const* soundElement = soundsElement->FirstChildElement("Sound");
	if (soundElement)
	{
		//---------------sound-----------------------------
		m_sound = ParseXMLAttribute(*soundElement, "sound", m_sound);
		m_soundName = ParseXMLAttribute(*soundElement, "name", m_soundName);
	    g_theAudio->CreateOrGetSound(m_soundName, true);
	}
	


	return true;
}

