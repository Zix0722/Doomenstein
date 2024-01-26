#include "ActorDefinition.hpp"
#include "GameCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
std::vector<ActorDefinition*> ActorDefinition::s_actorDefinitions;
std::vector<ActorDefinition*> ActorDefinition::s_projectileActorDefinitions;

ActorDefinition* ActorDefinition::GetActorDefByName(std::string name)
{
	for (int defIndex = 0; defIndex < s_actorDefinitions.size(); ++defIndex)
	{
		if (s_actorDefinitions[defIndex]->m_name == name)
		{
			return s_actorDefinitions[defIndex];
		}
	}
	return nullptr;
}

ActorDefinition* ActorDefinition::GetProjectileActorDefByName(std::string name)
{
	for (int defIndex = 0; defIndex < s_projectileActorDefinitions.size(); ++defIndex)
	{
		if (s_projectileActorDefinitions[defIndex]->m_name == name)
		{
			return s_projectileActorDefinitions[defIndex];
		}
	}
	return nullptr;
}

AnimationGroup* ActorDefinition::GetAnimGroupByName(std::string name)
{
	for (int groupIndex = 0; groupIndex < m_animGroups.size(); ++groupIndex)
	{
		if (m_animGroups[groupIndex]->m_name == name)
		{
			return m_animGroups[groupIndex];
		}
	}
	return nullptr;
}

std::string ActorDefinition::GetSoundByName(std::string name)
{
	for (int soundIndex = 0; soundIndex < m_sounds.size(); soundIndex++)
	{
		if (m_sounds[soundIndex].m_sound == name)
		{
			return m_sounds[soundIndex].m_name;
		}
	}
	return "";
}

ActorDefinition::ActorDefinition()
	:m_fraction(Fraction::NEUTRAL)
{

}

ActorDefinition::~ActorDefinition()
{

}

bool ActorDefinition::LoadFromXmlElement(XmlElement const& element)
{
	m_name = ParseXMLAttribute(element, "name", m_name);
	std::string fractionStr;
	fractionStr = ParseXMLAttribute(element, "faction", fractionStr);
	if (fractionStr == "Demon")
	{
		m_fraction = Fraction::DEMON;
	}
	else if (fractionStr == "Marine")
	{
		m_fraction = Fraction::MARINE;
	}
	else
	{
		m_fraction = Fraction::NEUTRAL;
	}

	m_health = ParseXMLAttribute(element, "health", m_health);
	m_canBePossessed = ParseXMLAttribute(element, "canBePossessed", m_canBePossessed);
 	m_corpseLifetime = ParseXMLAttribute(element, "corpseLifetime", m_corpseLifetime);
	m_visible = ParseXMLAttribute(element, "visible", m_visible);
	m_dieOnSpawn = ParseXMLAttribute(element, "dieOnSpawn", m_dieOnSpawn);
	m_solidColor = ParseXMLAttribute(element, "solidColor", m_solidColor);
	m_wireframeColor = ParseXMLAttribute(element, "wireframeColor", m_wireframeColor);
	m_renderForward = ParseXMLAttribute(element, "renderForward", m_renderForward);
	//--------Collision---------------
	XmlElement const* collisionElement = element.FirstChildElement("Collision");
	if (collisionElement)
	{
		m_radius = ParseXMLAttribute(*collisionElement, "radius", m_radius);
		m_height = ParseXMLAttribute(*collisionElement, "height", m_height);
		m_collidesWithWorld = ParseXMLAttribute(*collisionElement, "collidesWithWorld", m_collidesWithWorld);
		m_collidesWithActors = ParseXMLAttribute(*collisionElement, "collidesWithActors", m_collidesWithActors);
		m_damageOnCollide = ParseXMlAttribute(*collisionElement, "damageOnCollide", m_damageOnCollide, '~');
		m_impulseOnCollide = ParseXMLAttribute(*collisionElement, "impulseOnCollide", m_impulseOnCollide);
		m_dieOnCollide = ParseXMLAttribute(*collisionElement, "dieOnCollide", m_dieOnCollide);
	}
	//--------physics---------------
	XmlElement const* physicsElement = element.FirstChildElement("Physics");
	if (physicsElement)
	{
		m_simulated = ParseXMLAttribute(*physicsElement, "simulated", m_simulated);
		m_flying = ParseXMLAttribute(*physicsElement, "flying", m_flying);
		m_walkSpeed = ParseXMLAttribute(*physicsElement, "walkSpeed", m_walkSpeed);
		m_runSpeed = ParseXMLAttribute(*physicsElement, "runSpeed", m_runSpeed);
		m_turnSpeed = ParseXMLAttribute(*physicsElement, "turnSpeed", m_turnSpeed);
		m_drag = ParseXMLAttribute(*physicsElement, "drag", m_drag);
	}
	//--------Camera---------------
	XmlElement const* cameraElement = element.FirstChildElement("Camera");
	if (cameraElement)
	{
		m_eyeHeight = ParseXMLAttribute(*cameraElement, "eyeHeight", m_eyeHeight);
		m_cameraFOV = ParseXMLAttribute(*cameraElement, "cameraFOV", m_cameraFOV);
	}
	//--------Weapons---------------
	XmlElement const* inventoryElement = element.FirstChildElement("Inventory");
	if (inventoryElement)
	{
		XmlElement const* weaponElement = inventoryElement->FirstChildElement();
		while (weaponElement != nullptr)
		{
			std::string weaponStr;
			weaponStr = ParseXMLAttribute(*weaponElement, "name", weaponStr);
			m_weapons.push_back(WeaponDefinition::GetWeaponDefinitionByName(weaponStr));
			weaponElement = weaponElement->NextSiblingElement();
		}
	}
	//--------AI---------------
	XmlElement const* aiElement = element.FirstChildElement("AI");
	if (aiElement)
	{
		m_aiEnabled = ParseXMLAttribute(*aiElement, "aiEnabled", m_aiEnabled);
		m_sightRadius = ParseXMLAttribute(*aiElement, "sightRadius", m_sightRadius);
		m_sightAngle = ParseXMLAttribute(*aiElement, "sightAngle", m_sightAngle);
	}
	//--------visual---------------
	XmlElement const* visualElement = element.FirstChildElement("Visuals");
	if (visualElement)
	{
		m_size = ParseXMLAttribute(*visualElement, "size", m_size);
		m_pivot = ParseXMLAttribute(*visualElement, "pivot", m_pivot);
		std::string billboardType;
		billboardType = ParseXMLAttribute(*visualElement, "billboardType", billboardType);
		if (billboardType == "WorldUpFacing")
		{
			m_billboardType = BillboardType::WORLD_UP_CAMERA_FACING;
		}
		else if(billboardType == "WorldUpOpposing")
		{
			m_billboardType = BillboardType::WORLD_UP_CAMERA_OPPOSING;
		}
		else if (billboardType == "FullOpposing")
		{
			m_billboardType = BillboardType::FULL_CAMERA_OPPOSING;
		}

		m_renderLit = ParseXMLAttribute(*visualElement, "renderLit", m_renderLit);
		m_renderRounded = ParseXMLAttribute(*visualElement, "renderRounded", m_renderRounded);
		std::string shader;
		shader = ParseXMLAttribute(*visualElement, "shader", shader).c_str();
		if (shader != "")
		{
			m_shader = g_theRenderer->CreateShader(shader.c_str());
		}
		std::string spriteSheet;
		m_cellCount = ParseXMLAttribute(*visualElement, "cellCount", m_cellCount);
		spriteSheet = ParseXMLAttribute(*visualElement, "spriteSheet", spriteSheet).c_str();
		Texture* usingTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheet.c_str());
		m_spriteSheet = new SpriteSheet(*usingTexture, m_cellCount);
		XmlElement const* animElement = visualElement->FirstChildElement("AnimationGroup");
		while (animElement != nullptr)
		{
			AnimationGroup* newAnimgroup = new AnimationGroup();
			newAnimgroup->LoadFromXmlElement(*animElement, *m_spriteSheet);
			m_animGroups.push_back(newAnimgroup);
			animElement = animElement->NextSiblingElement("AnimationGroup");
		}
	}
	XmlElement const* soundsElement = element.FirstChildElement("Sounds");
	if (soundsElement)
	{
		XmlElement const* soundElement = soundsElement->FirstChildElement("Sound");
		while (soundElement != nullptr)
		{
			Sound sound;
			sound.m_sound = ParseXMLAttribute(*soundElement, "sound", sound.m_sound);
			sound.m_name = ParseXMLAttribute(*soundElement, "name", sound.m_name);
			m_sounds.push_back(sound);
			g_theAudio->CreateOrGetSound(sound.m_name, true);
			soundElement = soundElement->NextSiblingElement("Sound");
		}
	}


	return true;
}

