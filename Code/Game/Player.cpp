#include "Player.hpp"
#include "GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

constexpr float MOUSE_DELTA_SPEED = 0.075f;

Player::Player(Game* owner)
	:m_game(owner)
{
	m_camera = new Camera();
	m_HUDCamera = new Camera();
	m_camera->SetOrthographicView(Vec2(-1.f, -1.f), Vec2(1.f, 1.f));
	Vec2 min = Vec2::ZERO;
	Vec2 max = Vec2((float)g_theWindow->GetClientDimensions().x, (float)g_theWindow->GetClientDimensions().y);
	m_HUDCamera->SetOrthographicView(min, max);
	m_angularVelocity.m_pitchDegrees = m_turnRatePerSec;
	m_angularVelocity.m_yawDegrees = m_turnRatePerSec;
	m_angularVelocity.m_rollDegrees = m_turnRatePerSec;
	m_respawnTimer = new Stopwatch(2.f);

	
}

Player::~Player()
{
	delete m_camera;
	m_camera = nullptr;
}

void Player::Update(float deltaSeconds)
{
	
	m_HUDCamera->SetViewport(m_camera->m_viewport.m_mins, m_camera->m_viewport.m_maxs);
	if (m_playerIndex == -1)
	{
		g_theAudio->UpdateListener(0, m_position, m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D(), Vec3(0.f, 0.f, 1.f));
	}
	else
	{
		g_theAudio->UpdateListener(1, m_position, m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D(), Vec3(0.f, 0.f, 1.f));
	}

	if (GetActor() == nullptr)
	{
		if (m_respawnTimer->HasDurationElapsed())
		{
			m_currentMap->SpawnPlayer();
		}
		return;
	}
	m_baseHUD =  GetActor()->m_currentWeapon->m_definition->m_baseTexture;
	m_healthNum = Stringf("%d", GetActor()->m_currentHealth);
	if (GetActor()->m_isDead)
	{
		m_respawnTimer->Start();
		if (m_camera->m_position.z >= 0.f)
		{
			m_camera->m_position.z -= 0.01f;
		}

		return;
	}
	if (g_theInput->WasKeyJustPressed('V') && !m_game->m_hasPlayerTwo)
	{
		GetActor()->Damage(100, nullptr);
	}
	if (g_theInput->WasKeyJustPressed('N') && !m_game->m_hasPlayerTwo)
	{
		m_currentMap->DebugPossessNext();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFTARROW))
	{
		int currentWeaponIndex = GetActor()->GetCurrentWeaponIndexInInventory();
		int inventorySize = (int)GetActor()->m_weaponInventory.size();
		int priviousIndex = currentWeaponIndex - 1;
		if (priviousIndex < 0)
		{
			priviousIndex = inventorySize - 1;
		}
		GetActor()->EquipWeapon(priviousIndex);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHTARROW))
	{
		int currentWeaponIndex = GetActor()->GetCurrentWeaponIndexInInventory();
		int inventorySize = (int)GetActor()->m_weaponInventory.size();
		int nextIndex = currentWeaponIndex + 1;
		if (nextIndex > inventorySize - 1)
		{
			nextIndex = 0;
		}
		GetActor()->EquipWeapon(nextIndex);
	}
	if (g_theInput->WasKeyJustPressed('F'))
	{
		if (m_cameraMode == CameraMode::ControllingMode  && !m_game->m_hasPlayerTwo)
		{
			m_cameraMode = CameraMode::FreeFlyMode;
			GetActor()->m_controller = nullptr;
		}
		else
		{
			m_cameraMode = CameraMode::ControllingMode;
			GetActor()->m_controller = this;
			GetActor()->OnPossessed();
		}
	}

	if (g_theInput->WasKeyJustPressed('P'))
	{
		Clock::GetSystemClock().TogglePause();
	}
	
	XboxController const& controller = g_theInput->GetController(0);
	float movement = deltaSeconds * m_movementSpeed;
	Mat44 mat = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	mat.Orthonormalize_XFwd_YLeft_ZUp();
	Vec3 iBasis = mat.GetIBasis3D();
	Vec3 jBasis = mat.GetJBasis3D();
	Vec3 kBasis = mat.GetKBasis3D();
	Vec3 moveIntention;
	float speedFactor = 1.f;
	float actorWalkSpeed = GetActor()->m_definition->m_walkSpeed;
	float actorDrag = GetActor()->m_definition->m_drag;
	float actorRunSpeed = GetActor()->m_definition->m_runSpeed;
	bool isRunning = false;
	
	if (g_theInput->IsKeyDown(KEYCODE_LMB) && m_cameraMode == CameraMode::ControllingMode && m_playerIndex == -1)
	{
		GetActor()->Attack();
	}
	else if (controller.GetRightTrigger() > 0.1f && m_playerIndex == 0)
	{
		GetActor()->Attack();
	}
	
	if (g_theInput->IsKeyDown(KEYCODE_LEFTSHIFT) && m_playerIndex == -1 || controller.IsButtonDown(XBOX_BUTTON_A) && m_playerIndex == 0)
	{
		speedFactor = 15.f;
		isRunning = true;
	}
	else
	{
		isRunning = false;
	}
	if (g_theInput->IsKeyDown('W') && m_playerIndex == -1)
	{
		if (m_cameraMode == CameraMode::FreeFlyMode)
		{
			moveIntention += iBasis * movement * speedFactor;
		}
		else
		{
			if (isRunning)
			{
				GetActor()->AddForce(iBasis * actorDrag * actorRunSpeed);
			}
			else
			{
				GetActor()->AddForce(iBasis * actorDrag * actorWalkSpeed);
			}
		}
	}
	if (g_theInput->IsKeyDown('S') && m_playerIndex == -1)
	{
		if (m_cameraMode == CameraMode::FreeFlyMode)
		{
			moveIntention -= iBasis * movement * speedFactor;
		}
		else
		{
			if (isRunning)
			{
				GetActor()->AddForce(-iBasis * actorDrag * actorRunSpeed);
			}
			else
			{
				GetActor()->AddForce(-iBasis * actorDrag * actorWalkSpeed);
			}
		}
	}
	if (g_theInput->IsKeyDown('A') && m_playerIndex == -1)
	{
		if (m_cameraMode == CameraMode::FreeFlyMode)
		{
			moveIntention += jBasis * movement * speedFactor;
		}
		else
		{
			if (isRunning)
			{
				GetActor()->AddForce(jBasis* actorDrag * actorRunSpeed);
			}
			else
			{
				GetActor()->AddForce(jBasis * actorDrag * actorWalkSpeed);
			}
		}
	}
	if (g_theInput->IsKeyDown('D') && m_playerIndex == -1)
	{
		if (m_cameraMode == CameraMode::FreeFlyMode)
		{
			moveIntention -= jBasis * movement * speedFactor;
		}
		else
		{
			if (isRunning)
			{
				GetActor()->AddForce(-jBasis * actorDrag * actorRunSpeed);
			}
			else
			{
				GetActor()->AddForce(-jBasis* actorDrag * actorWalkSpeed);
			}
		}
	}
	switch (m_cameraMode)
	{
	case CameraMode::FreeFlyMode:
		m_camera->SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.f, 0.1f, 100.0f);
		break;
	case CameraMode::ControllingMode:
		float fov = GetActor()->m_definition->m_cameraFOV;
		m_camera->SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, fov, 0.1f, 100.0f);
		if (g_theInput->WasKeyJustPressed('1') || controller.WasButtonJustPressed(XBOX_BUTTON_X))
		{
			GetActor()->EquipWeapon(0);
		}
		if (g_theInput->WasKeyJustPressed('2') || controller.WasButtonJustPressed(XBOX_BUTTON_Y))
		{
			GetActor()->EquipWeapon(1);
		}
		if (g_theInput->WasKeyJustPressed('3') || controller.WasButtonJustPressed(XBOX_BUTTON_Y))
		{
			GetActor()->EquipWeapon(2);
		}
		break;
	}

// 	if (g_theInput->IsKeyDown('Q') || controller.GetLeftTrigger() > 0.2f)
// 	{
// 		m_orientation.m_rollDegrees += m_angularVelocity.m_rollDegrees * deltaSeconds * speedFactor;
// 	}
// 	if (g_theInput->IsKeyDown('E') || controller.GetRightTrigger() > 0.2f)
// 	{
// 		m_orientation.m_rollDegrees -= m_angularVelocity.m_rollDegrees * deltaSeconds * speedFactor;
// 	}
	if (g_theInput->IsKeyDown('Z') || controller.IsButtonDown(XBOX_BUTTON_L))
	{
		if (m_cameraMode == CameraMode::ControllingMode)
		{
			
		}
		else
		{
			m_position.z += movement * speedFactor;
		}
	}
	if (g_theInput->IsKeyDown('C') || controller.IsButtonDown(XBOX_BUTTON_R))
	{
		if (m_cameraMode == CameraMode::ControllingMode)
		{
			
		}
		else
		{
			m_position.z -= movement * speedFactor;
		}
	}
	
	float leftStickMag = controller.GetLeftStick().GetMagnitude();
	float rightStickMag = controller.GetRightStick().GetMagnitude();
	if (leftStickMag > 0.3f && m_playerIndex == 0)
	{
		float speedRate = RangeMap(leftStickMag, 0.3f, 1.f, 0.f, 1.f);
		Vec2  leftStickPos = Vec2::MakeFromPolarDegrees(controller.GetLeftStick().GetOrientationDegrees());
		if (m_cameraMode == CameraMode::FreeFlyMode)
		{
			moveIntention += iBasis * movement * speedFactor * leftStickPos.y * speedRate;
			moveIntention -= jBasis * movement * speedFactor * leftStickPos.x * speedRate;
		}
		else
		{
			if (isRunning)
			{
				Mat44 m = Mat44::CreateZRotationDegrees(leftStickPos.GetOrientationDegrees());
				mat.Append(m);
				GetActor()->AddForce(-mat.GetIBasis3D() * actorDrag * actorRunSpeed);
				GetActor()->AddForce(-mat.GetJBasis3D() * actorDrag * actorRunSpeed);
			}
			else
			{
				Mat44 m = Mat44::CreateZRotationDegrees(leftStickPos.GetOrientationDegrees());
				mat.Append(m);
				GetActor()->AddForce(-mat.GetIBasis3D() * actorDrag * actorWalkSpeed);
				GetActor()->AddForce(-mat.GetJBasis3D() * actorDrag * actorWalkSpeed);
			}
		}
	}

	if (rightStickMag > 0.3f && m_playerIndex == 0)
	{
		float speedRate = RangeMap(rightStickMag, 0.3f, 1.f, 0.f, 1.f);
		Vec2  RightStickPos = -Vec2::MakeFromPolarDegrees(controller.GetRightStick().GetOrientationDegrees());
		m_orientation.m_pitchDegrees += m_angularVelocity.m_pitchDegrees * deltaSeconds * speedFactor * RightStickPos.y * speedRate;
		m_orientation.m_yawDegrees += m_angularVelocity.m_yawDegrees * deltaSeconds * speedFactor * RightStickPos.x * speedRate;
	}
	// ----------------------------------------Adding Text on Screen Camera---------------------------------------------------------------------------
	std::string topLeftStr;
// 	if (m_cameraMode == CameraMode::FreeFlyMode)
// 	{
// 		topLeftStr = "[F1] Control Mode: Camera";
// 		DebugAddScreenText(topLeftStr, Vec2(0.f, 0.f), 20.f, Vec2(0.f, 0.99f), 0.f);
// 	}
// 	else
// 	{
// 		topLeftStr = "[F1] Control Mode: Actor";
// 		DebugAddScreenText(topLeftStr, Vec2(0.f, 0.f), 20.f, Vec2(0.f, 0.99f), 0.f, Rgba8::BLUE);
// 	}
	if (m_UID.IsValid())
	{
		
	}
	std::string topRightStr = "[GAME CLOCK] Time:  ";
	float totalSec = Clock::GetSystemClock().GetTotalSeconds();
	topRightStr.append(std::to_string(totalSec).substr(0, 4) + " ");
	topRightStr.append("FPS: ");
	float FPS;
	FPS = 1.f / Clock::GetSystemClock().GetDeltaSeconds();
	if (Clock::GetSystemClock().IsPaused())
	{
		topRightStr.append("inf ");
	}
	else
	{
		topRightStr.append(std::to_string(FPS).substr(0, 4) + " ");
	}
	topRightStr.append("Scale: ");
	float timeScale = Clock::GetSystemClock().GetTimeScale();
	topRightStr.append(std::to_string(timeScale).substr(0, 4));
	DebugAddScreenText(topRightStr, Vec2(0.f, 0.f), 13.f, Vec2(0.99f, 0.99f), 0.f);
	//-------------------------------------------------------------------------------------------------------------------------------------------------
	if (m_playerIndex == -1)
	{
		m_orientation.m_yawDegrees -= g_theInput->GetCursorClientDelta().x * MOUSE_DELTA_SPEED;
		m_orientation.m_pitchDegrees += g_theInput->GetCursorClientDelta().y * MOUSE_DELTA_SPEED;
	}

	m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);
	m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);

	if (m_cameraMode == CameraMode::FreeFlyMode)
	{
		m_position += moveIntention;
	}
	else
	{
		m_position = GetActor()->m_position;
		m_position.z = GetActor()->m_definition->m_eyeHeight;
		GetActor()->m_orientation.m_yawDegrees = m_orientation.m_yawDegrees;
	}
	m_camera->SetTransform(m_position, m_orientation);
}

void Player::Render()
{
	if (GetActor()->m_definition->m_name != "Marine")
	{
		return;
	}
	float viewportWidthForCam = m_camera->m_viewport.m_maxs.x - m_camera->m_viewport.m_mins.x;
	float viewportHeightForCam = m_camera->m_viewport.m_maxs.y - m_camera->m_viewport.m_mins.y;
	float aspect = viewportWidthForCam / viewportHeightForCam;
	float scaler = 2.f / aspect;
	Vec2 bottomLeft = m_HUDCamera->GetOrthoBottomLeft();
	Vec2 TopRight = m_HUDCamera->GetOrthoTopRight();
	TopRight.y /= scaler;
	float viewportHeight = TopRight.y - bottomLeft.y;
	float viewportWidth = TopRight.x - bottomLeft.x;

	

	g_theRenderer->BeginCamera(*m_HUDCamera);
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> text;
	std::vector<Vertex_PCU> gunVerts;
	std::vector<Vertex_PCU> reticle;
	std::vector<Vertex_PCU> dieScreen;
	
	AABB2 baseBox;
	baseBox.m_mins = Vec2::ZERO;
	baseBox.m_maxs = Vec2(TopRight.x, TopRight.y * 0.2f);
	AddVertsForAABB2(verts, baseBox, Rgba8::WHITE);

	AABB2 dieBox;
	dieBox.m_mins = bottomLeft;
	dieBox.m_maxs = TopRight;
	BitmapFont* font = m_game->m_gameFont;

	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->BindTexture(m_baseHUD);
	if (GetActor())
	{
		g_theRenderer->BindShader(GetActor()->m_currentWeapon->m_definition->m_UIshader);
	}
	else
	{
		g_theRenderer->BindShader(nullptr);
	}
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());

	if (GetActor())
	{
		if (GetActor()->m_currentHealth > 0.f)
		{
			Vec2 midOfViewport = Vec2(bottomLeft.x + viewportWidth * 0.5f, bottomLeft.y + viewportHeight * 0.5f);

			font->AddVertsForTextInBox2D(text, baseBox, 80.f, m_healthNum, Rgba8::WHITE, 0.5f, Vec2(0.29f, 0.6f));
			font->AddVertsForTextInBox2D(text, baseBox, 80.f, Stringf("%d", m_killNum), Rgba8::WHITE, 0.5f, Vec2(0.05f, 0.6f));
			font->AddVertsForTextInBox2D(text, baseBox, 80.f, Stringf("%d", m_dealthTime), Rgba8::WHITE, 0.5f, Vec2(0.95f, 0.6f));
			g_theRenderer->BindTexture(&font->GetTexture());
			g_theRenderer->DrawVertexArray((int)text.size(), text.data());


			AABB2 weaponBox;
			float offset = (float)GetActor()->m_currentWeapon->m_definition->m_spriteSize.x * 0.5f;
			weaponBox.m_mins = Vec2(midOfViewport.x - offset, TopRight.y * 0.2f);
			weaponBox.m_maxs = Vec2(midOfViewport.x + offset, TopRight.y * 0.2f +  offset * 2.f / scaler);
			AddVertsForAABB2(gunVerts, weaponBox, Rgba8::WHITE, GetActor()->m_currentWeapon->m_currentUV.m_mins, GetActor()->m_currentWeapon->m_currentUV.m_maxs);
			g_theRenderer->BindTexture(&(GetActor()->m_currentWeapon->m_definition->m_spriteSheet->GetTexture()));
			g_theRenderer->DrawVertexArray((int)gunVerts.size(), gunVerts.data());


			AABB2 reticleBox;
			reticleBox.m_mins = Vec2(midOfViewport.x - (float)(GetActor()->m_currentWeapon->m_definition->m_reticleSize.x),
				midOfViewport.y - (float)(GetActor()->m_currentWeapon->m_definition->m_reticleSize.y) / scaler);
			reticleBox.m_maxs = Vec2(midOfViewport.x + (float)(GetActor()->m_currentWeapon->m_definition->m_reticleSize.x),
				midOfViewport.y + (float)(GetActor()->m_currentWeapon->m_definition->m_reticleSize.y) / scaler);
			AddVertsForAABB2(reticle, reticleBox, Rgba8::WHITE);
			g_theRenderer->BindTexture(GetActor()->m_currentWeapon->m_definition->m_reticleTexture);
			g_theRenderer->DrawVertexArray((int)reticle.size(), reticle.data());
		}
		else
		{
			AddVertsForAABB2(dieScreen, dieBox, Rgba8(0, 0, 0, 100));
			g_theRenderer->SetBlendMode(BlendMode::ALPHA);
			g_theRenderer->BindShader(nullptr);
			g_theRenderer->BindTexture(nullptr);
			g_theRenderer->DrawVertexArray((int)dieScreen.size(), dieScreen.data());
		}
		
	}

	if (GetActor() == nullptr && !m_respawnTimer->HasDurationElapsed())
	{
		AddVertsForAABB2(dieScreen, dieBox, Rgba8(0, 0 ,0, 100));
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray((int)dieScreen.size(), dieScreen.data());
	}


	g_theRenderer->EndCamera(*m_HUDCamera);
}

