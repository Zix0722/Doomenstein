#include "GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"

extern Renderer* g_theRenderer;

void	DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;
	float innerRadius = radius - halfThickness;
	float outerRadius = radius + halfThickness;

	constexpr int NUM_SIDES = 32;
	constexpr int NUM_TRIS = 2 * NUM_SIDES;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	Vertex_PCU vert[NUM_VERTS];

	constexpr float DEGREES_PER_SIDE = 360.f / (float)NUM_SIDES;

	for (int sideNum = 0; sideNum < NUM_SIDES; ++sideNum)
	{
		float startDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum);
		float endDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum+1);
		float cosStart = CosDegrees(startDegrees);
		float sinStart = SinDegrees(startDegrees);
		float cosEnd = CosDegrees(endDegrees);
		float sinEnd = SinDegrees(endDegrees);

		Vec3 innerStartPos = Vec3( center.x+innerRadius * cosStart, center.y+innerRadius * sinStart, 0.f);
		Vec3 outerStartPos = Vec3(center.x+outerRadius * cosStart, center.y + outerRadius * sinStart, 0.f);
		Vec3 outerEndPos = Vec3(center.x + outerRadius * cosEnd, center.y + outerRadius * sinEnd, 0.f);
		Vec3 innerEndPos = Vec3(center.x + innerRadius * cosEnd, center.y + innerRadius * sinEnd, 0.f);

		int vertIndexA = (6 * sideNum) + 0;
		int vertIndexB = (6 * sideNum) + 1;
		int vertIndexC = (6 * sideNum) + 2;
		int vertIndexD = (6 * sideNum) + 3;
		int vertIndexE = (6 * sideNum) + 4;
		int vertIndexF = (6 * sideNum) + 5;

		vert[vertIndexA].m_position = innerEndPos;
		vert[vertIndexB].m_position = innerStartPos;
		vert[vertIndexC].m_position = outerStartPos;
		vert[vertIndexA].m_color = color;
		vert[vertIndexB].m_color = color;
		vert[vertIndexC].m_color = color;

		vert[vertIndexD].m_position = innerEndPos;
		vert[vertIndexE].m_position = outerStartPos;
		vert[vertIndexF].m_position = outerEndPos;
		vert[vertIndexD].m_color = color;
		vert[vertIndexE].m_color = color;
		vert[vertIndexF].m_color = color;

	}
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(NUM_VERTS, vert);
}


void	DebugDrawLine(Vec2 const& startPos, Vec2 const& endPos, float thickness, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;

	Vec2 D = endPos - startPos;
	Vec2 F = D.GetNormalized();
	Vec2 FS = F * halfThickness;
	Vec2 LS = FS.GetRotated90Degrees();

	Vec2 EL = endPos + FS + LS;
	Vec2 ER = endPos + FS - LS;
	Vec2 SL = startPos - FS + LS;
	Vec2 SR = startPos - FS - LS;

	constexpr int NUM_VERTS = 3 * 2;
	Vertex_PCU vert[NUM_VERTS];

	vert[0].m_position = Vec3(SL.x,SL.y,0.f);
	vert[1].m_position = Vec3(SR.x, SR.y, 0.f);
	vert[2].m_position = Vec3(ER.x, ER.y, 0.f);

	vert[3].m_position = Vec3(ER.x, ER.y, 0.f);
	vert[4].m_position = Vec3(EL.x, EL.y, 0.f);
	vert[5].m_position = Vec3(SL.x, SL.y, 0.f);

	for (int vertIndex = 0; vertIndex < NUM_VERTS; ++vertIndex)
	{
		vert[vertIndex].m_color = color;
	}

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(NUM_VERTS, vert);
}