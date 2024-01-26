#pragma once
#include "ActorUID.hpp"
#include "Map.hpp"


class Controller
{
public:
	Controller();
	~Controller();

	void Possess(ActorUID UID);
	Actor* GetActor();
public:
	ActorUID m_UID = ActorUID::INVALID;
	Map* m_currentMap = nullptr;

};