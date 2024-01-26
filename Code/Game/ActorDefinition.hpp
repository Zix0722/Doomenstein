#pragma once
#include <vector>
#include <string>
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "WeaponDefinition.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "AnimationGroup.hpp"


enum class Fraction
{
	NEUTRAL,
	MARINE,
	DEMON,

	COUNT
};

struct Sound
{
	std::string m_sound;
	std::string m_name;
};

class ActorDefinition
{
public:
	ActorDefinition();
	~ActorDefinition();

	bool LoadFromXmlElement(XmlElement const& element);
	static std::vector<ActorDefinition*> s_actorDefinitions;
	static std::vector<ActorDefinition*> s_projectileActorDefinitions;
	static ActorDefinition* GetActorDefByName(std::string name);
	static ActorDefinition* GetProjectileActorDefByName(std::string name);
	AnimationGroup* GetAnimGroupByName(std::string name);
	std::string GetSoundByName(std::string name);


public:
	std::string m_name;
	Fraction m_fraction = Fraction::NEUTRAL;
	int m_health = 1;
	bool m_canBePossessed = false;
	float m_corpseLifetime = 0.f;
	bool m_visible = false;
	Rgba8 m_solidColor = Rgba8(32,32,32);
	Rgba8 m_wireframeColor = Rgba8(192,192,192);
	bool m_renderForward = false;
	bool m_dirOnSpawn = false;


	//--------Collision---------------
	float m_radius = 0.f;
	float m_height = 0.f;
	bool m_collidesWithWorld = false;
	bool m_collidesWithActors = false;
	FloatRange m_damageOnCollide;
	float m_impulseOnCollide = 0.f;
	bool m_dieOnCollide = false;
	//--------physics---------------
	bool m_simulated = false;
	bool m_flying = false;
	float m_walkSpeed = 0.f;
	float m_runSpeed = 0.f;
	float m_turnSpeed = 0.f;
	float m_drag = 0.f;
	//--------Camera---------------
	float m_eyeHeight = 0.f;
	float m_cameraFOV = 60.f;
	//--------Weapons---------------
	std::vector<WeaponDefinition*> m_weapons;
	//--------AI---------------
	bool m_aiEnabled = false;
	float m_sightRadius = 0.f;
	float m_sightAngle = 0.f;

	//---------Visual-----------------
	Vec2 m_size = Vec2(1, 1);
	Vec2 m_pivot = Vec2(0.5f, 0.5f);
	BillboardType m_billboardType = BillboardType::NONE;
	bool m_renderLit = false;
	bool m_renderRounded = false;
	Shader* m_shader = nullptr;
	SpriteSheet* m_spriteSheet = nullptr;
	IntVec2 m_cellCount = IntVec2(1, 1);
	std::vector<AnimationGroup*> m_animGroups;
	bool m_dieOnSpawn = false;
	//--------------sound----------------------
	std::vector<Sound> m_sounds;

};