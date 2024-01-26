#include "Map.hpp"
#include "Tile.hpp"
#include "TileDefinition.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Game/Player.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "ProjectileActor.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "AIController.hpp"


Map::Map(Game* owner, MapDefinition const& def)
	:m_game(owner)
{	
	InitializeMapTiles(def);

	m_player = new Player(owner);
	if (m_game->m_hasPlayerTwo)
	{
		m_player2 = new Player(owner);
		m_player2->m_currentMap = this;
		m_player2->m_camera->SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.f, 0.1f, 100.0f);
		m_player2->m_camera->SetRenderBasis(Vec3(0.f, 0.f, 1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f));
	}
	m_player->m_currentMap = this;
	m_player->m_camera->SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.f, 0.1f, 100.0f);
	m_player->m_camera->SetRenderBasis(Vec3(0.f, 0.f, 1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f));
	m_player->m_position = Vec3(2.5f, 8.5f, 0.5f);
	SpawnActors(def);
	SpawnPlayer();
	
}

Map::~Map()
{
	delete m_player;
	m_player = nullptr;
	delete m_indexBuffer;
	m_indexBuffer = nullptr;
	delete m_vertexBuffer;
	m_vertexBuffer = nullptr;
}

void Map::Update(float deltaseconds)
{
	UpdateLightingData();
	for (int actorIndex = 0; actorIndex < m_actors.size(); ++actorIndex)
	{
		if (m_actors[actorIndex])
		{
			m_actors[actorIndex]->Update(deltaseconds);
		}
	}
	CollideActors(); 
	CollideActorWithMap();
}

void Map::Render(bool player1) const
{
	g_theRenderer->UpdateLightConstantsBuffer(m_sunDirection, m_sunIntensity, m_ambientIntensity);

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->BindShader(m_shader);
	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->DrawVertexArray(m_vertexBuffer, (int)m_indexes.size(), m_indexBuffer);

	RenderActors(player1);
}

IntVec2 Map::GetTileCoordinatesForTileIndex(int tileIndex)
{
	int tileX = tileIndex % m_dimensions.x;
	int tileY = tileIndex / m_dimensions.x;
	return IntVec2(tileX, tileY);
}

bool Map::IsPositionInBounds(Vec3 position, float const tolerance /*= 0.f*/) const
{
	if (position.z + tolerance > 1.f || position.z - tolerance < 0.f)
	{
		return false;
	}
	Vec3 firstTileMin = m_tiles[0]->m_bounds.m_mins;
	Tile* lastTile = GetTile(m_dimensions.x - 1, m_dimensions.y - 1);
	Vec3 lastTileMax = lastTile->m_bounds.m_maxs;
	if (position.x + tolerance > lastTileMax.x || position.x - tolerance < firstTileMin.x)
	{
		return false;
	}
	if (position.y + tolerance > lastTileMax.y || position.y - tolerance < firstTileMin.y)
	{
		return false;
	}
	return true;
}

bool Map::AreCoordsInBounds(int x, int y) const
{
	if (x > m_dimensions.x - 1 || y > m_dimensions.y - 1|| x < 0 || y < 0)
	{
		return false;
	}
	return true;
}

Tile* const Map::GetTile(int x, int y) const
{
	int tileIndex = y * m_dimensions.x + x;
	return m_tiles[tileIndex];
}

IntVec2 Map::GetTileCoordsForWorldPos(Vec3 const& worldPos) const
{
	int IntPosX = (int)worldPos.x;
	int IntPosY = (int)worldPos.y;
	IntVec2 currentPostionInt(IntVec2(IntPosX, IntPosY));

	return currentPostionInt;
}

int Map::GetTileIndexByTileCoords(IntVec2 const& tileCoords) const
{
	return tileCoords.x + m_dimensions.x * tileCoords.y;
}

void Map::CollideActors()
{
	for (int actorAIndex = 0; actorAIndex < m_actors.size(); ++actorAIndex)
	{
		for (int actorBIndex = actorAIndex; actorBIndex < m_actors.size(); ++actorBIndex)
		{
			if (!m_actors[actorAIndex] || !m_actors[actorBIndex])
			{
				continue;
			}
			if (m_actors[actorAIndex]->m_owner != ActorUID::INVALID && m_actors[actorBIndex]->m_owner != ActorUID::INVALID)
			{
				if (m_actors[actorAIndex]->m_owner == m_actors[actorBIndex]->m_owner)
				{
					continue;
				}
			}
			if (m_actors[actorAIndex]->m_definition->m_collidesWithActors && m_actors[actorBIndex]->m_definition->m_collidesWithActors)
			{
				CollideActors(m_actors[actorAIndex], m_actors[actorBIndex]);
			}
		}
	}
}

void Map::CollideActors(Actor* actorA, Actor* actorB)
{
	if (actorA->m_owner == actorB->m_UID || actorB->m_owner == actorA->m_UID || actorA->m_UID == actorB->m_UID)
	{
		return;
	}
	bool isOverlapingDisc = DoDiscsOverlap(Vec2(actorA->m_position), actorA->m_physics_radius, Vec2(actorB->m_position), actorB->m_physics_radius);
	if (!isOverlapingDisc)
	{
		return;
	}
	float actorABottom = actorA->m_position.z;
	float actorATop = actorA->m_position.z + actorA->m_physics_height;

	float actorBBottom = actorB->m_position.z;
	float actorBTop = actorB->m_position.z + actorB->m_physics_height;
	bool isOverlapingOnZ = false;
	if (actorABottom <= actorBTop && actorABottom >= actorBBottom)
	{
		isOverlapingOnZ = true;
	}
	if (actorATop <= actorBTop && actorATop >= actorBBottom)
	{
		isOverlapingOnZ = true;
	}

	if (actorBBottom <= actorATop && actorBBottom >= actorABottom)
	{
		isOverlapingOnZ = true;
	}
	if (actorBTop <= actorATop && actorBTop >= actorABottom)
	{
		isOverlapingOnZ = true;
	}

	if (isOverlapingOnZ && isOverlapingDisc)
	{
		if (actorA->m_isStatic && actorB->m_isStatic)
		{
			return;
		}
		if (!actorA->m_isStatic && actorB->m_isStatic)
		{
			Vec2 Pos2D = Vec2(actorA->m_position);
			PushDiscOutOfFixedDisc2D(Pos2D, actorA->m_physics_radius, Vec2(actorB->m_position), actorB->m_physics_radius);
			actorA->m_position.x = Pos2D.x;
			actorA->m_position.y = Pos2D.y;
			actorA->OnCollide(actorB);
			actorB->OnCollide(actorA);
			return;
		}
		if (actorA->m_isStatic && !actorB->m_isStatic)
		{
			Vec2 Pos2D = Vec2(actorB->m_position);
			PushDiscOutOfFixedDisc2D(Pos2D, actorB->m_physics_radius, Vec2(actorA->m_position), actorA->m_physics_radius);
			actorB->m_position.x = Pos2D.x;
			actorB->m_position.y = Pos2D.y;
			actorA->OnCollide(actorB);
			actorB->OnCollide(actorA);
			return;
		}
		if (!actorA->m_isStatic && !actorB->m_isStatic)
		{
			Vec2 Pos2DA = Vec2(actorA->m_position);
			Vec2 Pos2DB = Vec2(actorB->m_position);
			PushDiscsOutOfEachOther2D(Pos2DA, actorA->m_physics_radius, Pos2DB, actorB->m_physics_radius);
			actorA->m_position.x = Pos2DA.x;
			actorA->m_position.y = Pos2DA.y;
			actorB->m_position.x = Pos2DB.x;
			actorB->m_position.y = Pos2DB.y;
			actorA->OnCollide(actorB);
			actorB->OnCollide(actorA);
			return;
		}
	}
}

void Map::CollideActorWithMap()
{
	for (int actorIndex = 0; actorIndex < m_actors.size(); ++actorIndex)
	{
		if (!m_actors[actorIndex])
		{
			continue;
		}
		if (m_actors[actorIndex]->m_definition->m_collidesWithWorld)
		{
			CollideActorWithMap(m_actors[actorIndex]);
		}
	}
}

void Map::CollideActorWithMap(Actor* actor)
{
	// push out of xy
	Vec2 actorPos2D = Vec2(actor->m_position.x, actor->m_position.y);

	IntVec2 currentTileCroods = GetTileCoordsForWorldPos(actor->m_position);

	IntVec2 leftTileCoords(currentTileCroods.x - 1, currentTileCroods.y);
	IntVec2 rightTileCoords(currentTileCroods.x + 1, currentTileCroods.y);
	IntVec2 upTileCoords(currentTileCroods.x, currentTileCroods.y + 1);
	IntVec2 downTileCoords(currentTileCroods.x, currentTileCroods.y - 1);

	int currentLeftIndex = -1, currentRightIndex = -1, currentUpIndex = -1, currentDownIndex = -1;

	if (AreCoordsInBounds(leftTileCoords.x, leftTileCoords.y))
	{
		currentLeftIndex = GetTileIndexByTileCoords(leftTileCoords);
	}
	if (AreCoordsInBounds(rightTileCoords.x, rightTileCoords.y))
	{
		currentRightIndex = GetTileIndexByTileCoords(rightTileCoords);
	}
	if (AreCoordsInBounds(upTileCoords.x, upTileCoords.y))
	{
		currentUpIndex = GetTileIndexByTileCoords(upTileCoords);
	}
	if (AreCoordsInBounds(downTileCoords.x, downTileCoords.y))
	{
		currentDownIndex = GetTileIndexByTileCoords(downTileCoords);
	}


	IntVec2 topLeftTileCoords(currentTileCroods.x - 1, currentTileCroods.y + 1);
	IntVec2 topRightTileCoords(currentTileCroods.x + 1, currentTileCroods.y + 1);
	IntVec2 botttomLeftTileCoords(currentTileCroods.x - 1, currentTileCroods.y - 1);
	IntVec2 bottomRightTileCoords(currentTileCroods.x + 1, currentTileCroods.y - 1);

	int currentTopLeftIndex = -1, currentTopRightIndex = -1, currentBotttomLeftIndex = -1, currentBottomRightIndex = -1;

	if (AreCoordsInBounds(topLeftTileCoords.x, topLeftTileCoords.y))
	{
		currentTopLeftIndex = GetTileIndexByTileCoords(topLeftTileCoords);
	}
	if (AreCoordsInBounds(topRightTileCoords.x, topRightTileCoords.y))
	{
		currentTopRightIndex = GetTileIndexByTileCoords(topRightTileCoords);
	}
	if (AreCoordsInBounds(botttomLeftTileCoords.x, botttomLeftTileCoords.y))
	{
		currentBotttomLeftIndex = GetTileIndexByTileCoords(botttomLeftTileCoords);
	}
	if (AreCoordsInBounds(bottomRightTileCoords.x, bottomRightTileCoords.y))
	{
		currentBottomRightIndex = GetTileIndexByTileCoords(bottomRightTileCoords);
	}
	// left right forward backward
	if (currentLeftIndex != -1)
	{
		if (m_tiles[currentLeftIndex]->m_def->m_isSolid)
		{
			Tile*& tile = m_tiles[currentLeftIndex];
			AABB2 bounds = AABB2(Vec2(tile->m_bounds.m_mins.x, tile->m_bounds.m_mins.y),
								 Vec2(tile->m_bounds.m_maxs.x, tile->m_bounds.m_maxs.y));
			bool check = PushDiscOutOfFixedAABB2D(actorPos2D, actor->m_physics_radius, bounds);
			if (check)
			{
				actor->OnCollide();
			}
		}
	}
	if (currentRightIndex != -1)
	{
		if (m_tiles[currentRightIndex]->m_def->m_isSolid)
		{
			Tile*& tile = m_tiles[currentRightIndex];
			AABB2 bounds = AABB2(Vec2(tile->m_bounds.m_mins.x, tile->m_bounds.m_mins.y),
				Vec2(tile->m_bounds.m_maxs.x, tile->m_bounds.m_maxs.y));
			bool check = PushDiscOutOfFixedAABB2D(actorPos2D, actor->m_physics_radius, bounds);
			if (check)
			{
				actor->OnCollide();
			}
		}
	}
	if (currentUpIndex != -1)
	{
		if (m_tiles[currentUpIndex]->m_def->m_isSolid)
		{
			Tile*& tile = m_tiles[currentUpIndex];
			AABB2 bounds = AABB2(Vec2(tile->m_bounds.m_mins.x, tile->m_bounds.m_mins.y),
				Vec2(tile->m_bounds.m_maxs.x, tile->m_bounds.m_maxs.y));
			bool check = PushDiscOutOfFixedAABB2D(actorPos2D, actor->m_physics_radius, bounds);
			if (check)
			{
				actor->OnCollide();
			}
		}
	}
	if (currentDownIndex != -1)
	{
		if (m_tiles[currentDownIndex]->m_def->m_isSolid)
		{
			Tile*& tile = m_tiles[currentDownIndex];
			AABB2 bounds = AABB2(Vec2(tile->m_bounds.m_mins.x, tile->m_bounds.m_mins.y),
				Vec2(tile->m_bounds.m_maxs.x, tile->m_bounds.m_maxs.y));
			bool check = PushDiscOutOfFixedAABB2D(actorPos2D, actor->m_physics_radius, bounds);
			if (check)
			{
				actor->OnCollide();
			}
		}
	}
	// forward left& right  backard left&right

	if (currentTopLeftIndex != -1)
	{
		if (m_tiles[currentTopLeftIndex]->m_def->m_isSolid)
		{
			Tile*& tile = m_tiles[currentTopLeftIndex];
			AABB2 bounds = AABB2(Vec2(tile->m_bounds.m_mins.x, tile->m_bounds.m_mins.y),
				Vec2(tile->m_bounds.m_maxs.x, tile->m_bounds.m_maxs.y));
			bool check = PushDiscOutOfFixedAABB2D(actorPos2D, actor->m_physics_radius, bounds);
			if (check)
			{
				actor->OnCollide();
			}
		}
	}
	if (currentTopRightIndex != -1)
	{
		if (m_tiles[currentTopRightIndex]->m_def->m_isSolid)
		{
			Tile*& tile = m_tiles[currentTopRightIndex];
			AABB2 bounds = AABB2(Vec2(tile->m_bounds.m_mins.x, tile->m_bounds.m_mins.y),
				Vec2(tile->m_bounds.m_maxs.x, tile->m_bounds.m_maxs.y));
			bool check = PushDiscOutOfFixedAABB2D(actorPos2D, actor->m_physics_radius, bounds);
			if (check)
			{
				actor->OnCollide();
			}
		}
	}
	if (currentBotttomLeftIndex != -1)
	{
		if (m_tiles[currentBotttomLeftIndex]->m_def->m_isSolid)
		{
			Tile*& tile = m_tiles[currentBotttomLeftIndex];
			AABB2 bounds = AABB2(Vec2(tile->m_bounds.m_mins.x, tile->m_bounds.m_mins.y),
				Vec2(tile->m_bounds.m_maxs.x, tile->m_bounds.m_maxs.y));
			bool check = PushDiscOutOfFixedAABB2D(actorPos2D, actor->m_physics_radius, bounds);
			if (check)
			{
				actor->OnCollide();
			}
		}
	}
	if (currentBottomRightIndex != -1)
	{
		if (m_tiles[currentBottomRightIndex]->m_def->m_isSolid)
		{
			Tile*& tile = m_tiles[currentBottomRightIndex];
			AABB2 bounds = AABB2(Vec2(tile->m_bounds.m_mins.x, tile->m_bounds.m_mins.y),
				Vec2(tile->m_bounds.m_maxs.x, tile->m_bounds.m_maxs.y));
			bool check = PushDiscOutOfFixedAABB2D(actorPos2D, actor->m_physics_radius, bounds);
			if (check)
			{
				actor->OnCollide();
			}
		}
	}

	actor->m_position.x = actorPos2D.x;
	actor->m_position.y = actorPos2D.y;

	//push out of the ceiling & floor
	if (AreCoordsInBounds(currentTileCroods.x, currentTileCroods.y))
	{
		int currentTileIndex = GetTileIndexByTileCoords(currentTileCroods);
		if (!m_tiles[currentTileIndex]->m_def->m_isSolid)
		{
			float offdistCeiling = actor->m_position.z + actor->m_physics_height - 1.0f;
			float offdistFloor = 0.f - actor->m_position.z;
			if (offdistCeiling >= 0.f)
			{
				actor->m_position.z = 1.0f - actor->m_physics_height;
				actor->OnCollide();
			}
			if (offdistFloor >= 0.f)
			{
				actor->m_position.z = 0.f;
				actor->OnCollide();
			}

		}
	}
}

RaycastResult3D Map::RaycastAll(Vec3 const& start, Vec3 const& direction, float distance) const
{
	RaycastResult3D xyResult = RaycastWorldXY(start, direction, distance);
	RaycastResult3D zResult = RaycastWorldZ(start, direction, distance);
	RaycastResult3D actorResult = RaycastWorldActors(start, direction, distance);
	float cloestDist = 999999999.f;
	RaycastResult3D cloestOne = xyResult;

	if (actorResult.m_didImpact)
	{
		if (actorResult.m_impactDistance < cloestDist)
		{
			cloestDist = actorResult.m_impactDistance;
			cloestOne = actorResult;
		}
	}
	if (zResult.m_didImpact)
	{
		if (zResult.m_impactDistance < cloestDist)
		{
			cloestDist = zResult.m_impactDistance;
			cloestOne = zResult;
		}
	}
	if(xyResult.m_didImpact)
	{
		if (xyResult.m_impactDistance < cloestDist)
		{
			cloestDist = xyResult.m_impactDistance;
			cloestOne = xyResult;
		}
	}
	return cloestOne;

}

RaycastResult3D Map::RaycastAll(Vec3 const& start, Vec3 const& direction, float distance, Actor* owner) const
{
	RaycastResult3D xyResult = RaycastWorldXY(start, direction, distance);
	RaycastResult3D zResult = RaycastWorldZ(start, direction, distance);
	RaycastResult3D actorResult = RaycastWorldActors(start, direction, distance, owner);
	float cloestDist = 999999999.f;
	RaycastResult3D cloestOne = xyResult;

	if (actorResult.m_didImpact)
	{
		if (actorResult.m_impactDistance < cloestDist)
		{
			cloestDist = actorResult.m_impactDistance;
			cloestOne = actorResult;
		}
	}
	if (zResult.m_didImpact)
	{
		if (zResult.m_impactDistance < cloestDist)
		{
			cloestDist = zResult.m_impactDistance;
			cloestOne = zResult;
		}
	}
	if (xyResult.m_didImpact)
	{
		if (xyResult.m_impactDistance < cloestDist)
		{
			cloestDist = xyResult.m_impactDistance;
			cloestOne = xyResult;
		}
	}
	return cloestOne;
}

RaycastResult3D Map::RaycastWorldXY(Vec3 const& start, Vec3 const& direction, float distance) const
{
	Vec2 pos2D = Vec2(start.x, start.y);
	RaycastResult3D result;
	result.m_rayDirection = direction;
	result.m_rayLength = distance;
	result.m_rayStartPosition = start;
	if (!AreCoordsInBounds((int)pos2D.x, (int)pos2D.y))
	{
		result.m_didImpact = false;
		return result;
	}
	Tile* tile = GetTile((int)pos2D.x, (int)pos2D.y);
	if (tile->m_def->m_isSolid && start.z > 0.f && start.z < 1.f)
	{
		Vec3 resultDir = Vec3(-direction.x, -direction.y, -direction.z);
		result.m_impactNormal = resultDir;
		result.m_didImpact = true;
		result.m_impactDistance = 0.f;
		result.m_impactPosition = start;
		return result;
	}
	else
	{
		float fwdDistPerXCrossing = 1.f / abs(direction.x);
		int tileStepDirectionX;
		int tileX = (int)pos2D.x;
		int tileY = (int)pos2D.y;
		if (direction.x < 0)
		{
			tileStepDirectionX = -1;
		}
		else
		{
			tileStepDirectionX = 1;
		}
		float xAtFirstXCrossing = tileX + (tileStepDirectionX + 1) / 2.f;
		float xDistToFirstXCrossing = xAtFirstXCrossing - start.x;
		float fwdDistAtNextXCroosing = fabsf(xDistToFirstXCrossing) * fwdDistPerXCrossing;

		float fwdDistPerYCrossing = 1.f / abs(direction.y);
		int tileStepDirectionY;
		if (direction.y < 0)
		{
			tileStepDirectionY = -1;
		}
		else
		{
			tileStepDirectionY = 1;
		}
		float yAtFirstYCrossing = tileY + (tileStepDirectionY + 1) / 2.f;
		float yDistToFirstYCrossing = yAtFirstYCrossing - start.y;
		float fwdDistAtNextYCroosing = fabsf(yDistToFirstYCrossing) * fwdDistPerYCrossing;

		while (true)
		{
			if (fwdDistAtNextXCroosing < fwdDistAtNextYCroosing)
			{
				if (fwdDistAtNextXCroosing > distance)
				{
					result.m_didImpact = false;
					return result;
				}
				tileX += tileStepDirectionX;
				if (!AreCoordsInBounds(tileX, tileY))
				{
					result.m_didImpact = false;
					return result;
				}
				if (GetTile(tileX, tileY)->m_def->m_isSolid)
				{
					result.m_didImpact = true;
					if (tileStepDirectionX > 0.f)
					{
						result.m_impactNormal = Vec3(-1.f, 0.f, 0.f);

					}
					else
					{
						result.m_impactNormal = Vec3(1.f, 0.f, 0.f);
					}
					result.m_impactDistance = fwdDistAtNextXCroosing;
					result.m_impactPosition = start + (direction * fwdDistAtNextXCroosing);
					if (result.m_impactPosition.z > 0.f && result.m_impactPosition.z < 1.f)
					{
						return result;
					}
					else
					{
						fwdDistAtNextXCroosing += fwdDistPerXCrossing;
					}
				}
				else
				{
					fwdDistAtNextXCroosing += fwdDistPerXCrossing;
				}
			}
			else
			{
				if (fwdDistAtNextYCroosing > distance)
				{
					result.m_didImpact = false;
					return result;
				}
				tileY += tileStepDirectionY;
				if (!AreCoordsInBounds(tileX, tileY))
				{
					result.m_didImpact = false;
					return result;
				}
				if (GetTile(tileX, tileY)->m_def->m_isSolid)
				{
					result.m_didImpact = true;
					if (tileStepDirectionY > 0.f)
					{
						result.m_impactNormal = Vec3(0.f, -1.f, 0.f);

					}
					else
					{
						result.m_impactNormal = Vec3(0.f, 1.f, 0.f);
					}
					result.m_impactDistance = fwdDistAtNextYCroosing;
					result.m_impactPosition = start + (direction * fwdDistAtNextYCroosing);
					if (result.m_impactPosition.z > 0.f && result.m_impactPosition.z < 1.f)
					{
						return result;
					}
					else
					{
						fwdDistAtNextYCroosing += fwdDistPerYCrossing;
					}
				}
				else
				{
					fwdDistAtNextYCroosing += fwdDistPerYCrossing;
				}
			}
		}
		
	}
	return result;
}

RaycastResult3D Map::RaycastWorldZ(Vec3 const& start, Vec3 const& direction, float distance) const
{
	RaycastResult3D result;
	result.m_rayStartPosition = start;
	result.m_rayDirection = direction;
	result.m_impactDistance = distance;
	

	float zComponent = direction.z;
	if (zComponent > 0.f)
	{
		float factorT = (1.f - start.z) / zComponent;
		if (factorT >= 0.f && factorT <= distance)
		{
			result.m_didImpact = true;
			result.m_impactPosition = start + direction * factorT;
			if (IsPositionInBounds(result.m_impactPosition, 0.0f))
			{
				result.m_impactDistance = factorT;
				result.m_impactNormal = Vec3(0.f, 0.f, -1.f);
				return result;
			}
			result.m_didImpact = false;
			return result;
		}

	}
	else if(zComponent < 0.f)
	{
		float factorT = (0.f - start.z) / zComponent;
		if (factorT >= 0.f && factorT <= distance)
		{
			result.m_didImpact = true;
			result.m_impactPosition = start + direction * factorT;
			if (IsPositionInBounds(result.m_impactPosition, 0.0f))
			{
				result.m_impactDistance = factorT;
				result.m_impactNormal = Vec3(0.f, 0.f, 1.f);
				return result;
			}
			result.m_didImpact = false;
			return result;
		}
	}
	result.m_didImpact = false;
	return result;
}

RaycastResult3D Map::RaycastWorldActors(Vec3 const& start, Vec3 const& direction, float distance) const
{
	RaycastResult3D result;
	result.m_didImpact = false;
	result.m_rayStartPosition = start;
	result.m_rayDirection = direction;
	result.m_rayLength = distance;

	float cloestDist = 99999999.f;
	for (int actorIndex = 0; actorIndex < m_actors.size(); ++actorIndex)
	{
		Vec3 pos = m_actors[actorIndex]->m_position;
		float height = m_actors[actorIndex]->m_physics_height;
		float r = m_actors[actorIndex]->m_physics_radius;
		RaycastResult3D actorResult = RaycastVsCylinderZ3D(start, direction, distance, Vec2(pos), pos.z, pos.z + height, r);
		if (actorResult.m_didImpact)
		{
			if (actorResult.m_impactDistance < cloestDist)
			{
				cloestDist = actorResult.m_impactDistance;
				result = actorResult;
			}
		}
	}
	return result;
}

RaycastResult3D Map::RaycastWorldActors(Vec3 const& start, Vec3 const& direction, float distance, Actor* owner) const
{
	
	RaycastResult3D result;
	result.m_didImpact = false;
	result.m_rayStartPosition = start;
	result.m_rayDirection = direction;
	result.m_rayLength = distance;

	float cloestDist = 99999999.f;
	for (int actorIndex = 0; actorIndex < m_actors.size(); ++actorIndex)
	{
		if (!m_actors[actorIndex])
		{
			continue;
		}
		Vec3 pos = m_actors[actorIndex]->m_position;
		float height = m_actors[actorIndex]->m_physics_height;
		float r = m_actors[actorIndex]->m_physics_radius;
		RaycastResult3D actorResult = RaycastVsCylinderZ3D(start, direction, distance, Vec2(pos), pos.z, pos.z + height, r);
		if (actorResult.m_didImpact)
		{
			if (actorResult.m_impactDistance < cloestDist)
			{
				if (m_actors[actorIndex]->m_UID == owner->m_UID)
				{

				}
				else if(!m_actors[actorIndex]->m_isDead)
				{
					cloestDist = actorResult.m_impactDistance;
					result = actorResult;
					result.m_touchedActor = m_actors[actorIndex];
				}
			}
		}
	}
	return result;
}

void Map::DeleteDestroyActors()
{

	for (int actorIndex = 0; actorIndex < m_actors.size(); ++actorIndex)
	{
		if (m_actors[actorIndex] != nullptr)
		{
			if (m_actors[actorIndex]->m_destroyMarked)
			{
				Actor*& deleteTarget = m_actors[actorIndex];
				delete deleteTarget;
				deleteTarget = nullptr;
			}
		}
	}

}

void Map::SpawnPlayer()
{
	std::vector<Vec3> spawnPoints;
	for (int actorIndex = 0; actorIndex < m_actors.size(); ++actorIndex)
	{
		if (m_actors[actorIndex])
		{
			if (m_actors[actorIndex]->m_definition->m_name == "SpawnPoint")
			{
				spawnPoints.push_back(m_actors[actorIndex]->m_position);
			}
		}
	}

	Vec3 spawnPosition = spawnPoints[g_theRNG->RollRandomIntInRange(0, 1)];

	ActorDefinition* marineActorDef = ActorDefinition::GetActorDefByName("Marine");
	Actor* newActor = new Actor(m_game, this, ActorUID::INVALID, *marineActorDef);
	bool findSlot = false;
	int index = -1;
	int listSize = (int)m_actors.size();
	for (int actorsIndex = 0; actorsIndex < listSize; ++actorsIndex)
	{
		if (m_actors[actorsIndex] == nullptr)
		{
			m_actors[actorsIndex] = newActor;
			index = actorsIndex;
			findSlot = true;
		}
	}
	if (!findSlot)
	{
		m_actors.push_back(newActor);
		index = listSize;
	}
	m_players.push_back(newActor);
	newActor->m_definition = marineActorDef;
	newActor->m_color = marineActorDef->m_solidColor;
	newActor->m_wireframeColor = marineActorDef->m_wireframeColor;
	newActor->m_isStatic = false;
	newActor->m_position = spawnPosition;
	m_player->m_orientation.m_yawDegrees = -90.f;
	newActor->m_UID = ActorUID(index, actorSalt);
	actorSalt++;

	newActor->m_controller = m_player;
	m_player->m_UID = newActor->m_UID;
	if (m_game->m_playerOneUsingController)
	{
		m_player->m_playerIndex = 0;
	}
	else
	{
		m_player->m_playerIndex = -1;
	}

	if (m_game->m_hasPlayerTwo)
	{
		float maxX = (float)g_theWindow->GetClientDimensions().x;
		float maxY = (float)g_theWindow->GetClientDimensions().y * 0.5f;
		m_player->m_camera->SetViewport(Vec2::ZERO, Vec2(maxX, maxY));
		m_player2->m_camera->SetViewport(Vec2(0.f, maxY), Vec2(maxX, (float)g_theWindow->GetClientDimensions().y));


		spawnPosition = spawnPoints[g_theRNG->RollRandomIntInRange(0, 1)];

		marineActorDef = ActorDefinition::GetActorDefByName("Marine");
		newActor = new Actor(m_game, this, ActorUID::INVALID, *marineActorDef);
		findSlot = false;
		index = -1;
		listSize = (int)m_actors.size();
		for (int actorsIndex = 0; actorsIndex < listSize; ++actorsIndex)
		{
			if (m_actors[actorsIndex] == nullptr)
			{
				m_actors[actorsIndex] = newActor;
				index = actorsIndex;
				findSlot = true;
			}
		}
		if (!findSlot)
		{
			m_actors.push_back(newActor);
			index = listSize;
		}
		newActor->m_definition = marineActorDef;
		newActor->m_color = marineActorDef->m_solidColor;
		newActor->m_wireframeColor = marineActorDef->m_wireframeColor;
		newActor->m_isStatic = false;
		newActor->m_position = spawnPosition;
		m_player2->m_orientation.m_yawDegrees = -90.f;
		newActor->m_UID = ActorUID(index, actorSalt);
		actorSalt++;

		newActor->m_controller = m_player2;
		m_player2->m_UID = newActor->m_UID;
		if (m_game->m_playerOneUsingController)
		{
			m_player2->m_playerIndex = -1;
		}
		else
		{
			m_player2->m_playerIndex = 0;
		}
		m_players.push_back(newActor);
	}
}

Actor* Map::SpawnActor(SpawnInfo* info, Actor* owner)
{
	ActorDefinition* currentActorDef = nullptr;
	if (info->m_isProjectile)
	{
		currentActorDef = ActorDefinition::GetProjectileActorDefByName(info->m_actor);
	}
	else
	{
		currentActorDef = ActorDefinition::GetActorDefByName(info->m_actor);
	}
	Actor* newActor = nullptr;
	if (owner == nullptr)
	{
		newActor = new Actor(m_game, this, ActorUID::INVALID, *currentActorDef);
	}
	else
	{
		newActor = new Actor(m_game, this, owner->m_UID, *currentActorDef);
	}
	bool findSlot = false;
	int index = -1;
	int listSize = (int)m_actors.size();
	for (int actorsIndex = 0; actorsIndex < listSize; ++actorsIndex)
	{
		if (m_actors[actorsIndex] == nullptr && !findSlot)
		{
			m_actors[actorsIndex] = newActor;
			index = actorsIndex;
			findSlot = true;
		}
	}
	if (!findSlot)
	{
		m_actors.push_back(newActor);
		index = listSize;
	}
	newActor->m_definition = currentActorDef;
	newActor->m_color = currentActorDef->m_solidColor;
	newActor->m_wireframeColor = currentActorDef->m_wireframeColor;
	newActor->m_isStatic = false;
	newActor->m_position = info->m_position;
	newActor->m_orientation = EulerAngles(info->m_orientation.x, info->m_orientation.y, info->m_orientation.z);
	newActor->m_UID = ActorUID(index, actorSalt);
	if (newActor->m_definition->m_aiEnabled)
	{
		newActor->m_AIController->m_UID = newActor->m_UID;
	}
	actorSalt++;
	return newActor;
}

Actor* Map::SpawnActor(SpawnInfo* info, Actor* owner, ActorUID target)
{
	ActorDefinition* currentActorDef = nullptr;
	if (info->m_isProjectile)
	{
		currentActorDef = ActorDefinition::GetProjectileActorDefByName(info->m_actor);
	}
	else
	{
		currentActorDef = ActorDefinition::GetActorDefByName(info->m_actor);
	}
	Actor* newActor = nullptr;
	if (owner == nullptr)
	{
		newActor = new Actor(m_game, this, ActorUID::INVALID, *currentActorDef);
	}
	else
	{
		newActor = new Actor(m_game, this, owner->m_UID, *currentActorDef);
	}
	bool findSlot = false;
	int index = -1;
	int listSize = (int)m_actors.size();
	for (int actorsIndex = 0; actorsIndex < listSize; ++actorsIndex)
	{
		if (m_actors[actorsIndex] == nullptr && !findSlot)
		{
			m_actors[actorsIndex] = newActor;
			index = actorsIndex;
			findSlot = true;
		}
	}
	if (!findSlot)
	{
		m_actors.push_back(newActor);
		index = listSize;
	}
	newActor->m_definition = currentActorDef;
	newActor->m_color = currentActorDef->m_solidColor;
	newActor->m_wireframeColor = currentActorDef->m_wireframeColor;
	newActor->m_isStatic = false;
	newActor->m_position = info->m_position;
	newActor->m_orientation = EulerAngles(info->m_orientation.x, info->m_orientation.y, info->m_orientation.z);
	newActor->m_UID = ActorUID(index, actorSalt);
	if (newActor->m_definition->m_aiEnabled)
	{
		newActor->m_AIController->m_UID = newActor->m_UID;
		((AIController*)(newActor->m_AIController))->m_targetUID = target;
	}
	actorSalt++;
	return newActor;
}

Actor* Map::GetActorByUID(ActorUID& uid)
{
	if (!uid.IsValid())
	{
		return nullptr;
	}
	int index = uid.GetIndex();
	return m_actors[index];
}

Actor* Map::GetCloestVisibleEnemy(ActorUID& uid)
{
	if (!uid.IsValid())
	{
		return nullptr;
	}
	RaycastResult3D result;
	Actor* cloestActor = nullptr;
	int index = uid.GetIndex();
	for (int actorIndex = 0; actorIndex < m_actors.size(); ++actorIndex)
	{
		if (actorIndex != index)  // not self
		{
			if (!m_actors[actorIndex] || !m_actors[index]) // nullptr
			{
				continue;
			}
			if (m_actors[actorIndex]->m_definition->m_fraction == m_actors[index]->m_definition->m_fraction || m_actors[actorIndex]->m_definition->m_fraction == Fraction::NEUTRAL) // same fraction
			{
				continue;
			}
			Vec2 enemyPos2D = Vec2(m_actors[actorIndex]->m_position);
			Vec2 selfPos2D  = Vec2(m_actors[index]->m_position);
			Vec2 forward2D = Vec2(m_actors[index]->m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D());

			if (IsPointInsideDirectedSector2D(enemyPos2D, selfPos2D, forward2D, m_actors[index]->m_definition->m_sightAngle, m_actors[index]->m_definition->m_sightRadius))
			{
				Vec3 dir = (m_actors[actorIndex]->m_position - m_actors[index]->m_position).GetNormalized();
				Vec3 camPos = m_actors[index]->m_position;
				camPos.z += m_actors[index]->m_definition->m_eyeHeight;
				RaycastResult3D currentRaycastResult3D = RaycastAll(camPos, dir, m_actors[index]->m_definition->m_sightRadius, m_actors[index]);
				if (result.m_didImpact == false && currentRaycastResult3D.m_didImpact == true)
				{
					result = currentRaycastResult3D;
					cloestActor = m_actors[actorIndex];
				}
				if (result.m_didImpact && currentRaycastResult3D.m_didImpact)
				{
					if (result.m_impactDistance > currentRaycastResult3D.m_impactDistance)
					{
						result = currentRaycastResult3D;
						cloestActor = m_actors[actorIndex];
					}
				}
			}
		}
	}
	return cloestActor;
	
}

void Map::DebugPossessNext()
{
	int currentIndex = m_player->m_UID.GetIndex();
	int next = currentIndex + 1;
	if (next >= m_actors.size())
	{
		next = 0;
	}
	for (int actorsIndex = next; actorsIndex < m_actors.size(); ++actorsIndex)
	{
		if (!m_actors[actorsIndex])
		{
			continue;
		}
		if (m_actors[actorsIndex]->m_definition->m_canBePossessed)
		{
			m_player->Possess(m_actors[actorsIndex]->m_UID);
			return;
		}
	}
}

void Map::RenderActors(bool player1) const
{
	for (int actorsIndex = 0; actorsIndex < m_actors.size(); ++actorsIndex)
	{
		if (m_actors[actorsIndex])
		{
			if (m_actors[actorsIndex] == m_players[0])
			{
				continue;
			}
			if (m_game->m_hasPlayerTwo)
			{
				if (m_actors[actorsIndex] == m_players[1])
				{
					continue;
				}
			}
			
			m_actors[actorsIndex]->Render(player1);

		}
	}
}

void Map::InitializeMapTiles(MapDefinition const& def)
{
	m_dimensions = def.m_image.GetDimensions();
	m_texture = def.m_spriteSheetTexture;
	m_shader = def.m_shader;

	int tileNum = m_dimensions.x * m_dimensions.y;
	m_tiles.reserve(tileNum);
	SpriteSheet* currentSheet = new SpriteSheet(*m_texture, IntVec2(8, 8), IntVec2(256, 256));

	for (int tileIndex = 0; tileIndex < tileNum; ++tileIndex)
	{
		Tile* newTile = new Tile();
		newTile->m_bounds.m_mins.x = GetTileCoordinatesForTileIndex(tileIndex).x * 1.f;
		newTile->m_bounds.m_mins.y = GetTileCoordinatesForTileIndex(tileIndex).y * 1.f;
		newTile->m_bounds.m_maxs.x = (GetTileCoordinatesForTileIndex(tileIndex).x + 1) * 1.f;
		newTile->m_bounds.m_maxs.y = (GetTileCoordinatesForTileIndex(tileIndex).y + 1) * 1.f;
		newTile->m_bounds.m_mins.z = 0.f;
		newTile->m_bounds.m_maxs.z = 1.f;

		Vec3 FBL = Vec3(newTile->m_bounds.m_maxs.x, newTile->m_bounds.m_maxs.y, newTile->m_bounds.m_mins.z);
		Vec3 FBR = Vec3(newTile->m_bounds.m_maxs.x, newTile->m_bounds.m_mins.y, newTile->m_bounds.m_mins.z);
		Vec3 FTR = Vec3(newTile->m_bounds.m_maxs.x, newTile->m_bounds.m_mins.y, newTile->m_bounds.m_maxs.z);
		Vec3 FTL = Vec3(newTile->m_bounds.m_maxs.x, newTile->m_bounds.m_maxs.y, newTile->m_bounds.m_maxs.z);

		Vec3 BBL = Vec3(newTile->m_bounds.m_mins.x, newTile->m_bounds.m_maxs.y, newTile->m_bounds.m_mins.z);
		Vec3 BBR = Vec3(newTile->m_bounds.m_mins.x, newTile->m_bounds.m_mins.y, newTile->m_bounds.m_mins.z);
		Vec3 BTR = Vec3(newTile->m_bounds.m_mins.x, newTile->m_bounds.m_mins.y, newTile->m_bounds.m_maxs.z);
		Vec3 BTL = Vec3(newTile->m_bounds.m_mins.x, newTile->m_bounds.m_maxs.y, newTile->m_bounds.m_maxs.z);
		m_tiles.push_back(newTile);

		int lengthOfSprite = 8;
		Rgba8 currentMapTexelColor = def.m_image.GetTexelColor(GetTileCoordinatesForTileIndex(tileIndex));
		TileDefinition* currentTileDef = TileDefinition::GetTileDefByTexelColor(currentMapTexelColor);
		newTile->m_def = currentTileDef;
		// floor 
		if (currentTileDef->m_floorSpriteCoords != IntVec2(-1, -1))
		{
			int spriteIndex = currentTileDef->m_floorSpriteCoords.y * lengthOfSprite + currentTileDef->m_floorSpriteCoords.x;
			AABB2 currentSpriteSheetUVs = currentSheet->GetSpriteUVs(spriteIndex);
			AddVertsForQuad3D(m_vertexes, m_indexes, FBL, BBL, BBR, FBR, Rgba8::WHITE, currentSpriteSheetUVs);
		}
		// wall
		if (currentTileDef->m_wallSpriteCoords != IntVec2(-1, -1))
		{
			int spriteIndex = currentTileDef->m_wallSpriteCoords.y * lengthOfSprite + currentTileDef->m_wallSpriteCoords.x;
			AABB2 currentSpriteSheetUVs = currentSheet->GetSpriteUVs(spriteIndex);
			AddVertsForQuad3D(m_vertexes, m_indexes, BTL, BBL, BBR, BTR, Rgba8::WHITE, currentSpriteSheetUVs);
			AddVertsForQuad3D(m_vertexes, m_indexes, FTL, FBL, BBL, BTL, Rgba8::WHITE, currentSpriteSheetUVs);
			AddVertsForQuad3D(m_vertexes, m_indexes, BTR, BBR, FBR, FTR, Rgba8::WHITE, currentSpriteSheetUVs);
			AddVertsForQuad3D(m_vertexes, m_indexes, FTR, FBR, FBL, FTL, Rgba8::WHITE, currentSpriteSheetUVs);
		}
		// ceiling
		if (currentTileDef->m_ceillingSpriteCoords != IntVec2(-1, -1))
		{
			int spriteIndex = currentTileDef->m_ceillingSpriteCoords.y * lengthOfSprite + currentTileDef->m_ceillingSpriteCoords.x;
			AABB2 currentSpriteSheetUVs = currentSheet->GetSpriteUVs(spriteIndex);
			AddVertsForQuad3D(m_vertexes, m_indexes, BTL, FTL, FTR, BTR, Rgba8::WHITE, currentSpriteSheetUVs);
		}
	}

	m_indexBuffer = g_theRenderer->CreateIndexBuffer(m_indexes.size() * sizeof(unsigned int));
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(m_vertexes.size() * sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));

	g_theRenderer->CopyCPUToGPU(m_vertexes.data(), (size_t)m_vertexes.size() * m_vertexBuffer->GetStride(), m_vertexBuffer);
	g_theRenderer->CopyCPUToGPU(m_indexes.data(), (size_t)m_indexes.size() * m_indexBuffer->GetStride(), m_indexBuffer);
	delete currentSheet;
}

void Map::SpawnActors(MapDefinition const& def)
{
	int InfoSize = (int)def.m_SpawnInfos.size();
	for (int spawnIndex = 0; spawnIndex < InfoSize; ++spawnIndex)
	{
		SpawnInfo* currentInfo = def.m_SpawnInfos[spawnIndex];
		SpawnActor(currentInfo, nullptr);
	}
}

void Map::UpdateLightingData()
{
	m_sunIntensity = GetClamped(m_sunIntensity, 0.f, 1.f);
	m_ambientIntensity = GetClamped(m_ambientIntensity, 0.f, 1.f);

	if (g_theInput->WasKeyJustPressed(F1_KEY))
	{
		std::string msg = Stringf("Sun direction x: %.2f ( F2 / F3 to change )\nSun direction y: %.2f ( F4 / F5 to change )\nSun intensity:   %.2f ( F6 / F7 to change )\nAmbient intensity: %.2f ( F8 / F9 to change ) ",
									m_sunDirection.x, m_sunDirection.y, m_sunIntensity, m_ambientIntensity);
		DebugAddMessage(msg, 4.f);
	}
	if (g_theInput->WasKeyJustPressed(F2_KEY))
	{
		m_sunDirection.x += -1.f;
		std::string msg = Stringf("Sun direction x: %.2f", m_sunDirection.x);
		DebugAddMessage(msg, 4.f);
	}
	if (g_theInput->WasKeyJustPressed(F3_KEY))
	{
		m_sunDirection.x += 1.f;
		std::string msg = Stringf("Sun direction x: %.2f", m_sunDirection.x);
		DebugAddMessage(msg, 4.f);
	}
	if (g_theInput->WasKeyJustPressed(F4_KEY))
	{
		m_sunDirection.y += -1.f;
		std::string msg = Stringf("Sun direction y: %.2f", m_sunDirection.y);
		DebugAddMessage(msg, 4.f);
	}
	if (g_theInput->WasKeyJustPressed(F5_KEY))
	{
		m_sunDirection.y += 1.f;
		std::string msg = Stringf("Sun direction y: %.2f", m_sunDirection.y);
		DebugAddMessage(msg, 4.f);
	}
	if (g_theInput->WasKeyJustPressed(F6_KEY))
	{
		m_sunIntensity += -0.05f;
		std::string msg = Stringf("Sun intensity: %.2f", m_sunIntensity);
		DebugAddMessage(msg, 4.f);
	}
	if (g_theInput->WasKeyJustPressed(F7_KEY))
	{
		m_sunIntensity += 0.05f;
		std::string msg = Stringf("Sun intensity: %.2f", m_sunIntensity);
		DebugAddMessage(msg, 4.f);
	}
	if (g_theInput->WasKeyJustPressed(F8_KEY))
	{
		m_ambientIntensity += -0.05f;
		std::string msg = Stringf("ambient intensity: %.2f", m_ambientIntensity);
		DebugAddMessage(msg, 4.f);
	}
	if (g_theInput->WasKeyJustPressed(F9_KEY))
	{
		m_ambientIntensity += 0.05f;
		std::string msg = Stringf("ambient intensity: %.2f", m_ambientIntensity);
		DebugAddMessage(msg, 4.f);
	}

}

