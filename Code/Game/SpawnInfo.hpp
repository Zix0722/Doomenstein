#pragma once
#include <string>
#include "Engine/Math/Vec3.hpp"

class SpawnInfo 
{
public:
	SpawnInfo();
	~SpawnInfo();

public:
	bool m_isProjectile = false;
	std::string m_actor;
	Vec3 m_position; 
	Vec3 m_orientation; 
};