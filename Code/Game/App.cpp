#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/App.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Player.hpp"
#include "Engine/Core/XmlUtils.hpp"

Renderer* g_theRenderer = nullptr;			
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window* g_theWindow = nullptr;
DevConsole* g_theDevConsole = nullptr;

constexpr float MAX_FRAME_SEC = 1.f / 10.f;


bool QuitAppCallbackFunction(EventArgs& args)
{
	UNUSED(args);
	m_isQuitting = true;
	return true;
}


App::App()
{
	BGM = 0;
	m_theGame = nullptr;

	m_screenCamera = new Camera();
	
}

App::~App()
{
	delete g_theDevConsole;
	delete m_theGame;
	delete g_theAudio;
	delete g_theRenderer;
	delete g_theWindow;
	delete g_theInput;
	delete m_screenCamera;

	g_theDevConsole = nullptr;
	m_theGame = nullptr;
	g_theAudio = nullptr;
	g_theWindow = nullptr;
	g_theRenderer = nullptr;
	g_theInput = nullptr;
	m_screenCamera = nullptr;
}

void App::Startup()
{
	XmlDocument document;
	document.LoadFile("Data/GameConfig.xml");
	XmlElement* rootElement = document.RootElement();
	float windowAspect = ParseXMLAttribute(*rootElement, "windowAspect", 0.0f);
	std::string mapName;

	m_defaultMap = ParseXMLAttribute(*rootElement, "defaultMap", m_defaultMap);
	m_musicVolume = ParseXMLAttribute(*rootElement, "musicVolume", m_musicVolume);
	m_mainMenuMusic = ParseXMLAttribute(*rootElement, "mainMenuMusic", m_mainMenuMusic);
	m_gameMusic = ParseXMLAttribute(*rootElement, "gameMusic", m_gameMusic);
	m_buttonClickSound = ParseXMLAttribute(*rootElement, "buttonClickSound", m_buttonClickSound);

	m_screenCamera->SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y));

	InputSystemConfig inputConfig;
	g_theInput = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_windowTitle = "Doomenstein";
	windowConfig.m_clientAspect = windowAspect;
	windowConfig.m_inputSystem = g_theInput;
	g_theWindow = new Window(windowConfig);

	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(rendererConfig);

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_camera = m_screenCamera;
	devConsoleConfig.m_renderer = g_theRenderer;

	g_theDevConsole = new DevConsole(devConsoleConfig);

	AudioSystemConfig audioConfig;
	g_theAudio = new AudioSystem(audioConfig);

	m_theGame = new Game(this);

	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theDevConsole->Startup();
	g_theInput->Startup();
	g_theAudio->Startup();
	m_theGame->Startup();
	SubscribeEventCallbackFunction("quit", QuitAppCallbackFunction);

	Vec2 min = Vec2(0.f, 0.f);
	Vec2 max = Vec2((float)g_theWindow->GetClientDimensions().x, (float)g_theWindow->GetClientDimensions().y);
	m_screenCamera->SetViewport(min, max);

	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Controls");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "F1    - Toggle Control Mode");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "LMB   - Raycast Long");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "RMB   - Raycast Short");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "AWSD  - Moving");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Q/E     - Rolling Left/Right");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Z/C     - Flying Vertically");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "H       - Set Back to the Origin");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Shift   - Speed Up(Hold)");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "New Weapon(3) Gravity Gun: It can pull mobs in an area.");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "New Demon: Biochemical Demon: He will spawn more small-sized biochemical Demon after death");

	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_renderer = g_theRenderer;
	debugRenderConfig.m_startHidden = false;
	DebugRenderSystemStartup(debugRenderConfig);
	FireEvent("debugrenderclear");
}


void App::Run()
{
	while (!m_isQuitting)			
	{
		Sleep(0); 
        Runframe();
	}
}

void App::Shutdown()
{	
	DebugRenderSystemShutdown();
	g_theInput->Shutdown();
	g_theWindow->Shutdown();
	g_theRenderer->Shutdown();
	g_theAudio->Shutdown();
	m_theGame->Shutdown();
}

void App::Runframe()
{	
	float deltaSeconds = Clock::GetSystemClock().GetDeltaSeconds();

	BeginFrame();
	Update(deltaSeconds);
	Render();
	EndFrame();
}

bool App::HandleQuitRequested()
{
	return false;
}

bool App::OpenSlowMo()
{
	g_theAudio->SetSoundPlaybackSpeed(BGM, 0.1f);

	return false;
}

bool App::CloseSlowMo()
{
	//if (!m_isPaused)
	{
		g_theAudio->SetSoundPlaybackSpeed(BGM, 1.f);
	}

	return false;
}

bool App::SwitchPaused()
{
	//if (m_isPaused)
	{
		g_theAudio->SetSoundPlaybackSpeed(BGM, 1.f);
	}
	//else
	{
		g_theAudio->SetSoundPlaybackSpeed(BGM, 0.f);
	}


	return false;
}

void App::MoveOneStepThenPaused()
{
	//if (m_isPaused) 
	{
		m_theGame->Update(0.f);
		m_theGame->Render();

	}
	//else
	{
		//if (m_isPaused)
		{
			g_theAudio->SetSoundPlaybackSpeed(BGM, 1.f);
		}
		//else
		{
			g_theAudio->SetSoundPlaybackSpeed(BGM, 0.f);
		}
	}
}

bool const App::IsKeyDown(unsigned char keyCode)
{
	return g_theInput->IsKeyDown(keyCode);
}

bool const App::WasKeyJustPressed(unsigned char keyCode)
{
	return g_theInput->WasKeyJustPressed(keyCode);
}

void App::ShowAttractMode()
{
	m_theGame->ShowAttractMode();
}

GameState App::GetIsAttractMo()
{
	return m_theGame->m_curentState;
}


SoundPlaybackID App::GetBGMPlaybackID() const
{
	return BGM;
}


void App::BeginFrame()
{
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theDevConsole->BeginFrame();
	DebugRenderBeginFrame();
	g_theInput->BeginFrame();
	g_theAudio->BeginFrame();

	if (g_theInput->WasKeyJustPressed(ESC_KEY) && !(m_theGame->m_curentState == GameState::LOBBY))
	{
		g_theAudio->StopSound(BGM);

		if (m_theGame->m_curentState == GameState::ATTRACT)
		{
			m_isQuitting = true;
		}
		else
		{
			delete m_theGame;
			m_theGame = nullptr;

			m_theGame = new Game(this);
			m_theGame->Startup();
		}
	}

	if (g_theInput->IsKeyDown('T'))
	{
		this->OpenSlowMo();
	}
	else
	{
		this->CloseSlowMo();
	}


	if (g_theInput->WasKeyJustPressed('O'))
	{
		this->MoveOneStepThenPaused();
	}

	if (g_theInput->WasKeyJustPressed('P'))
	{
		this->SwitchPaused();
	}

// 	if (g_theInput->WasKeyJustPressed(F8_KEY))
// 	{
// 		delete m_theGame;
// 		m_theGame = nullptr;
// 
// 		m_theGame = new Game(this);
// 		m_theGame->Startup();
// 	}

	if (g_theInput->WasKeyJustPressed(F1_KEY)) 
	{
		m_theGame->g_DebugMo = !m_theGame->g_DebugMo;
	}

	if (g_theInput->WasKeyJustPressed(187)) //+
	{
		SoundID testSound = g_theAudio->CreateOrGetSound("Data/Audio/TestSound.mp3");
		g_theAudio->StartSound(testSound);
		
	}
	if (m_theGame->m_curentState != m_theGame->m_nextState)
	{
		m_theGame->ExitState(m_theGame->m_curentState);
		m_theGame->EnterState(m_theGame->m_nextState);
	}

	XboxController const& controller = g_theInput->GetController(0);
	if (m_theGame->m_curentState == GameState::ATTRACT && (g_theInput->IsKeyDown(SPACE_KEY) || g_theInput->IsKeyDown('N') || controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START)))
	{
		m_theGame->m_nextState = GameState::LOBBY;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_theDevConsole->ToggleOpen();
	}

}

void App::Update(float deltaSeconds)
{
	Clock::TickSystemClock();
	if (m_theGame->m_curentState == GameState::ATTRACT || g_theDevConsole->IsOpen() || !g_theWindow->IsFocusingWindow())
	{
		g_theInput->SetCursorMode(true, false);
	}
	else
	{
		g_theInput->SetCursorMode(false, true);
	}

	m_theGame->Update(deltaSeconds);
	
}

void App::Render() const
{
	m_theGame->Render();
}

void App::EndFrame()
{
	CopyIsDownToWasDown();
	m_theGame->EndFrame();
	g_theAudio->EndFrame();
	DebugRenderEndFrame();
	g_theRenderer->EndFrame();

}

void App::UpdateShip(float deltaSeconds)
{
	deltaSeconds;
}



void App::CopyIsDownToWasDown()
{
	g_theInput->EndFrame();
}

