#pragma once
#include <string>
#include <map>
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

class AnimationGroup
{
public:

	AnimationGroup();
	~AnimationGroup();

	SpriteAnimDefinition* GetAnimDefByDirection(Vec2 const& direction);
	bool LoadFromXmlElement(XmlElement const& element, SpriteSheet const& spriteSheet);
public:
	std::string m_name;
	bool m_scaleBySpeed = false;
	std::vector<Vec3> m_directions;
	std::vector<SpriteAnimDefinition*> m_anims;
};