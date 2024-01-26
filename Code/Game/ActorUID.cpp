#include "ActorUID.hpp"

const ActorUID ActorUID::INVALID(0xFFFFFFFF);

void ActorUID::operator=(const ActorUID& copyFrom)
{
	m_data = copyFrom.m_data;
}

bool ActorUID::operator!=(const ActorUID& compare) const
{
	if (m_data != compare.m_data)
	{
		return true;
	}
	return false;
}

bool ActorUID::operator==(const ActorUID& compare) const
{
	if (m_data == compare.m_data)
	{
		return true;
	}
	return false;
}

ActorUID::ActorUID(unsigned int index, unsigned int salt)
{
	m_data = (salt << 16) | index;
}

ActorUID::ActorUID(unsigned int raw)
	:m_data(raw)
{

}

ActorUID::~ActorUID()
{

}

unsigned int ActorUID::GetIndex() const
{
	return m_data & 0xFFFF;
}

bool ActorUID::IsValid() const
{
	if (m_data == ActorUID::INVALID.m_data)
	{
		return false;
	}
	return true;
}

