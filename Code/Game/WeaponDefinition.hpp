#pragma once
#include <vector>
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

class WeaponDefinition
{
public:
	WeaponDefinition();
	~WeaponDefinition();

	static WeaponDefinition* GetWeaponDefinitionByName(std::string name);
	bool LoadFromXmlElement(XmlElement const& element);
	static std::vector<WeaponDefinition*> s_weaponDefinitions;
public:
	std::string m_name;
	float m_refireTime = 0.f;
	int m_rayCount = 0;
	float m_rayCone = 0.f;
	float m_rayRange = 0.f;
	FloatRange m_rayDamage = FloatRange(0.f, 0.f);
	float m_rayImpulse = 0.f;
	int m_projectileCount = 0;
	float m_projectileCone = 0.f;
	float m_projectileSpeed = 0.f;
	std::string m_projectileActor;
	int m_meleeCount = 0;
	float m_meleeRange = 0.f;
	float m_meleeArc = 0.f;
	FloatRange m_meleeDamage = FloatRange(0.f, 0.f);
	float m_meleeImpulse = 0.f;
	//-------------HUD-----------------------------
	Shader* m_UIshader = nullptr;
	Texture* m_baseTexture = nullptr;
	Texture* m_reticleTexture = nullptr;
	IntVec2 m_reticleSize = IntVec2(1, 1);
	IntVec2 m_spriteSize = IntVec2(1, 1);
	Vec2    m_spritePivot = Vec2(0.5f, 0.f);
	//-----------------Animation----------------------
	Shader* m_shaderAnim = nullptr;
	SpriteSheet* m_spriteSheet = nullptr;
	std::vector<std::string> m_animNames;
	std::vector<SpriteAnimDefinition*> m_animsDef;
	//---------------sound-----------------------------
	std::string m_sound;
	std::string m_soundName;

};