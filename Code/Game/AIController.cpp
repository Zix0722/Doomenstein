#include "AIController.hpp"

AIController::AIController()
{

}

AIController::~AIController()
{

}

void AIController::Update(float deltaSeconds)
{
	if (m_targetUID == ActorUID::INVALID)
	{
		Actor* result = m_currentMap->GetCloestVisibleEnemy(m_UID);
		if (result)
		{
			m_targetUID = result->m_UID;
		}
	}
	else
	{
		int index = m_targetUID.GetIndex();
		Actor* targetActor = m_currentMap->m_actors[index];
		if (targetActor)
		{
			Actor* self = GetActor();
			Vec3 currentDir = self->m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
			Vec3 disp = targetActor->m_position - self->m_position;
			self->TurnInDirection(disp.GetNormalized(), self->m_definition->m_turnSpeed * deltaSeconds);
			self->MoveInDirection(currentDir.GetNormalized(), self->m_definition->m_runSpeed);
			float dist = disp.GetLength() - targetActor->m_physics_radius - self->m_physics_radius;
			if (dist < self->m_currentWeapon->m_definition->m_meleeRange)
			{
				self->m_currentWeapon->Fire(self);
			}
		}
		else
		{
			m_targetUID = ActorUID::INVALID;
		}
		Actor* self = GetActor();
		if (self && !self->m_hasSpawnedBaby && self->m_isDead && self->m_definition->m_name == "GreenDemon")
		{
			Map* currentMap = self->m_map;
			SpawnInfo currentInfo;
			currentInfo.m_actor = "DemonBaby";
			currentInfo.m_position = self->m_position + Vec3(1.3f, 0.f, 0.f);
			currentInfo.m_orientation = self->m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
			currentMap->SpawnActor(&currentInfo, nullptr, m_targetUID);
			currentInfo.m_position = self->m_position + Vec3(-1.3f, 0.f, 0.f);
			currentMap->SpawnActor(&currentInfo, nullptr, m_targetUID);
			currentInfo.m_position = self->m_position + Vec3(-0.f, 1.3f, 0.f);
			currentMap->SpawnActor(&currentInfo, nullptr, m_targetUID);
			currentInfo.m_position = self->m_position + Vec3(-0.f, -1.3f, 0.f);
			currentMap->SpawnActor(&currentInfo, nullptr, m_targetUID);
			self->m_hasSpawnedBaby = true;
		}
	}
}

void AIController::DamagedBy(ActorUID& UID)
{
	m_targetUID = UID;
}

