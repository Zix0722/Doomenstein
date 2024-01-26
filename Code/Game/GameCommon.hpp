#pragma once
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"

struct Vec2;
extern InputSystem* g_theInput;
extern RandomNumberGenerator* g_theRNG;
extern AudioSystem* g_theAudio;
extern Renderer* g_theRenderer; 
extern Window* g_theWindow;

constexpr float WORLD_SIZE_X = 400.f;
constexpr float WORLD_SIZE_Y = 200.f;
constexpr float WORLD_CAMERA_SIZE_X = 200.f;
constexpr float WORLD_CAMERA_SIZE_Y = 100.f;
constexpr float SCREEN_CAMERA_SIZE_X = 1600.f;
constexpr float SCREEN_CAMERA_SIZE_Y = 800.f;


constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;
constexpr float SCRREN_CENTER_Y = SCREEN_CAMERA_SIZE_Y / 2.f;
constexpr float SCRREN_CENTER_X = SCREEN_CAMERA_SIZE_X / 2.f;

void	DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color);
void	DebugDrawLine(Vec2 const& startPos, Vec2 const& endPos, float thickness, Rgba8 const& color);

