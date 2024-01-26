#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include <vector>


class App;
class Clock;
class Stopwatch;
class Map;
class Actor;

enum class GameState
{
	NONE,
	ATTRACT,
	LOBBY,
	PLAYING,
	COUNT
};

class Game
{
public:
	Game(App* owner);
	~Game();
	void EnterState(GameState state);
	void ExitState(GameState state);

	void Startup();
	void Update(float deltaSeconds);
	void Render() const;
	void EndFrame();
	void Shutdown();

	void EnterAttract();
	void EnterLobby();
	void EnterPlaying();

	void ExitAttract();
	void ExitLobby();
	void ExitPlaying();

	void UpdateAttract(float deltaSeconds);
	void UpdateLobby(float deltaSeconds);
	void UpdatePlaying(float deltaSeconds);

	void RenderAttract() const;
	void RenderLobby() const;
	void RenderPlaying(bool player1 = true) const;
public :
	bool g_DebugMo = false;
	bool m_isControllingProjectile = false;
	Actor* m_fakeProjectileActor = nullptr;
	Map* m_currentMap = nullptr;
	void ShowAttractMode() const;
	GameState m_curentState = GameState::ATTRACT;
	GameState m_nextState = GameState::ATTRACT;

	bool m_playerOneUsingController = false;
	bool m_hasPlayerTwo = false;
	BitmapFont* m_gameFont = nullptr;
	SoundPlaybackID m_lobbyBGM;
	SoundPlaybackID m_gameBGM;
	SoundPlaybackID m_button;
private:
	
	void UpdateWorldCamera(float deltaSeconds);
	void UpdateScreenCamera(float deltaSeconds);

	void InitializeMap();
	void LoadingDefinitions();
	void LoadingSounds();
private:
	App* m_app = nullptr;
	float m_backAttractCounter = 0.f;
	float m_CameraShakeLevel = 0.f;
	Clock* m_gameClock = nullptr;
	Stopwatch* m_colorWatch = nullptr;
	Map* m_testMap = nullptr;
};