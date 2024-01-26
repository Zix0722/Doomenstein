#include "Weapon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec3.hpp"
#include "GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Controller.hpp"
#include "Game/Actor.hpp"
#include "Player.hpp"
#include "SpawnInfo.hpp"
#include "ProjectileActor.hpp"
#include "AIController.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Clock.hpp"

Weapon::Weapon(WeaponDefinition* def)
	:m_definition(def)
{
	m_refireWatch = new Stopwatch(def->m_refireTime);
	m_refireWatch->Start();
}


Weapon::~Weapon()
{
	delete m_refireWatch;
	m_refireWatch = nullptr;
}

void Weapon::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (!m_canFire)
	{
		if (m_refireWatch->HasDurationElapsed())
		{
			m_canFire = true;
			m_refireWatch->Start();
		}
	}
	if (m_animWatch)
	{
		if (m_animWatch->HasDurationElapsed())
		{
			if (m_definition->m_animsDef.size() != 0)
			{
				m_currentUV = m_definition->m_animsDef[0]->GetSpriteDefAtTime(0).GetUVs();
			}
		}
		else
		{
			float sec = Clock::GetSystemClock().GetTotalSeconds() - m_animWatch->m_startTime;
			m_currentUV = m_definition->m_animsDef[1]->GetSpriteDefAtTime(sec).GetUVs();
		}
	}
	else
	{
		if (m_definition->m_animsDef.size() != 0)
		{
			m_currentUV = m_definition->m_animsDef[0]->GetSpriteDefAtTime(0).GetUVs();
		}
	}


}

void Weapon::Fire(Actor* owner)
{
	if (m_canFire)
	{
		
		
		owner->m_currentUV = owner->GetUVsByAnim("Attack", true);
		Vec3 iBasis;
		Map* currentMap = nullptr;

		Vec3 pos;
		pos = owner->m_position;
		pos.z += owner->m_definition->m_eyeHeight;

		if (owner->m_controller)
		{
			currentMap = owner->m_controller->m_currentMap;
			Player* player = (Player*)owner->m_controller;
			iBasis = player->m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
		}
		else
		{
			currentMap = owner->m_AIController->m_currentMap;
		}
		if (m_definition->m_name == "Pistol")
		{
			m_canFire = false;
			m_refireWatch->Start();

			if (!m_animWatch)
			{
				m_animWatch = nullptr;
				float durationTime = m_definition->m_animsDef[1]->GetSecondsPerFrame() * m_definition->m_animsDef[1]->GetNumOfFrameHaveToPlay();
				m_animWatch = new Stopwatch(durationTime);
				m_animWatch->Start();
			}
			else
			{
				m_animWatch->Start();
			}
			
			float rayLength = m_definition->m_rayRange;
			for (int rayIndex = 0; rayIndex < m_definition->m_rayCount; rayIndex++)
			{
				Vec3 randDir = GetRandomDirectionInCone(m_definition->m_rayCone, iBasis.GetNormalized());
				RaycastResult3D result = currentMap->RaycastAll(pos, randDir, rayLength, owner);
				SoundID fireSound = g_theAudio->CreateOrGetSound(m_definition->m_soundName, true);
				g_theAudio->StartSoundAt(fireSound, pos, false, 0.1f);
				if (result.m_didImpact)
				{
// 					DebugAddWorldLine(result.m_rayStartPosition, result.m_impactPosition, 0.01f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
// 					DebugAddWorldPoint(result.m_impactPosition, 0.06f, 10.f);
// 					DebugAddWorldArrow(result.m_impactPosition, result.m_impactPosition + result.m_impactNormal * 0.3f, 0.03f, 10.f, Rgba8::BLUE, Rgba8::BLUE);
					if (result.m_touchedActor)
					{
						result.m_touchedActor->Damage(g_theRNG->RollRandomIntInRange((int)m_definition->m_rayDamage.m_min, (int)m_definition->m_rayDamage.m_max), owner);
						result.m_touchedActor->AddImpulse(m_definition->m_rayImpulse * randDir);
						SpawnInfo info = SpawnInfo();
						
						info.m_actor = "BloodSplatter";
						info.m_position = result.m_impactPosition;
						Actor* newProjectile = nullptr;
						newProjectile = currentMap->SpawnActor(&info, owner);
					}
					else
					{
						SpawnInfo info = SpawnInfo();
						info.m_actor = "BulletHit";
						info.m_position = result.m_impactPosition;
						Actor* newProjectile = nullptr;
						newProjectile = currentMap->SpawnActor(&info, owner);
					}
				}
				else
				{
					/*DebugAddWorldLine(result.m_rayStartPosition, result.m_rayStartPosition + result.m_rayDirection * rayLength, 0.01f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);*/
					
				}
			}
		}
		else if (m_definition->m_name == "PlasmaRifle")
		{
			m_canFire = false;
			m_refireWatch->Start();
			
			if (!m_animWatch)
			{
				m_animWatch = nullptr;
				float durationTime = m_definition->m_animsDef[1]->GetSecondsPerFrame() * m_definition->m_animsDef[1]->GetNumOfFrameHaveToPlay();
				m_animWatch = new Stopwatch(durationTime);
				m_animWatch->Start();
			}
			else
			{
				m_animWatch->Start();
			}

			SpawnInfo info = SpawnInfo();
			info.m_isProjectile = true;
			info.m_actor = m_definition->m_projectileActor;
			pos.z = pos.z - 0.15f * 0.5f;
			info.m_position = pos;
			SoundID fireSound = g_theAudio->CreateOrGetSound(m_definition->m_soundName, true);
			g_theAudio->StartSoundAt(fireSound, pos, false, 0.1f);
			Actor* newProjectile = nullptr;
			newProjectile = currentMap->SpawnActor(&info, owner);
			float cone = m_definition->m_projectileCone;
			Vec3 randDir = GetRandomDirectionInCone(cone, iBasis.GetNormalized());
			newProjectile->AddImpulse(randDir * 4.f);

		}
		else if (m_definition->m_name == "DemonMelee")
		{
			m_canFire = false;
			m_refireWatch->Start();
			Actor* cloestTarget = currentMap->m_actors[((AIController*)owner->m_AIController)->m_targetUID.GetIndex()];
			float meleeDist = m_definition->m_meleeRange;
			Vec3 disp = cloestTarget->m_position - owner->m_position;
			SoundID fireSound = g_theAudio->CreateOrGetSound(m_definition->m_soundName, true);
			g_theAudio->StartSoundAt(fireSound, owner->m_position, false, 0.1f);
			float dist = disp.GetLength() - cloestTarget->m_physics_radius - owner->m_physics_radius;
			float dispDegrees = disp.GetAngleAboutZDegrees();
			for (int meleeIndex = 0; meleeIndex < m_definition->m_meleeCount; ++meleeIndex)
			{
				if (dist < meleeDist && dispDegrees < m_definition->m_meleeArc)
				{
					cloestTarget->Damage(g_theRNG->RollRandomIntInRange((int)m_definition->m_meleeDamage.m_min, (int)m_definition->m_meleeDamage.m_max), owner);
					cloestTarget->AddImpulse(m_definition->m_meleeImpulse * disp.GetNormalized());
				}
			}
		}
		else if (m_definition->m_name == "GravityGun")
		{
			m_canFire = false;
			m_refireWatch->Start();
			
			if (!m_animWatch)
			{
				m_animWatch = nullptr;
				float durationTime = m_definition->m_animsDef[1]->GetSecondsPerFrame() * m_definition->m_animsDef[1]->GetNumOfFrameHaveToPlay();
				m_animWatch = new Stopwatch(durationTime);
				m_animWatch->Start();
			}
			else
			{
				m_animWatch->Start();
			}

			SpawnInfo info = SpawnInfo();
			info.m_isProjectile = true;
			info.m_actor = m_definition->m_projectileActor;
			pos.z = pos.z - 0.15f * 0.5f;
			info.m_position = pos;
			SoundID fireSound = g_theAudio->CreateOrGetSound(m_definition->m_soundName, true);
			g_theAudio->StartSoundAt(fireSound, pos, false, 0.1f);
			Actor* newProjectile = nullptr;
			newProjectile = currentMap->SpawnActor(&info, owner);
			float cone = m_definition->m_projectileCone;
			Vec3 randDir = GetRandomDirectionInCone(cone, iBasis.GetNormalized());
			newProjectile->AddImpulse(randDir * 4.f);
		}
	}
}

Vec3 Weapon::GetRandomDirectionInCone(float coneRange, Vec3 const& currentDirection)
{
	float halfRange = coneRange * 0.5f;
	float currentXYDegrees = Vec2(currentDirection).GetOrientationDegrees();
	float zComponent = currentDirection.z;
	zComponent = zComponent + g_theRNG->RollRandomFloatInRange(-halfRange * 0.03f, halfRange * 0.03f);
	currentXYDegrees = currentXYDegrees + g_theRNG->RollRandomFloatInRange(-halfRange, halfRange);
	Vec2 xyValue = Vec2::MakeFromPolarDegrees(currentXYDegrees);
	xyValue = xyValue.GetNormalized();
	Vec3 result;
	result.x = xyValue.x;
	result.y = xyValue.y;
	result.z = zComponent;
	return result.GetNormalized();
}

