#pragma once
#include "WeaponDefinition.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Math/AABB2.hpp"

class Actor;

class Weapon
{
public:
	Weapon(WeaponDefinition* def);
	~Weapon();
	
	void Update(float deltaSeconds);
	void Fire(Actor* owner);
	Vec3 GetRandomDirectionInCone(float coneRange, Vec3 const& currentDirection);

public:
	WeaponDefinition*  m_definition = nullptr;
	Stopwatch* m_refireWatch = nullptr;
	bool m_canFire = true;
	Stopwatch* m_animWatch = nullptr;
	AABB2 m_currentUV;
};