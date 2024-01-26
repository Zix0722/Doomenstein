#pragma once

class ActorUID
{
public:
	ActorUID(unsigned int raw);
	ActorUID(unsigned int index, unsigned int salt);
	~ActorUID();

	unsigned int GetIndex() const;
	bool IsValid() const;
	static const ActorUID INVALID;

	bool		operator==(const ActorUID& compare) const;		
	bool		operator!=(const ActorUID& compare) const;
	void		operator=(const ActorUID& copyFrom);
private:
	unsigned int m_data;
}; 