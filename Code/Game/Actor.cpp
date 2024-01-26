#include "Actor.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Weapon.hpp"
#include "Game/Map.hpp"
#include "Player.hpp"
#include "AIController.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


Actor::Actor(Game* owner)
	:m_game(owner)
{
	
}

Actor::Actor(Game* game, Map* map, ActorUID owner, ActorDefinition const& def)
	:m_game(game)
	,m_map(map)
	,m_owner(owner)
	,m_physics_height(def.m_height)
	,m_physics_radius(def.m_radius)
{
	if (def.m_weapons.size() > 0)
	{
		for (int weaponIndex = 0; weaponIndex < def.m_weapons.size(); ++weaponIndex)
		{
			m_weaponInventory.push_back(new Weapon(WeaponDefinition::GetWeaponDefinitionByName(def.m_weapons[weaponIndex]->m_name)));
		}
		m_currentWeapon = m_weaponInventory[0];
	}
	if (def.m_aiEnabled)
	{
		m_AIController = new AIController();
		m_AIController->m_currentMap = m_map;
	}

	if (def.m_visible)
	{
		AddVertsForCylindarZ3D(m_vertexes, Vec2(0.f, 0.f),
			FloatRange(0.f, m_physics_height), m_physics_radius, 16);
		if (def.m_renderForward)
		{
			float eyeConeRadius = 0.1f;
			AddVertsForCone3D(m_vertexes, Vec3(m_physics_radius - 0.1f, 0.f, def.m_eyeHeight), Vec3(m_physics_radius + 0.15f, 0.f, def.m_eyeHeight)
				, eyeConeRadius, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 16);
		}
	
	}
	m_currentHealth = def.m_health;

	m_markDestroyWatch = new Stopwatch(def.m_corpseLifetime);
	m_damageOnCollide = def.m_damageOnCollide;
	m_impulseOnCollide = def.m_impulseOnCollide;
	m_dieOnCollide = def.m_dieOnCollide;

	if (def.m_name == "GravityProjectile")
	{
		m_gravityDamageClock = new Stopwatch(0.7f);
		m_gravityDamageClock->Start();
	}

}

Actor::~Actor()
{
	
	if (m_markDestroyWatch)
	{
		delete m_markDestroyWatch;
		m_markDestroyWatch = nullptr;
	}
	delete m_animWatch;

	for (int weaponIndex = 0; weaponIndex < m_weaponInventory.size(); ++weaponIndex)
	{
		if (m_weaponInventory[weaponIndex])
		{
			delete m_weaponInventory[weaponIndex];
			m_weaponInventory[weaponIndex] = nullptr;
		}
	}
	m_weaponInventory.clear();
}

void Actor::Update(float deltaSeconds)
{
	if (m_definition->m_dieOnSpawn && !m_isDead)
	{
		m_isDead = true;
		m_currentUV = GetUVsByAnim("Death", true);
	}
	if (m_controller)
	{

	}
	else
	{
		if (m_AIController)
		{
			((AIController*)m_AIController)->Update(deltaSeconds);
		}
	}
	if (!m_isDead)
	{
		UpdatePhysics(deltaSeconds);
		UpdateWeapons(deltaSeconds);
	}
	if (!m_definition->m_flying)
	{
		if (m_definition->m_name != "BulletHit" || m_definition->m_name != "BloodSplatter")
		{
			m_velocity.z = 0.f;
		}
	}
	if (m_isDead)
	{
		if (!isStartClock)
		{
			m_markDestroyWatch->Start();
			m_color = Rgba8(100, 100, 100, 100);
			isStartClock = true;
		}
		if (m_markDestroyWatch->HasDurationElapsed())
		{
			m_destroyMarked = true;
		}
	}
	if (m_definition->m_animGroups.size() != 0)
	{
		if (m_animWatch)
		{
			if (m_animWatch->HasDurationElapsed())
			{
				if (m_definition->m_name != "GravityProjectile")
				{
					delete m_animWatch;
					m_animWatch = nullptr;
					m_currentAnim = "Walk";
				}
			}
		}

		m_currentUV = GetUVsByAnim(m_currentAnim);
	}

	UpdateGravityGunFunction();
	
}

void Actor::Render(bool m_player1) const
{	
	std::vector<Vertex_PCUTBN> verts;
	std::vector<Vertex_PCU> vertsPCU;
	float sizeY = m_definition->m_size.x;
	float sizeZ = m_definition->m_size.y;
	float offsetX = m_definition->m_pivot.x * sizeY;
	float offsetY = m_definition->m_pivot.y * sizeZ;
	Vec3 bottLeft = Vec3(0.f, 0.f - offsetX, 0.f - offsetY);
	Vec3 bottRight = Vec3(0.f, sizeY - offsetX, 0.f - offsetY);
	Vec3 topLeft = Vec3(0.f, 0.f - offsetX, sizeZ - offsetY);
	Vec3 topRight = Vec3(0.f, sizeY - offsetX, sizeZ - offsetY);
	if (m_definition->m_renderRounded && m_definition->m_visible)
	{
		AddVertsForRoundedQuad3D(verts, topLeft, bottLeft, bottRight, topRight, Rgba8::WHITE, m_currentUV);
		
	}
	else if(m_definition->m_visible)
	{
		AddVertsForQuad3D(vertsPCU, bottLeft, bottRight, topRight, topLeft, Rgba8::WHITE, m_currentUV);
	}

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	if (m_definition->m_shader)
	{
		g_theRenderer->BindShader(m_definition->m_shader);
	}
	else
	{
		g_theRenderer->BindShader(nullptr);
	}
	if (m_definition->m_spriteSheet) 
	{
		g_theRenderer->BindTexture(&m_definition->m_spriteSheet->GetTexture());
	}
	else
	{
		g_theRenderer->BindTexture(nullptr);
	}
	Camera* cam;
	if (!m_player1)
	{
		cam = m_game->m_currentMap->m_player2->m_camera;
	}
	else
	{
		cam = m_game->m_currentMap->m_player->m_camera;
	}
	g_theRenderer->SetModelConstants(GetBillboardMatrix(m_definition->m_billboardType, cam->m_orientation.GetAsMatrix_XFwd_YLeft_ZUp(), m_position), Rgba8::WHITE);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
	g_theRenderer->DrawVertexArray((int)vertsPCU.size(), vertsPCU.data());
}

void Actor::UpdatePhysics(float deltaSeconds)
{
	if (!m_definition->m_simulated)
	{
		return;
	}
	if (m_isDead)
	{
		return;
	}
	if (!m_definition->m_flying)
	{
		m_velocity.z = 0.f;
	}

	float dragForce = -m_definition->m_drag * m_velocity.GetLength();
	m_acceleration += dragForce * m_velocity.GetNormalized();
	m_velocity += m_acceleration * deltaSeconds;
	float maxWalkSpeed = m_definition->m_walkSpeed;
	m_velocity.GetClamped(maxWalkSpeed);
	m_position += m_velocity * deltaSeconds;

	m_acceleration = Vec3(0.f, 0.f, 0.f);
}

void Actor::Damage(int dmg, Actor* attacker)
{
	m_currentHealth -= dmg;

	SoundID hurtSound = g_theAudio->CreateOrGetSound(m_definition->GetSoundByName("Hurt"), true);
	g_theAudio->StartSoundAt(hurtSound, m_position, false, 0.1f);

	if (m_AIController)
	{
		((AIController*)m_AIController)->DamagedBy(attacker->m_UID);
		m_currentUV = GetUVsByAnim("Hurt", true);
	}
	if (m_currentHealth <= 0.f)
	{
		m_isDead = true;
		SoundID deadSound = g_theAudio->CreateOrGetSound(m_definition->GetSoundByName("Death"), true);
		g_theAudio->StartSoundAt(deadSound, m_position, false, 0.1f);
		m_currentUV = GetUVsByAnim("Death", true);
	}
}

void Actor::AddForce(Vec3 force)
{
	m_acceleration += force;
}

void Actor::AddImpulse(Vec3 impulse)
{
	m_velocity += impulse;
}

void Actor::OnCollide(Actor* otherActor)
{
	if (m_owner != ActorUID::INVALID)
	{
		Actor* ownerActor = m_map->m_actors[m_owner.GetIndex()];
		if (otherActor)
		{
			if (otherActor->m_owner == m_owner)
			{
				return;
			}
			if (m_owner == otherActor->m_UID)
			{
				return;
			}
		}
		if (m_definition->m_dieOnCollide && !m_isDead)
		{
			m_isDead = true;
			m_currentUV = GetUVsByAnim("Death", true);
		}
		if (otherActor)
		{
			if (!otherActor->m_isDead )
			{
				otherActor->AddImpulse(m_definition->m_impulseOnCollide * m_velocity.GetNormalized());
				if (m_definition->m_name != "GravityProjectile")
				{
					otherActor->Damage(g_theRNG->RollRandomIntInRange((int)m_definition->m_damageOnCollide.m_min, (int)m_definition->m_damageOnCollide.m_max), ownerActor);
				}
				else
				{
					if (m_gravityDamageClock->HasDurationElapsed())
					{
						otherActor->Damage(g_theRNG->RollRandomIntInRange((int)m_definition->m_damageOnCollide.m_min, (int)m_definition->m_damageOnCollide.m_max), ownerActor);
						m_gravityDamageClock->Start();
					}
				}
			}
		}
	}
}

void Actor::OnCollide()
{
	if (m_definition->m_dieOnCollide)
	{
		if(!m_isDead)
		{
			m_isDead = true;
			m_currentUV = GetUVsByAnim("Death", true);
		}

	}
}

void Actor::OnPossessed()
{
	if (m_controller)
	{
		static_cast<Player*>(m_controller)->m_orientation = m_orientation;
	}
}

void Actor::OnUnPossessed()
{
	m_controller = nullptr;
}

void Actor::MoveInDirection(Vec3 dir, float speed)
{
	float forceMagnitude = speed * m_definition->m_drag;
	Vec3 force = dir.GetNormalized() * forceMagnitude;

	AddForce(force);
}


void Actor::TurnInDirection(Vec3 dir, float maxDegrees)
{
	Vec2 currentDir = Vec2::MakeFromPolarDegrees(m_orientation.m_yawDegrees);
	float goalAngleDegrees = dir.GetAngleAboutZDegrees();
	float turnDegrees = GetTurnedTowardDegrees(m_orientation.m_yawDegrees, goalAngleDegrees, maxDegrees);
	m_orientation.m_yawDegrees = turnDegrees;
}

void Actor::Attack()
{
	m_currentWeapon->Fire(this);
}

void Actor::EquipWeapon(int weaponIndex)
{
	if (weaponIndex >= m_weaponInventory.size())
	{
		return;
	}
	m_currentWeapon = m_weaponInventory[weaponIndex];
}

Mat44 Actor::GetModelMatrix() const
{
	Mat44 result = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	result.SetTranslation3D(m_position);
	return result;
}

int Actor::GetCurrentWeaponIndexInInventory()
{
	for (int weaponIndex = 0; weaponIndex < m_weaponInventory.size(); ++weaponIndex)
	{
		if (m_weaponInventory[weaponIndex] == m_currentWeapon)
		{
			return weaponIndex;
		}
	}
	return -1;
}


void Actor::UpdateWeapons(float deltaSeconds)
{
	for (int weaponIndex = 0; weaponIndex < m_weaponInventory.size(); ++weaponIndex)
	{
		m_weaponInventory[weaponIndex]->Update(deltaSeconds);
	}
}

void Actor::UpdateGravityGunFunction()
{
	if (m_definition->m_name == "GravityProjectile")
	{
		AddForce(Vec3(0.f, 0.f, -1.f));
	}
	if (m_isDead)
	{
		
		for (int actorIndex = 0; actorIndex < m_map->m_actors.size(); ++actorIndex)
		{
			Actor*& actor = m_map->m_actors[actorIndex];
			if (!actor)
			{
				continue;
			}
			if (actor->m_definition->m_name == "Demon" || actor->m_definition->m_name == "DemonBaby")
			{
				float dist = (m_position - actor->m_position).GetLength();
				if (dist > 3.f)
				{
					continue;
				}
				Vec3 dir = (m_position - actor->m_position).GetNormalized();
				actor->AddImpulse(dir);
			}
			else
			{
				continue;
			}
		}
	}
}

AABB2 Actor::GetUVsByAnim(std::string animName, bool m_isNewShoot)
{
	AnimationGroup* theGroup = m_definition->GetAnimGroupByName(animName);
	if (theGroup == nullptr)
	{
		return AABB2(Vec2::ZERO, Vec2::ZERO);
	}
	Vec3 camPos = m_game->m_currentMap->m_player->m_camera->m_position;
	Vec2 camPos2D = camPos.GetVec2();
	Vec2 pos2D = m_position.GetVec2();
	Vec2 viewDir = (pos2D - camPos2D).GetNormalized();
	Mat44 actorMat = GetModelMatrix();
	actorMat = actorMat.GetOrthonormalInverse();
	viewDir = actorMat.TransformVectorQuantity2D(viewDir);
	viewDir = viewDir.GetNormalized();

// 	if (theGroup->GetAnimDefByDirection(viewDir) == nullptr)
// 	{
// 		return AABB2(Vec2::ZERO, Vec2::ZERO);
// 	}

	if (m_animWatch)
	{
		if (!m_animWatch->HasDurationElapsed())
		{
			if (!m_isNewShoot)
			{
				float timeForOnce = Clock::GetSystemClock().GetTotalSeconds() - m_animWatch->m_startTime;
				SpriteAnimDefinition* anim = theGroup->GetAnimDefByDirection(viewDir);
				if (anim)
				{
					return anim->GetSpriteDefAtTime(timeForOnce).GetUVs();
				}
			}
			else
			{
				if (theGroup->m_name == "Attack")
				{
					delete m_animWatch;
					m_animWatch = nullptr;
					float secondsPerFrame = theGroup->GetAnimDefByDirection(viewDir)->GetSecondsPerFrame();
					float totalFrame = (float)theGroup->GetAnimDefByDirection(viewDir)->GetNumOfFrameHaveToPlay();
					float duration = secondsPerFrame * totalFrame;
					m_animWatch = new Stopwatch(duration);
					m_animWatch->Start();
					float timeForOnce = Clock::GetSystemClock().GetTotalSeconds() - m_animWatch->m_startTime;
					m_currentAnim = "Attack";
					return theGroup->GetAnimDefByDirection(viewDir)->GetSpriteDefAtTime(timeForOnce).GetUVs();

				}
				else if (theGroup->m_name == "Hurt")
				{
					delete m_animWatch;
					m_animWatch = nullptr;
					float secondsPerFrame = theGroup->GetAnimDefByDirection(viewDir)->GetSecondsPerFrame();
					float totalFrame = (float)theGroup->GetAnimDefByDirection(viewDir)->GetNumOfFrameHaveToPlay();
					float duration = secondsPerFrame * totalFrame;
					m_animWatch = new Stopwatch(duration);
					m_animWatch->Start();
					float timeForOnce = Clock::GetSystemClock().GetTotalSeconds() - m_animWatch->m_startTime;
					m_currentAnim = "Hurt";
					return theGroup->GetAnimDefByDirection(viewDir)->GetSpriteDefAtTime(timeForOnce).GetUVs();

				}
				else if (theGroup->m_name == "Death")
				{
					delete m_animWatch;
					m_animWatch = nullptr;
					float secondsPerFrame = theGroup->GetAnimDefByDirection(viewDir)->GetSecondsPerFrame();
					float totalFrame = (float)theGroup->GetAnimDefByDirection(viewDir)->GetNumOfFrameHaveToPlay();
					float duration = secondsPerFrame * totalFrame;
					m_animWatch = new Stopwatch(duration);
					m_animWatch->Start();
					float timeForOnce = Clock::GetSystemClock().GetTotalSeconds() - m_animWatch->m_startTime;
					m_currentAnim = "Death";
					return theGroup->GetAnimDefByDirection(viewDir)->GetSpriteDefAtTime(timeForOnce).GetUVs();

				}
			}
		}
		if (m_animWatch->HasDurationElapsed())
		{
			delete m_animWatch;
			m_animWatch = nullptr;
			m_currentAnim = "Walk";
		}

	}
	if (theGroup->m_name == "Walk")
	{
		float sec;
		if (m_velocity.GetLength() >= m_definition->m_walkSpeed)
		{
			sec = Clock::GetSystemClock().GetTotalSeconds();
		}
		else
		{
			sec = 0;
		}
		return theGroup->GetAnimDefByDirection(viewDir)->GetSpriteDefAtTime(sec).GetUVs();
	}
	else if (theGroup->m_name == "Attack")
	{
		float secondsPerFrame = theGroup->GetAnimDefByDirection(viewDir)->GetSecondsPerFrame();
		float totalFrame = (float)theGroup->GetAnimDefByDirection(viewDir)->GetNumOfFrameHaveToPlay();
		float duration = secondsPerFrame * totalFrame;
		m_animWatch = new Stopwatch(duration);
		m_animWatch->Start();
		float timeForOnce = Clock::GetSystemClock().GetTotalSeconds() - m_animWatch->m_startTime;
		m_currentAnim = "Attack";
		return theGroup->GetAnimDefByDirection(viewDir)->GetSpriteDefAtTime(timeForOnce).GetUVs();

	}
	else if (theGroup->m_name == "Hurt")
	{
		float secondsPerFrame = theGroup->GetAnimDefByDirection(viewDir)->GetSecondsPerFrame();
		float totalFrame = (float)theGroup->GetAnimDefByDirection(viewDir)->GetNumOfFrameHaveToPlay();
		float duration = secondsPerFrame * totalFrame;
		m_animWatch = new Stopwatch(duration);
		m_animWatch->Start();
		float timeForOnce = Clock::GetSystemClock().GetTotalSeconds() - m_animWatch->m_startTime;
		m_currentAnim = "Hurt";
		return theGroup->GetAnimDefByDirection(viewDir)->GetSpriteDefAtTime(timeForOnce).GetUVs();

	}
	else if (theGroup->m_name == "Death")
	{
		float secondsPerFrame = theGroup->GetAnimDefByDirection(viewDir)->GetSecondsPerFrame();
		float totalFrame = (float)theGroup->GetAnimDefByDirection(viewDir)->GetNumOfFrameHaveToPlay();
		float duration = secondsPerFrame * totalFrame;
		m_animWatch = new Stopwatch(duration);
		m_animWatch->Start();
		float timeForOnce = Clock::GetSystemClock().GetTotalSeconds() - m_animWatch->m_startTime;
		m_currentAnim = "Death";
		return theGroup->GetAnimDefByDirection(viewDir)->GetSpriteDefAtTime(timeForOnce).GetUVs();

	}

	return theGroup->GetAnimDefByDirection(viewDir)->GetSpriteDefAtTime(0.f).GetUVs();
}
