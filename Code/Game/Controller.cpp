#include "Actor.hpp"
#include "Controller.hpp"

Controller::Controller()
{

}

Controller::~Controller()
{

}

void Controller::Possess(ActorUID UID)
{
	if (!UID.IsValid())
	{
		return;
	}
	if (UID != m_UID)
	{
		GetActor()->OnUnPossessed();
		m_UID = UID;
		GetActor()->m_controller = this;
		GetActor()->OnPossessed();
	}
}

Actor* Controller::GetActor()
{
	if (!m_UID.IsValid())
	{
		return nullptr;
	}
	else
	{
		int actorIndex = m_UID.GetIndex();
		return m_currentMap->m_actors[actorIndex];
	}
}

