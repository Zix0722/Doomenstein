#pragma once
#include "Actor.hpp"

class ProjectileActor : public Actor
{
public:
	ProjectileActor(Game* game, Map* map, ActorUID owner, ActorDefinition const& def);
	~ProjectileActor();
public:
	FloatRange m_damageOnCollide;
	float m_impulseOnCollide = 0.f;
	bool m_dieOnCollide = false;
};