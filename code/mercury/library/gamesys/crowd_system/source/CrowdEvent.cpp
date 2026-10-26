#include "CrowdSystemPch.h"

#include <crowd_system/CrowdEvent.h>
#include <crowd_system/CrowdMaths.h>

namespace AnimatedCrowd
{

C_Event::C_Event()
	: m_Type(E_CROWD_EVENT_UNKNOWN),
	m_AreaType(E_AREA_TYPE_GLOBAL),
	m_Lifetime(0.f),
	m_Duration(0.f)
{ }

C_Event::C_Event(const CrowdEventType eventType, const Maths::HVec3& position, const Maths::HVec3& direction)
	: m_Type(eventType)
{
	Init(position, direction);
}

C_Event::C_Event(const CrowdEventType eventType, const Maths::HVec3& position)
	: m_Type(eventType)
{
	Init(position, Maths::HVec3(1.f, 0.f, 0.f));
}

C_Event::C_Event(const CrowdEventType eventType)
	: m_Type(eventType)
{
	Init(Maths::HVec3(0.f), Maths::HVec3(1.f, 0.f, 0.f));
}

void C_Event::Init(const Maths::HVec3& position, const Maths::HVec3& direction)
{
	m_Position = position;
	m_Direction = direction;

	switch (m_Type)
	{
	case E_CROWD_EVENT_LOCAL_UNHAPPY:
	case E_CROWD_EVENT_LOCAL_IDLE:
	case E_CROWD_EVENT_LOCAL_INTERESTED:
	case E_CROWD_EVENT_LOCAL_EXCITED:
	case E_CROWD_EVENT_LOCAL_ECSTATIC:
		{
			m_AreaType		= E_AREA_TYPE_CYLINDER;
			m_Radius		= 15.f;
			m_HalfHeight	= 3.5f;
		}
		break;
	case E_CROWD_EVENT_LOCAL_REACT_TO_DANGER:
		{
			m_AreaType		= E_AREA_TYPE_CYLINDER;
			m_Radius		= 10.f;
			m_HalfHeight	= 2.5f;
		}
		break;
	case E_CROWD_EVENT_GLOBAL_UNHAPPY:
	case E_CROWD_EVENT_GLOBAL_IDLE:
	case E_CROWD_EVENT_GLOBAL_INTERESTED:
	case E_CROWD_EVENT_GLOBAL_EXCITED:
	case E_CROWD_EVENT_GLOBAL_ECSTATIC:
		{
			m_AreaType		= E_AREA_TYPE_GLOBAL;
		}
		break;
	case E_CROWD_EVENT_STATE_OVERRIDE:
	default:
		break;
	}

	m_Duration = GetEventDuration(static_cast<CrowdEventType>(m_Type));
	m_Lifetime = m_Duration;
}

const CrowdEventType C_Event::GetType() const
{
	return static_cast<CrowdEventType>(m_Type);
}

const C_Event::AreaType C_Event::GetAreaType() const
{
	return static_cast<C_Event::AreaType>(m_AreaType);
}

const Maths::HVec3& C_Event::GetPosition() const
{
	return m_Position;
}

const F32 C_Event::GetRadius() const
{
	return m_Radius;
}

const F32 C_Event::GetHalfHeight() const
{
	return m_HalfHeight;
}

const Maths::HVec3& C_Event::GetDirection() const
{
	return m_Direction;
}

const F32 C_Event::GetMagnitude() const
{
	return m_Magnitude;
}


const F32 C_Event::GetNormalisedDistance(const Maths::HVec3& point) const
{
	F32 d = 0.f;

	if (m_AreaType == E_AREA_TYPE_CYLINDER)
	{
		d = Maths::Length(point - GetPosition()) / GetRadius();
		// TODO: [rthomas]	Use height to clamp to 1.f if outside height range
	}
	else if (m_AreaType == E_AREA_TYPE_PYRAMID)
	{
		d = 1.f; // TODO: [rthomas]
	}

	return Maths::Clamp<F32>(d, 0.f, 1.f);
}

const char* C_Event::GetName() const
{
	switch (m_Type)
	{
	case E_CROWD_EVENT_LOCAL_UNHAPPY:
		return "LocalUnhappy"; break;
	case E_CROWD_EVENT_LOCAL_IDLE:
		return "LocalIdle"; break;
	case E_CROWD_EVENT_LOCAL_INTERESTED:
		return "LocalInterested"; break;
	case E_CROWD_EVENT_LOCAL_EXCITED:
		return "LocalExcited"; break;
	case E_CROWD_EVENT_LOCAL_ECSTATIC:
		return "LocalEcstatic"; break;
	case E_CROWD_EVENT_LOCAL_REACT_TO_DANGER:
		return "LocalDanger"; break;
	case E_CROWD_EVENT_GLOBAL_UNHAPPY:
		return "GlobalUnhappy"; break;
	case E_CROWD_EVENT_GLOBAL_IDLE:
		return "GlobalIdle"; break;
	case E_CROWD_EVENT_GLOBAL_INTERESTED:
		return "GlobalInterested"; break;
	case E_CROWD_EVENT_GLOBAL_EXCITED:
		return "GlobalExcited"; break;
	case E_CROWD_EVENT_GLOBAL_ECSTATIC:
		return "GlobalEcstatic"; break;
	case E_CROWD_EVENT_STATE_OVERRIDE:
		return "StateOverride"; break;
	default:
		BIZ_ASSERT(false);
		break;
	}

	return NULL;
}

F32 C_Event::GetEventDuration(const CrowdEventType eventType)
{
	switch (eventType)
	{
	case E_CROWD_EVENT_LOCAL_UNHAPPY:
		return 0.5f; break;
	case E_CROWD_EVENT_LOCAL_IDLE:
		return 1.f; break;
	case E_CROWD_EVENT_LOCAL_INTERESTED:
		return 0.25f; break;
	case E_CROWD_EVENT_LOCAL_EXCITED:
		return 0.5f; break;
	case E_CROWD_EVENT_LOCAL_ECSTATIC:
		return 1.f; break;
	case E_CROWD_EVENT_LOCAL_REACT_TO_DANGER:
		return 0.25f; break;
	case E_CROWD_EVENT_GLOBAL_UNHAPPY:
		return 4.f; break;
	case E_CROWD_EVENT_GLOBAL_IDLE:
		return 2.f; break;
	case E_CROWD_EVENT_GLOBAL_INTERESTED:
		return 3.f; break;
	case E_CROWD_EVENT_GLOBAL_EXCITED:
		return 4.f; break;
	case E_CROWD_EVENT_GLOBAL_ECSTATIC:
		return 5.f; break;
	case E_CROWD_EVENT_STATE_OVERRIDE:
		return 0.f; break;
	default:
		BIZ_ASSERT(false);
		break;
	}

	return 0.f;
}

void C_Event::Update(const F32 elapsedTime)
{
	m_Lifetime -= elapsedTime;
}

const bool C_Event::HasExpired() const
{
	return ((m_Type == E_CROWD_EVENT_UNKNOWN) || (m_Lifetime < 0.f));
}

const U32 C_Event::GetPriority() const
{
	static unsigned char priorities[E_CROWD_EVENT_COUNT + 1] =
	{
		10,	// E_CROWD_EVENT_LOCAL_UNHAPPY
		7,	// E_CROWD_EVENT_LOCAL_IDLE
		8,	// E_CROWD_EVENT_LOCAL_INTERESTED
		9,	// E_CROWD_EVENT_LOCAL_EXCITED
		11,	// E_CROWD_EVENT_LOCAL_ECSTATIC
		12,	// E_CROWD_EVENT_LOCAL_REACT_TO_DANGER

		4,	// E_CROWD_EVENT_GLOBAL_UNHAPPY
		1,	// E_CROWD_EVENT_GLOBAL_IDLE
		2,	// E_CROWD_EVENT_GLOBAL_INTERESTED
		3,	// E_CROWD_EVENT_GLOBAL_EXCITED
		5,	// E_CROWD_EVENT_GLOBAL_ECSTATIC

		6,	// E_CROWD_EVENT_STATE_OVERRIDE

		0	// E_CROWD_EVENT_UNKNOWN
	};

	return priorities[m_Type];
}

C_Event::AffectsBlockResult C_Event::AffectsBlock(const Maths::C_AxisBox& axisBox) const
{
	if (m_Type == E_CROWD_EVENT_UNKNOWN)
		return E_AFFECT_RESULT_NONE;

	if (m_AreaType == E_AREA_TYPE_PYRAMID)
	{
		return E_AFFECT_RESULT_NONE; // TODO: [rthomas]
	}
	else if (m_AreaType == E_AREA_TYPE_CYLINDER)
	{
		if (AABBCylinderIntersect(axisBox, m_Position, m_Radius, m_HalfHeight))
			return E_AFFECT_RESULT_ALL;
		return E_AFFECT_RESULT_NONE; // TODO: [rthomas]
	}

	return E_AFFECT_RESULT_ALL;
}

bool C_Event::AffectsSpectator(const Maths::HVec3& position, const Maths::HVec3& direction, const F32 angleRange) const
{
	switch (m_Type)
	{
	case E_CROWD_EVENT_LOCAL_UNHAPPY:
	case E_CROWD_EVENT_LOCAL_INTERESTED:
	case E_CROWD_EVENT_LOCAL_EXCITED:
	case E_CROWD_EVENT_LOCAL_ECSTATIC:
		{
			Maths::HVec3 eventDirection = (position - m_Position);
			eventDirection.Normalise();
			
			return (acosf(Maths::Clamped<F32>(/*Maths::Abs<F32>*/(eventDirection | -direction), -1.f, 1.f)) <= angleRange);
		}	
	case E_CROWD_EVENT_LOCAL_REACT_TO_DANGER:
		{
			//return (Maths::Abs<F32>(m_Direction | direction) <= angleRange);
			return (acosf(Maths::Clamped<F32>(/*Maths::Abs<F32>*/(m_Direction | -direction), -1.f, 1.f)) <= angleRange);
		}
	case E_CROWD_EVENT_LOCAL_IDLE:
	case E_CROWD_EVENT_GLOBAL_UNHAPPY:
	case E_CROWD_EVENT_GLOBAL_IDLE:
	case E_CROWD_EVENT_GLOBAL_INTERESTED:
	case E_CROWD_EVENT_GLOBAL_EXCITED:
	case E_CROWD_EVENT_GLOBAL_ECSTATIC:
	case E_CROWD_EVENT_STATE_OVERRIDE:
	default:
		return true;
	}
}

}; // namespace AnimatedCrowd
