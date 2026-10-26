#ifndef __PHYSICS_EVENT_H__
#define __PHYSICS_EVENT_H__

namespace Impact
{

class Body;

/// bitfield to filter event types
#define IMPACT_EVENTFILTER_NONE			0
#define IMPACT_EVENTFILTER_COLLISION	1

/// the impact type
enum E_EventType
{
	E_EventType_Collision,
};

/// parameters specific to a collision event
struct CollisionEvent
{
	Body			*pOtherBody;
	float			fImpulseStrength;
	float			vPos[3];
};

/// contains information about a physics event that occurred
struct Event
{
	E_EventType	type;
	Body		*pBody;

	union
	{
		CollisionEvent	collisionEvent;
	};
};

/// contains a list of events that have happened
class EventList : public Ut::Vector<Event>
{
};

};

#endif
