#include "ProjectileActor.hpp"

ProjectileActor::ProjectileActor(Game* game, Map* map, ActorUID owner, ActorDefinition const& def)
	:Actor(game, map, owner, def)
{
	m_dieOnCollide = def.m_dieOnCollide;
	m_impulseOnCollide = def.m_impulseOnCollide;
	m_damageOnCollide = def.m_damageOnCollide;
}

ProjectileActor::~ProjectileActor()
{
	
}

