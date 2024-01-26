#pragma once
#include "Controller.hpp"

class AIController : public Controller
{
public:
	AIController();
	~AIController();

	void Update(float deltaSeconds);
	void DamagedBy(ActorUID& UID);

public:
	ActorUID m_targetUID = ActorUID::INVALID;
};