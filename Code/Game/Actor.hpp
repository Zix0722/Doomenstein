#pragma once
#include "Game/Game.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "ActorUID.hpp"
#include "Weapon.hpp"
#include "ActorDefinition.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"

class Controller;

class Actor
{
public:
	Actor(Game* owner);
	Actor(Game* game, Map* map, ActorUID owner, ActorDefinition const& def);
	~Actor();

	void Update(float deltaSeconds);
	void Render(bool m_player1 = true) const;

	void UpdatePhysics(float deltaSeconds);
	void Damage(int dmg, Actor* attacker);

	void AddForce(Vec3 force);
	void AddImpulse(Vec3 impulse);

	void OnCollide();
	void OnCollide(Actor* otherActor);
	void OnPossessed();
	void OnUnPossessed();

	void MoveInDirection(Vec3 dir, float speed);
	void TurnInDirection(Vec3 dir, float maxDegrees);

	void Attack();
	void EquipWeapon(int weaponIndex);

	Mat44 GetModelMatrix() const;
	int GetCurrentWeaponIndexInInventory();
	AABB2 GetUVsByAnim(std::string animName, bool m_isNewShoot = false);
public:
	Game* m_game = nullptr;
	Map* m_map = nullptr;
	ActorUID m_owner = ActorUID::INVALID; // only used on projectile actors

	ActorUID m_UID = ActorUID::INVALID;
	ActorDefinition* m_definition = nullptr;
	Rgba8 m_color = Rgba8::WHITE;
	Rgba8 m_wireframeColor = Rgba8::WHITE;
	std::vector<Vertex_PCU> m_vertexes;

	Weapon* m_currentWeapon = nullptr;
	std::vector<Weapon*> m_weaponInventory;

	Vec3 m_position;
	Vec3 m_velocity;
	Vec3 m_acceleration;

	EulerAngles m_orientation;
	EulerAngles m_angularVelocity;

	bool m_isStatic = false;
	float m_physics_radius = 1.f;
	float m_physics_height = 1.f;

	FloatRange m_damageOnCollide;
	float m_impulseOnCollide = 0.f;
	bool m_dieOnCollide = false;

	bool m_isDead = false;
	Stopwatch* m_markDestroyWatch = nullptr;
	Stopwatch* m_animWatch = nullptr;
	Stopwatch* m_gravityDamageClock = nullptr;
	bool m_destroyMarked = false;

	int m_currentHealth = 0;
	//--------------------------------
	Controller* m_controller = nullptr;
	Controller* m_AIController = nullptr;

	//-------------------------------
	AABB2 m_currentUV;
	std::string m_currentAnim = "Walk";
	bool m_hasSpawnedBaby = false;
private:
	void UpdateWeapons(float deltaSeconds);
	bool isStartClock = false;

	void UpdateGravityGunFunction();
};