#include "AnimationGroup.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"


AnimationGroup::AnimationGroup()
{

}

AnimationGroup::~AnimationGroup()
{
	//m_anims.clear();
}

SpriteAnimDefinition* AnimationGroup::GetAnimDefByDirection(Vec2 const&direction)
{
	int indexWithMaxDotResult = -1;
	float maxVal = -99999999999999999999.f;
	for (int dirIndex = 0; dirIndex < m_directions.size(); ++dirIndex)
	{
		Vec2 dir = m_directions[dirIndex].GetVec2().GetNormalized();
		float dotResult = DotProduct2D(dir, direction);
		if (dotResult > maxVal)
		{
			maxVal = dotResult;
			indexWithMaxDotResult = dirIndex;
		}
	}
	if (indexWithMaxDotResult != -1)
	{
		return m_anims[indexWithMaxDotResult];
	}
	else
	{
		return nullptr;
	}
	
}

bool AnimationGroup::LoadFromXmlElement(XmlElement const& element, SpriteSheet const& spriteSheet)
{
	m_name = ParseXMLAttribute(element, "name", m_name);
	m_scaleBySpeed = ParseXMLAttribute(element, "scaleBySpeed", m_scaleBySpeed);
	XmlElement const* childElement = element.FirstChildElement("Direction");
	while (childElement != nullptr)
	{
		Vec3 direction = ParseXMLAttribute(*childElement, "vector", direction);
		SpriteAnimDefinition* newAnim = new SpriteAnimDefinition(spriteSheet, 0, 0);
		XmlElement const* animElement = childElement->FirstChildElement("Animation");
		if (animElement)
		{
			newAnim->LoadFromXmlElement(element, *animElement);
		}
		m_directions.push_back(direction);
		m_anims.push_back(newAnim);
		childElement = childElement->NextSiblingElement("Direction");
	}
	return true;
}
