#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <math.h>
#include <cassert>
#include <crtdbg.h>
#include "Engine/Core/Rgba8.hpp"

//#include <Engine/Renderer/Renderer.hpp>


//-----------------------------------------------------------------------------------------------
// #SD1ToDo: Remove all OpenGL references out of Main_Win32.cpp once you have a Renderer
// Both of the following lines should be relocated to the top of Engine/Renderer/Renderer.cpp
//
#include <Engine/Renderer/Renderer.hpp>
#include <gl/gl.h>					// Include basic OpenGL constants and function declarations
#include <Game/App.hpp>
//#include "..\renderer.hpp"
#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library

//-----------------------------------------------------------------------------------------------
// #SD1ToDo: Move this useful macro to a more central place, e.g. Engine/Core/EngineCommon.hpp
//
#define UNUSED(x) (void)(x);

App* g_theApp = nullptr;				// Created and owned by Main_Windows.cpp






int WINAPI WinMain(_In_ HINSTANCE, _In_ HINSTANCE, LPSTR commandLineString, _In_ int  )
{
	UNUSED( commandLineString );

	g_theApp = new App();
	g_theApp->Startup();
	g_theApp->Run();
	g_theApp->Shutdown();
	delete g_theApp;
	g_theApp = nullptr;

	return 0;
}


