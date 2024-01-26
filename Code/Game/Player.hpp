#pragma once
#include "Game/Game.hpp"
#include "Controller.hpp"

enum class CameraMode
{
	FreeFlyMode,
	ControllingMode,

	Count,
};

class Player : public Controller
{
public:
	Player(Game* owner);
	virtual ~Player();

	void Update(float deltaSeconds);
	void Render();
public:
	Game* m_game = nullptr;
	Vec3 m_position;
	Vec3 m_velocity;
	EulerAngles m_orientation;
	EulerAngles m_angularVelocity;
	Rgba8 m_color = Rgba8::WHITE;

	Camera* m_camera = nullptr;
	Camera* m_HUDCamera = nullptr;
	float m_movementSpeed = 1.f;
	float m_turnRatePerSec = 90.f;
	CameraMode m_cameraMode = CameraMode::ControllingMode;
	Stopwatch* m_respawnTimer = nullptr;

	Texture* m_baseHUD = nullptr;
	std::string m_healthNum;
	int m_killNum = 0;
	int m_dealthTime = 0;

	int m_playerIndex = -1;

};