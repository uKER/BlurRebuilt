#ifndef CROWD_EVENT_H
#define CROWD_EVENT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <crowd_system/CrowdDefines.h>

//#include <btl/containers/Vector.h>

namespace AnimatedCrowd
{
	class C_CrowdBlock;
};

namespace AnimatedCrowd
{

//! C_Event
class ALIGN16 C_Event
{
public:
	C_Event();
	C_Event(const CrowdEventType eventType, const Maths::HVec3& position, const Maths::HVec3& direction);
	C_Event(const CrowdEventType eventType, const Maths::HVec3& position);
	C_Event(const CrowdEventType eventType);

	enum AreaType
	{
		E_AREA_TYPE_CYLINDER,
		E_AREA_TYPE_PYRAMID,
		E_AREA_TYPE_GLOBAL
	};

	enum AffectsBlockResult
	{
		E_AFFECT_RESULT_NONE	= 0,
		E_AFFECT_RESULT_PARTIAL,
		E_AFFECT_RESULT_ALL
	};

	const CrowdEventType	GetType() const;
	const AreaType			GetAreaType() const;
	const Maths::HVec3&		GetPosition() const;
	const F32				GetRadius() const;
	const F32				GetHalfHeight() const;
	const Maths::HVec3&		GetDirection() const;
	const F32				GetMagnitude() const;
	
	const F32				GetNormalisedDistance(const Maths::HVec3& point) const;

	const char*				GetName() const;
	static F32				GetEventDuration(const CrowdEventType eventType);

	void					Update(const F32 elapsedTime);

	const bool				HasExpired() const;

	const U32				GetPriority() const;

	AffectsBlockResult		AffectsBlock(const Maths::C_AxisBox& axisBox) const;

	bool					AffectsSpectator(const Maths::HVec3& position, const Maths::HVec3& direction, const F32 angleRange) const;

private:
	void					Init(const Maths::HVec3& position, const Maths::HVec3& direction);

	Maths::HVec3	m_Position;
	Maths::HVec3	m_Direction;

	U16				m_Type;
	U16				m_AreaType;
	
	F32				m_Radius;
	F32				m_HalfHeight;
	
	F32				m_Magnitude;
	F32				m_Duration;
	F32				m_Lifetime;
};

//typedef Btl::Vector<C_Event, Btl::VectorParameters_Static<CROWD_MAX_EVENTS> >	EventVector;

}; // namespace AnimatedCrowd

#endif // CROWD_EVENT_H
