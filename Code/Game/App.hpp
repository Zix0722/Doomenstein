#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

constexpr int MAX_NUM_KEYCODE = 256;
bool static m_isQuitting = false;

class App
{
	friend class Game;
public :
	App();
	~App();
	void Startup();
	void Run();
	void Shutdown();
	void Runframe();
		
	bool IsQuitting() const { return m_isQuitting;  }
	bool HandleQuitRequested();
	bool OpenSlowMo();
	bool CloseSlowMo();
	bool SwitchPaused();
	void MoveOneStepThenPaused();
	bool const IsKeyDown(unsigned char keyCode);
	bool const WasKeyJustPressed(unsigned char keyCode);
	void ShowAttractMode();
	GameState GetIsAttractMo();
	SoundPlaybackID GetBGMPlaybackID() const;

	Camera* m_screenCamera = nullptr;

private:
	void BeginFrame();
	void Update(float deltaSeconds);
	void Render() const;
	void EndFrame();

	void UpdateShip(float deltaSeconds);
	void CopyIsDownToWasDown();

private:
	Game* m_theGame;
	SoundPlaybackID BGM;
	std::vector<std::string> m_maps;
public:
	std::string m_defaultMap;
	float m_musicVolume = 0.1f;
	std::string m_mainMenuMusic;
	std::string m_gameMusic;
	std::string m_buttonClickSound;
};