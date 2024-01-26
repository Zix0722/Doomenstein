#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>
#include "Actor.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "MapDefinition.hpp"

class Texture;
class Shader;
class VertexBuffer;
class IndexBuffer;
class Tile;
class Game;
class Player;

class Map
{
public:
	Map(Game* owner, MapDefinition const& def);
	~Map();
	void Update(float deltaseconds);
	void Render(bool player1 = true) const;

	IntVec2 GetTileCoordinatesForTileIndex(int tileIndex);
	bool IsPositionInBounds(Vec3 position, float const tolerance = 0.f) const;
	bool AreCoordsInBounds(int x, int y) const;
	Tile* const GetTile(int x, int y) const;
	IntVec2 GetTileCoordsForWorldPos(Vec3 const& worldPos) const;
	int GetTileIndexByTileCoords(IntVec2 const& tileCoords) const;

	void CollideActors();
	void CollideActors(Actor* actorA, Actor* actorB);
	void CollideActorWithMap();
	void CollideActorWithMap(Actor* actor);

	RaycastResult3D RaycastAll(Vec3 const& start, Vec3 const& direction, float distance) const;
	RaycastResult3D RaycastWorldXY(Vec3 const& start, Vec3 const& direction, float distance) const;
	RaycastResult3D RaycastWorldZ(Vec3 const& start, Vec3 const& direction, float distance) const;
	RaycastResult3D RaycastWorldActors(Vec3 const& start, Vec3 const& direction, float distance) const;

	RaycastResult3D RaycastWorldActors(Vec3 const& start, Vec3 const& direction, float distance, Actor* owner) const;
	RaycastResult3D RaycastAll(Vec3 const& start, Vec3 const& direction, float distance, Actor* owner) const;

	void DeleteDestroyActors();
	void SpawnPlayer();
	Actor* SpawnActor(SpawnInfo* info, Actor* owner);
	Actor* SpawnActor(SpawnInfo* info, Actor* owner, ActorUID target);
	Actor* GetActorByUID(ActorUID& uid);
	Actor* GetCloestVisibleEnemy(ActorUID& uid);
	void DebugPossessNext();
public:
	Game* m_game = nullptr;
	Player* m_player = nullptr;
	Player* m_player2 = nullptr;
	Actor* m_fakeProjectile = nullptr;
	std::vector<Tile*> m_tiles;
	std::vector<Actor*> m_actors;
	std::vector<Actor*> m_players;

	IntVec2 m_dimensions;
	Texture* m_texture = nullptr;
	Shader* m_shader = nullptr;
	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
	std::vector<Vertex_PCUTBN> m_vertexes;
	std::vector<unsigned int> m_indexes;
	unsigned char actorSalt = 1;
private:
	void RenderActors(bool player1 = true) const;
	void InitializeMapTiles(MapDefinition const& def);
	void SpawnActors(MapDefinition const& def);
	void UpdateLightingData();

	Vec3 m_sunDirection = Vec3(2, 1, -1);
	float m_sunIntensity = 0.85f;
	float m_ambientIntensity = 0.35f;
};
