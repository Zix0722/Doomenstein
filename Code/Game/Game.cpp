#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Prop.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "MapDefinition.hpp"
#include "TileDefinition.hpp"
#include "Tile.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Game/Player.hpp"
#include "WeaponDefinition.hpp"
#include "ActorDefinition.hpp"
#include "Engine/Renderer/BitmapFont.hpp"


RandomNumberGenerator* g_theRNG = nullptr;
extern Renderer* g_theRenderer;
extern DevConsole* g_theDevConsole;

Game::Game(App* owner)
	:m_app(owner)
{
	g_theRNG = new RandomNumberGenerator();
	m_gameClock = new Clock();
}

Game::~Game()
{
	delete m_gameClock;
	delete m_currentMap;
	m_currentMap = nullptr;
}

void Game::EnterState(GameState state)
{
	switch (state)
	{
	case GameState::NONE:
		break;
	case GameState::ATTRACT:
		m_curentState = GameState::ATTRACT;
		EnterAttract();
		break;
	case GameState::PLAYING:
		m_curentState = GameState::PLAYING;
		EnterPlaying();
		break;
	case GameState::LOBBY:
		m_curentState = GameState::LOBBY;
		EnterLobby();
		break;
	default:
		break;
	}
}

void Game::ExitState(GameState state)
{
	switch (state)
	{
	case GameState::NONE:
		break;
	case GameState::ATTRACT:
		ExitAttract();
		break;
	case GameState::PLAYING:
		ExitPlaying();
		break;
	case GameState::LOBBY:
		ExitLobby();
		break;
	default:
		break;
	}
}

void Game::Startup()
{
	m_gameFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	m_colorWatch = new Stopwatch(1.f);
	LoadingDefinitions();
	LoadingSounds();
	EnterState(m_curentState);
}

void Game::Update(float deltaSeconds)
{
	if (m_curentState == GameState::ATTRACT)
	{
		UpdateAttract(deltaSeconds);
		return;
	}
	else if(m_curentState == GameState::PLAYING)
	{
		UpdatePlaying(deltaSeconds);
	}
	else if (m_curentState == GameState::LOBBY)
	{
		UpdateLobby(deltaSeconds);
	}
}

void Game::Render() const
{
	switch (m_curentState)
	{
	case GameState::NONE:
		break;
	case GameState::ATTRACT:
		g_theRenderer->BeginCamera(*m_app->m_screenCamera);
		RenderAttract();
		g_theDevConsole->Render(AABB2(m_app->m_screenCamera->GetOrthoBottomLeft(), m_app->m_screenCamera->GetOrthoTopRight()));
		g_theRenderer->EndCamera(*m_app->m_screenCamera);
		break;
	case GameState::PLAYING:
		g_theRenderer->ClearScreen(Rgba8(100, 100, 100));
		g_theRenderer->BeginCamera(*m_currentMap->m_player->m_camera);
		RenderPlaying();
		if (m_hasPlayerTwo)
		{
			m_currentMap->m_players[1]->Render();
		}
		g_theRenderer->EndCamera(*m_currentMap->m_player->m_camera);

		if (m_hasPlayerTwo)
		{
			g_theRenderer->BeginCamera(*m_currentMap->m_player2->m_camera);
			RenderPlaying(false);
			m_currentMap->m_players[0]->Render(false);
			g_theRenderer->EndCamera(*m_currentMap->m_player2->m_camera);
		}

		DebugRenderWorld(*m_currentMap->m_player->m_camera);
		DebugRenderScreen(*m_app->m_screenCamera);

		m_currentMap->m_player->Render();
		if (m_hasPlayerTwo)
		{
			m_currentMap->m_player2->Render();
		}

		g_theRenderer->BeginCamera(*m_app->m_screenCamera);
		g_theDevConsole->Render(AABB2(m_app->m_screenCamera->GetOrthoBottomLeft(), m_app->m_screenCamera->GetOrthoTopRight()));
		g_theRenderer->EndCamera(*m_app->m_screenCamera);
		break;
	case GameState::LOBBY:
		g_theRenderer->BeginCamera(*m_app->m_screenCamera);
		RenderLobby();
		g_theDevConsole->Render(AABB2(m_app->m_screenCamera->GetOrthoBottomLeft(), m_app->m_screenCamera->GetOrthoTopRight()));
		g_theRenderer->EndCamera(*m_app->m_screenCamera);
		break;
	default:
		break;
	}

}


void Game::EndFrame()
{
	if (m_currentMap)
	{
		m_currentMap->DeleteDestroyActors();
	}
}

void Game::Shutdown()
{
	delete m_currentMap;
	m_currentMap = nullptr;
}


void Game::EnterAttract()
{
	SoundID bgm = g_theAudio->CreateOrGetSound(m_app->m_mainMenuMusic);
	m_lobbyBGM = g_theAudio->StartSound(bgm, true, m_app->m_musicVolume);
}

void Game::EnterLobby()
{

}

void Game::EnterPlaying()
{
	SoundID bgm = g_theAudio->CreateOrGetSound(m_app->m_gameMusic);
	m_gameBGM = g_theAudio->StartSound(bgm, true, m_app->m_musicVolume);
	if (m_hasPlayerTwo)
	{
		g_theAudio->SetNumListeners(2);
	}
	else
	{
		g_theAudio->SetNumListeners(1);
	}
	InitializeMap();
	m_currentMap = m_testMap;
}

void Game::ExitAttract()
{

}

void Game::ExitLobby()
{
	g_theAudio->StopSound(m_lobbyBGM);
}

void Game::ExitPlaying()
{
	delete m_currentMap;
	m_testMap = nullptr;
	m_currentMap = nullptr;
	g_theAudio->StopSound(m_gameBGM);
}

void Game::UpdateAttract(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	XboxController const& controller = g_theInput->GetController(0);
	if (g_theInput->WasKeyJustPressed(SPACE_KEY))
	{
		m_playerOneUsingController = false;
		SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
		m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
	}
	if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))
	{
		m_playerOneUsingController = true;
		SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
		m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
	}
}

void Game::UpdateLobby(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	XboxController const& controller = g_theInput->GetController(0);
	if (m_hasPlayerTwo)
	{
		if (m_playerOneUsingController)
		{
			if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))
			{
				m_nextState = GameState::PLAYING;
				SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
				m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
			}
			if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_BACK))
			{
				if (!m_hasPlayerTwo)
				{
					m_nextState = GameState::ATTRACT;
					SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
					m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
				}
				else
				{
					m_playerOneUsingController = false;
					m_hasPlayerTwo = false;
					SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
					m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
				}
			}
			if (g_theInput->WasKeyJustPressed(SPACE_KEY))
			{
				m_hasPlayerTwo = true;
				SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
				m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
			}
			if (g_theInput->WasKeyJustPressed(ESC_KEY))
			{
				m_hasPlayerTwo = false;
				SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
				m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
			}
		}
		else
		{
			if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))
			{
				m_hasPlayerTwo = true;
				SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
				m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
			}
			if (g_theInput->WasKeyJustPressed(ESC_KEY))
			{
				if (!m_hasPlayerTwo)
				{
					m_nextState = GameState::ATTRACT;
					SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
					m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
				}
				else
				{
					m_playerOneUsingController = true;
					m_hasPlayerTwo = false;
					SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
					m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
				}
			}
			if (g_theInput->WasKeyJustPressed(SPACE_KEY))
			{

				m_nextState = GameState::PLAYING;
				SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
				m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
			}
			if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_BACK))
			{
				m_hasPlayerTwo = false;
				SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
				m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
			}
		}
	}
	else
	{
		if (m_playerOneUsingController)
		{
			if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))
			{
				m_nextState = GameState::PLAYING;
				SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
				m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
			}
			if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_BACK))
			{
				if (!m_hasPlayerTwo)
				{
					m_nextState = GameState::ATTRACT;
					SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
					m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
				}
				else
				{
					m_playerOneUsingController = false;
					m_hasPlayerTwo = false;
					SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
					m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
				}
			}
			if (g_theInput->WasKeyJustPressed(SPACE_KEY))
			{
				m_hasPlayerTwo = true;
				SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
				m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
			}
		}
		else
		{
			if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))
			{
				m_hasPlayerTwo = true;
				SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
				m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
			}
			if (g_theInput->WasKeyJustPressed(ESC_KEY))
			{
				if (!m_hasPlayerTwo)
				{
					m_nextState = GameState::ATTRACT;
					SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
					m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
				}
				else
				{
					m_playerOneUsingController = true;
					m_hasPlayerTwo = false;
					SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
					m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
				}
			}
			if (g_theInput->WasKeyJustPressed(SPACE_KEY))
			{

				m_nextState = GameState::PLAYING;
				SoundID buttonSound = g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
				m_button = g_theAudio->StartSound(buttonSound, false, m_app->m_musicVolume);
			}
		}
	}
	
}

void Game::UpdatePlaying(float deltaSeconds)
{
	m_currentMap->m_player->Update(deltaSeconds);
	if (m_hasPlayerTwo)
	{
		m_currentMap->m_player2->Update(deltaSeconds);
	}
	m_currentMap->Update(deltaSeconds);
}

void Game::RenderAttract() const
{
	ShowAttractMode();
}

void Game::RenderLobby() const
{
	g_theRenderer->ClearScreen(Rgba8::BLACK);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);

	std::vector<Vertex_PCU> verts;
	AABB2 midBox, topBox, bottBox;
	midBox.m_mins = Vec2(400.f, 200.f);
	midBox.m_maxs = Vec2(1200.f, 600.f);
	topBox.m_mins = Vec2(400.f, 400.f);
	topBox.m_maxs = Vec2(1200.f, 800.f);
	bottBox.m_mins = Vec2(400.f, 0.f);
	bottBox.m_maxs = Vec2(1200.f, 400.f);
	if (!m_hasPlayerTwo)
	{
		std::string playerIndexStr = "Player 1\n";
		m_gameFont->AddVertsForTextInBox2D(verts, midBox, 50.f, playerIndexStr, Rgba8::WHITE, 1.25f, Vec2(0.5f, 0.7f));
		std::string controlStr;
		std::string optionStr;
		if (m_playerOneUsingController)
		{
			controlStr = "Controller\n";
			optionStr = "Press START to start game\nPress BACK to leave game\nPress SPACE to join player\n";
		}
		else
		{
			controlStr = "Keyboard & Mouse\n";
			optionStr = "Press SPACE to start game\nPress ESCAPE to leave game\nPress START to join player\n";
		}
		m_gameFont->AddVertsForTextInBox2D(verts, midBox, 25.f, controlStr, Rgba8::WHITE, 1.f, Vec2(0.5f, 0.5f));
		m_gameFont->AddVertsForTextInBox2D(verts, midBox, 15.f, optionStr, Rgba8::WHITE, 0.85f, Vec2(0.5f, 0.3f));
	}
	else
	{
		std::string playerIndexStr1 = "Player 1\n";
		m_gameFont->AddVertsForTextInBox2D(verts, topBox, 50.f, playerIndexStr1, Rgba8::WHITE, 1.25f, Vec2(0.5f, 0.7f));
		std::string controlStr1;
		std::string optionStr1;
		if (m_playerOneUsingController)
		{
			controlStr1 = "Controller\n";
			optionStr1 = "Press START to start game\nPress BACK to leave game\n";
		}
		else
		{
			controlStr1 = "Keyboard & Mouse\n";
			optionStr1 = "Press SPACE to start game\nPress ESCAPE to leave game\n";
		}
		m_gameFont->AddVertsForTextInBox2D(verts, topBox, 25.f, controlStr1, Rgba8::WHITE, 1.f, Vec2(0.5f, 0.5f));
		m_gameFont->AddVertsForTextInBox2D(verts, topBox, 15.f, optionStr1, Rgba8::WHITE, 0.85f, Vec2(0.5f, 0.3f));

		std::string playerIndexStr = "Player 2\n";
		m_gameFont->AddVertsForTextInBox2D(verts, bottBox, 50.f, playerIndexStr, Rgba8::WHITE, 1.25f, Vec2(0.5f, 0.7f));
		std::string controlStr;
		std::string optionStr;
		if (m_playerOneUsingController)
		{
			controlStr = "Keyboard & Mouse\n";
			optionStr = "Press SPACE to start game\nPress ESCAPE to leave game\n";
		}
		else
		{
			controlStr = "Controller\n";
			optionStr = "Press START to start game\nPress BACK to leave game\n";
		}
		m_gameFont->AddVertsForTextInBox2D(verts, bottBox, 25.f, controlStr, Rgba8::WHITE, 1.f, Vec2(0.5f, 0.5f));
		m_gameFont->AddVertsForTextInBox2D(verts, bottBox, 15.f, optionStr, Rgba8::WHITE, 0.85f, Vec2(0.5f, 0.3f));
	}

	g_theRenderer->BindTexture(&m_gameFont->GetTexture());
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
}

void Game::RenderPlaying(bool player1) const
{
	m_testMap->Render(player1);
}

void Game::UpdateWorldCamera(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void Game::UpdateScreenCamera(float deltaSeconds)
{
	UNUSED (deltaSeconds);
	m_app->m_screenCamera->SetOrthoView(Vec2(0.f, 0.f), Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y));
}




void Game::InitializeMap()
{
	MapDefinition* currenDef = MapDefinition::GetDefinitionByName(m_app->m_defaultMap);
	m_testMap = new Map(this, *currenDef);
}

void Game::LoadingDefinitions()
{
	XmlDocument weaponDocument;
	weaponDocument.LoadFile("Data/Definitions/WeaponDefinitions.xml");
	XmlElement* weaponRootElement = weaponDocument.RootElement();
	XmlElement* weaponDefElement = weaponRootElement->FirstChildElement();
	while (weaponDefElement != nullptr)
	{
		WeaponDefinition* weaponDef = new WeaponDefinition();
		weaponDef->LoadFromXmlElement(*weaponDefElement);
		WeaponDefinition::s_weaponDefinitions.push_back(weaponDef);
		weaponDefElement = weaponDefElement->NextSiblingElement();
	}

	XmlDocument actorsDocument;
	actorsDocument.LoadFile("Data/Definitions/ActorDefinitions.xml");
	XmlElement* actorsRootElement = actorsDocument.RootElement();
	XmlElement* actorsDefElement = actorsRootElement->FirstChildElement();
	while (actorsDefElement != nullptr)
	{
		ActorDefinition* actorDef = new ActorDefinition();
		actorDef->LoadFromXmlElement(*actorsDefElement);
		ActorDefinition::s_actorDefinitions.push_back(actorDef);
		actorsDefElement = actorsDefElement->NextSiblingElement();
	}

	XmlDocument projectileActorsDocument;
	projectileActorsDocument.LoadFile("Data/Definitions/ProjectileActorDefinitions.xml");
	XmlElement* projectileActorsRootElement = projectileActorsDocument.RootElement();
	XmlElement* projectileDefElement = projectileActorsRootElement->FirstChildElement();
	while (projectileDefElement != nullptr)
	{
		ActorDefinition* actorDef = new ActorDefinition();
		actorDef->LoadFromXmlElement(*projectileDefElement);
 		ActorDefinition::s_projectileActorDefinitions.push_back(actorDef);
		projectileDefElement = projectileDefElement->NextSiblingElement();
	}


	XmlDocument mapDocument;
	mapDocument.LoadFile("Data/Definitions/MapDefinitions.xml");
	XmlElement* rootElement = mapDocument.RootElement();
	XmlElement* mapDefElement = rootElement->FirstChildElement();
	while (mapDefElement != nullptr)
	{
		std::string elementName = mapDefElement->Name();
		MapDefinition* mapDef = new MapDefinition();
		mapDef->LoadFromXmlElement(*mapDefElement);
		MapDefinition::s_mapDefinitions.push_back(mapDef);
		XmlElement* spawnInfoElementRoot = mapDefElement->FirstChildElement("SpawnInfos");
		XmlElement* spawnInfoElement = spawnInfoElementRoot->FirstChildElement();
		while (spawnInfoElement != nullptr)
		{
			mapDef->LoadSpawnInfoFromElement(*spawnInfoElement);
			spawnInfoElement = spawnInfoElement->NextSiblingElement();
		}
		mapDefElement = mapDefElement->NextSiblingElement();
	}

	XmlDocument tileDocument;
	tileDocument.LoadFile("Data/Definitions/TileDefinitions.xml");
	XmlElement* tileRootElement = tileDocument.RootElement();
	XmlElement* tileDefElement = tileRootElement->FirstChildElement();
	while (tileDefElement != nullptr)
	{
		std::string elementName = tileDefElement->Name();
		TileDefinition* tileDef = new TileDefinition();
		tileDef->LoadFromXmlElement(*tileDefElement);
		TileDefinition::s_tileDefinitions.push_back(tileDef);
		tileDefElement = tileDefElement->NextSiblingElement();
	}

}

void Game::LoadingSounds()
{
	g_theAudio->CreateOrGetSound(m_app->m_gameMusic);
	g_theAudio->CreateOrGetSound(m_app->m_mainMenuMusic);
	g_theAudio->CreateOrGetSound(m_app->m_buttonClickSound);
}

void Game::ShowAttractMode() const
{
	g_theRenderer->ClearScreen(Rgba8::BLACK);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->BindTexture(nullptr);
 	Vec2 worldCenter(WORLD_CENTER_X, WORLD_CENTER_Y);
 	float changeValue = static_cast<float>(20.f * sin(2.f * GetCurrentTimeSeconds()));
	g_theRenderer->SetModelConstants();
 	DebugDrawRing(worldCenter * 4.0f, 40.f + changeValue * 2.f, 5.f + changeValue * 1.5f, Rgba8::YELLOW);

	std::vector<Vertex_PCU> verts;
	AABB2 box;
	box.m_mins = Vec2(400.f, 0.f);
	box.m_maxs = Vec2(1200.f, 400.f);
	std::string screenText = "Press Space to join with mouse and keyboard\nPress START to join with controller\nPress ESCAPE or BACK to exit";
	m_gameFont->AddVertsForTextInBox2D(verts, box, 20.f, screenText, Rgba8::GREEN, 0.85f, Vec2(0.5f, 0.2f));
	g_theRenderer->BindTexture(&m_gameFont->GetTexture());
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
}
















