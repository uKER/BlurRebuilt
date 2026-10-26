#ifndef CROWD_INSTANCE_H
#define CROWD_INSTANCE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <crowd_system/CrowdDefines.h>

namespace AnimatedCrowd
{

//! C_CrowdInstance

class C_CrowdInstance
{
public:

	C_CrowdInstance()
		: m_RandomSeed(0),
#ifdef CFG_CONVERTER
		m_Position(0.f, 0.f, 0.f),
		m_Direction(0.f, 0.f, 1.f),
		m_Scale(1.f),
		m_Route(-1),
#endif
		m_CharacterIndex(0)
	{
	}

	~C_CrowdInstance()
	{
	}

	Maths::HVec3 GetPosition() const
	{
#ifdef CFG_CONVERTER
		return m_Position;
#else
		return Maths::Load<Maths::HVec3, MathsStorage::Float3>(&m_Position);
#endif
	}

	Maths::HVec3 GetDirection() const
	{
#ifdef CFG_CONVERTER
		return m_Direction;
#else
		return Maths::Load<Maths::HVec3, MathsStorage::HenDN3>(&m_Direction);
#endif
	}

#ifdef CFG_CONVERTER
	const Maths::HF32 GetScale() const
	{
		return m_Scale;
	}
#endif

	const U16 GetCharacterIndex() const
	{
		return m_CharacterIndex;
	}

#ifdef CFG_CONVERTER
	const S32 GetRoute() const
	{
		return m_Route;
	}
#endif

	const U32 GetRandomSeed() const
	{
		return m_RandomSeed;
	}

	const S16 GetUniqueId() const
	{
		return m_UniqueId;
	}

#ifdef CFG_CONVERTER
	void SetPosition(const Maths::HVec3& position)
	{
		m_Position = position;
	}

	void SetPosition(const Maths::HF32 pX, const Maths::HF32 pY, const Maths::HF32 pZ)
	{
		m_Position.X(pX);
		m_Position.Y(pY);
		m_Position.Z(pZ);
	}

	void SetDirection(const Maths::HVec3& direction)
	{
		m_Direction = direction;
	}

	void SetScale(const Maths::HF32 scale)
	{
		m_Scale = scale;
	}

	void SetCharacterIndex(const S32 index)
	{
		m_CharacterIndex = index;
	}

	void SetRoute(const S32 route)
	{
		m_Route = route;
	}

	void SetRandomSeed(const U32 seed)
	{
		m_RandomSeed = seed;
	}

	void SetUniqueId(const S32 uniqueId)
	{
		m_UniqueId = uniqueId;
	}

#endif

#ifdef CFG_CONVERTER
	template< typename Archive > 
	void Serialise( Archive& ar )
	{
		ar << m_Position;
		ar << m_Direction;

		ar << m_Scale;

		ar << m_CharacterIndex;

		ar << m_RandomSeed;

		ar << m_UniqueId;
	}
#endif

#ifndef CFG_CONVERTER
	template< typename Archive > 
	void Deserialise( Archive& ar )
	{
		Maths::HVec3 position;
		ar >> position;
		Maths::Store(position, &m_Position);

		Maths::HVec3 direction;
		ar >> direction;
		Maths::Store(direction, &m_Direction);

		Maths::HF32 fScale;
		ar >> fScale;

		U32 characterIndex;
		ar >> characterIndex;
		m_CharacterIndex = characterIndex;

		ar >> m_RandomSeed;

		S32 uniqueId;
		ar >> uniqueId;
		m_UniqueId = uniqueId;
	}
#endif

private:

#ifdef CFG_CONVERTER
	Maths::HVec3	m_Position;
	Maths::HVec3	m_Direction;

	Maths::HF32		m_Scale;
#else
	MathsStorage::Float3	m_Position;
	MathsStorage::HenDN3	m_Direction;
#endif

#ifdef CFG_CONVERTER
	U32				m_CharacterIndex;
	S32				m_UniqueId;
#else
	U16				m_CharacterIndex;
	S16				m_UniqueId;
#endif

#ifdef CFG_CONVERTER
	S32				m_Route;
#endif

	U32				m_RandomSeed;
};

}; // namespace AnimatedCrowd

#endif // CROWD_INSTANCE_H
