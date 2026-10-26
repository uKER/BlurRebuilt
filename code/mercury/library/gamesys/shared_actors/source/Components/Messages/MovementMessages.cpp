#include "SharedActorsPch.h"


//#include "GameActors/GameActor.h"
//#include "GameWorld/GameWorld.h"
#include <actor_system/actor.h>
#include <shared_actors/XtSharedLevelActorParams.h>
#include <shared_actors/Components/Messages/MovementMessages.h>
#include <core\containers\algorithm.h>

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", off)
#endif

namespace ActorMsg
{

U32 MovementFlagsFromType(EMovementType type)
{
	switch(type)
	{
	case EMovementType_Linear:
		return E_MovementFlag_Linear;
	case EMovementType_EaseIn:
		return E_MovementFlag_EaseIn;
	case EMovementType_EaseOut:
		return E_MovementFlag_EaseOut;
	case EMovementType_Smooth:
		return E_MovementFlag_Smooth;
	}

	return 0;
}
// Messages

// Message Serialisation Code

Messaging::C_MessageSerialiser C_ActorMovementSpinStopMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

void C_ActorMovementSpinStopMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_ActorMovementSpinStopMsg* pMsg = Messaging::MessageCast<C_ActorMovementSpinStopMsg>(pMessage);

	stream << pMsg->m_Time;
}

void C_ActorMovementSpinStopMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_ActorMovementSpinStopMsg* pMsg = reinterpret_cast<C_ActorMovementSpinStopMsg*>(pMessage);

	stream >> pMsg->m_Time;
}


Messaging::C_MessageSerialiser C_ActorMovementUpdateMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_ActorMovementUpdateMsg::C_ActorMovementUpdateMsg(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans parentWorldTransform, const Components::E_MovementTransform transform) : 
	//m_RecalcTransform(recalcTransform), 
	m_TimeStep(timeStep), 
	m_QuatTrans(parentWorldTransform),
	m_Transform(transform)
{
}

C_ActorMovementUpdateMsg::~C_ActorMovementUpdateMsg()
{
}

void C_ActorMovementUpdateMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_ActorMovementUpdateMsg* pMsg = Messaging::MessageCast<C_ActorMovementUpdateMsg>(pMessage);

	//stream << pMsg->m_RecalcTransform;
	stream.WriteAs<U32>((U32)pMsg->m_Transform);
	stream << pMsg->m_TimeStep;
	stream << pMsg->m_QuatTrans;
}

void C_ActorMovementUpdateMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_ActorMovementUpdateMsg* pMsg = reinterpret_cast<C_ActorMovementUpdateMsg*>(pMessage);

	//stream >> pMsg->m_RecalcTransform;
	pMsg->m_Transform = (Components::E_MovementTransform)stream.ReadAs<U32>();
	stream >> pMsg->m_TimeStep;
	stream >> pMsg->m_QuatTrans;
}





Messaging::C_MessageSerialiser C_ActorMovementTranslateMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_ActorMovementTranslateMsg::C_ActorMovementTranslateMsg() :
	m_MovementFlags(0), 
	m_MoveLocal(false), 
	m_MovementTime(), 
	m_Translation()
{
}

C_ActorMovementTranslateMsg::C_ActorMovementTranslateMsg(const Maths::C_Vector& translation, const Maths::C_TimeS& movementTime, U32 movementFlags)	: 
	m_MovementFlags(movementFlags), 
	m_MoveLocal(false), 
	m_MovementTime(movementTime), 
	m_Translation(translation)
{
}

C_ActorMovementTranslateMsg::~C_ActorMovementTranslateMsg()
{
}

void C_ActorMovementTranslateMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_ActorMovementTranslateMsg* pMsg = Messaging::MessageCast<C_ActorMovementTranslateMsg>(pMessage);

	stream << pMsg->m_MovementFlags;
	stream << pMsg->m_MoveLocal;
	stream << pMsg->m_MovementTime;
	stream << pMsg->m_Translation;

	stream << pMsg->m_NotificationHandle;
}

void C_ActorMovementTranslateMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_ActorMovementTranslateMsg* pMsg = reinterpret_cast<C_ActorMovementTranslateMsg*>(pMessage);

	stream >> pMsg->m_MovementFlags;
	stream >> pMsg->m_MoveLocal;
	stream >> pMsg->m_MovementTime;
	stream >> pMsg->m_Translation;

	stream >> pMsg->m_NotificationHandle;
}




Messaging::C_MessageSerialiser C_ActorMovementRotateMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_ActorMovementRotateMsg::C_ActorMovementRotateMsg() :
	m_Degrees(0.0f), 
	m_MovementFlags(0), 
	m_MoveLocal(false), 
	m_MovementTime(), 
	m_Axis()
{
}

C_ActorMovementRotateMsg::C_ActorMovementRotateMsg(F32 degrees, const Maths::C_Normal& axis, const Maths::C_TimeS& movementTime, U32 movementFlags)	: 
	m_Degrees(degrees), 
	m_MovementFlags(movementFlags), 
	m_MoveLocal(false), 
	m_MovementTime(movementTime), 
	m_Axis(axis)
{
}

C_ActorMovementRotateMsg::~C_ActorMovementRotateMsg()
{
}

void C_ActorMovementRotateMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_ActorMovementRotateMsg* pMsg = Messaging::MessageCast<C_ActorMovementRotateMsg>(pMessage);

	stream << pMsg->m_Degrees;
	stream << pMsg->m_MovementFlags;
	stream << pMsg->m_MoveLocal;
	stream << pMsg->m_MovementTime;
	stream << pMsg->m_Axis;

	stream << pMsg->m_NotificationHandle;
}

void C_ActorMovementRotateMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_ActorMovementRotateMsg* pMsg = reinterpret_cast<C_ActorMovementRotateMsg*>(pMessage);

	stream >> pMsg->m_Degrees;
	stream >> pMsg->m_MovementFlags;
	stream >> pMsg->m_MoveLocal;
	stream >> pMsg->m_MovementTime;
	stream >> pMsg->m_Axis;

	stream >> pMsg->m_NotificationHandle;
}


Messaging::C_MessageSerialiser C_ActorMovementSpinAxisYMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_ActorMovementSpinAxisYMsg::C_ActorMovementSpinAxisYMsg() : 
	m_RevsPerMinute(0.0f), 
	m_SpinupTime()
{
}

C_ActorMovementSpinAxisYMsg::C_ActorMovementSpinAxisYMsg(F32 revsPerMinute, const Maths::C_TimeS& spinupTime) : 
	m_RevsPerMinute(revsPerMinute), 
	m_SpinupTime(spinupTime)
{
}

C_ActorMovementSpinAxisYMsg::~C_ActorMovementSpinAxisYMsg()
{
}

void C_ActorMovementSpinAxisYMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_ActorMovementSpinAxisYMsg* pMsg = Messaging::MessageCast<C_ActorMovementSpinAxisYMsg>(pMessage);

	stream << pMsg->m_RevsPerMinute;
	stream << pMsg->m_SpinupTime;

	stream << pMsg->m_NotificationHandle;
}

void C_ActorMovementSpinAxisYMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_ActorMovementSpinAxisYMsg* pMsg = reinterpret_cast<C_ActorMovementSpinAxisYMsg*>(pMessage);

	stream >> pMsg->m_RevsPerMinute;
	stream >> pMsg->m_SpinupTime;

	stream >> pMsg->m_NotificationHandle;
}




Messaging::C_MessageSerialiser C_ActorMovementAimAtMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_ActorMovementAimAtMsg::C_ActorMovementAimAtMsg() :
	m_Time(0.0f), 
	m_Position()
{
}

C_ActorMovementAimAtMsg::~C_ActorMovementAimAtMsg()
{
}

void C_ActorMovementAimAtMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_ActorMovementAimAtMsg* pMsg = Messaging::MessageCast<C_ActorMovementAimAtMsg>(pMessage);

	stream << pMsg->m_Time;
	stream << pMsg->m_Position;
}

void C_ActorMovementAimAtMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_ActorMovementAimAtMsg* pMsg = reinterpret_cast<C_ActorMovementAimAtMsg*>(pMessage);

	stream >> pMsg->m_Time;
	stream >> pMsg->m_Position;
}





Messaging::C_MessageSerialiser C_ActorMovementAimBetweenMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_ActorMovementAimBetweenMsg::C_ActorMovementAimBetweenMsg() : 
	m_Time(0.0f), 
	m_Pos1(), 
	m_Pos2()
{
}

C_ActorMovementAimBetweenMsg::~C_ActorMovementAimBetweenMsg()
{
}

void C_ActorMovementAimBetweenMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_ActorMovementAimBetweenMsg* pMsg = Messaging::MessageCast<C_ActorMovementAimBetweenMsg>(pMessage);

	stream << pMsg->m_Time;
	stream << pMsg->m_Pos1;
	stream << pMsg->m_Pos2;
}

void C_ActorMovementAimBetweenMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_ActorMovementAimBetweenMsg* pMsg = reinterpret_cast<C_ActorMovementAimBetweenMsg*>(pMessage);

	stream >> pMsg->m_Time;
	stream >> pMsg->m_Pos1;
	stream >> pMsg->m_Pos2;
}





Messaging::C_MessageSerialiser C_ActorMovementTrackMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_ActorMovementTrackMsg::C_ActorMovementTrackMsg() :
	m_ActorHandle(), 
	m_LocalOffset(false), 
	m_OffsetPos(),
	m_UseRetrackingSpeed(false),
	m_RetrackingSpeed(1.0f),
	m_ReplaceDefaultSpeed(false)
{
}

C_ActorMovementTrackMsg::~C_ActorMovementTrackMsg()
{
}

void C_ActorMovementTrackMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_ActorMovementTrackMsg* pMsg = Messaging::MessageCast<C_ActorMovementTrackMsg>(pMessage);

	stream << pMsg->m_ActorHandle;
	stream << pMsg->m_LocalOffset;
	stream << pMsg->m_OffsetPos;
	stream << pMsg->m_UseRetrackingSpeed;
	stream << pMsg->m_RetrackingSpeed;
	stream << pMsg->m_ReplaceDefaultSpeed;
}

void C_ActorMovementTrackMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_ActorMovementTrackMsg* pMsg = reinterpret_cast<C_ActorMovementTrackMsg*>(pMessage);

	stream >> pMsg->m_ActorHandle;
	stream >> pMsg->m_LocalOffset;
	stream >> pMsg->m_OffsetPos;
	stream >> pMsg->m_UseRetrackingSpeed;
	stream >> pMsg->m_ReplaceDefaultSpeed;
}





Messaging::C_MessageSerialiser C_ActorMovementTrackLagMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_ActorMovementTrackLagMsg::C_ActorMovementTrackLagMsg() :
	m_ActorHandle(), 
	m_LocalOffset(false), 
	m_OffsetPos()
{
}

C_ActorMovementTrackLagMsg::~C_ActorMovementTrackLagMsg()
{
}

void C_ActorMovementTrackLagMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_ActorMovementTrackLagMsg* pMsg = Messaging::MessageCast<C_ActorMovementTrackLagMsg>(pMessage);

	stream << pMsg->m_ActorHandle;
	stream << pMsg->m_LocalOffset;
	stream << pMsg->m_OffsetPos;
}

void C_ActorMovementTrackLagMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_ActorMovementTrackLagMsg* pMsg = reinterpret_cast<C_ActorMovementTrackLagMsg*>(pMessage);

	stream >> pMsg->m_ActorHandle;
	stream >> pMsg->m_LocalOffset;
	stream >> pMsg->m_OffsetPos;
}





Messaging::C_MessageSerialiser C_ActorMovementTrackPredictionMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_ActorMovementTrackPredictionMsg::C_ActorMovementTrackPredictionMsg() :
	m_ActorHandle(), 
	m_LocalOffset(false), 
	m_Time(0.0f),
	m_OffsetPos()
{
}

C_ActorMovementTrackPredictionMsg::~C_ActorMovementTrackPredictionMsg()
{
}

void C_ActorMovementTrackPredictionMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_ActorMovementTrackPredictionMsg* pMsg = Messaging::MessageCast<C_ActorMovementTrackPredictionMsg>(pMessage);

	stream << pMsg->m_ActorHandle;
	stream << pMsg->m_LocalOffset;
	stream << pMsg->m_Time;
	stream << pMsg->m_OffsetPos;
}

void C_ActorMovementTrackPredictionMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_ActorMovementTrackPredictionMsg* pMsg = reinterpret_cast<C_ActorMovementTrackPredictionMsg*>(pMessage);

	stream >> pMsg->m_ActorHandle;
	stream >> pMsg->m_LocalOffset;
	stream >> pMsg->m_Time;
	stream >> pMsg->m_OffsetPos;
}





Messaging::C_MessageSerialiser C_ActorMovementMoveAndAlignToMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_ActorMovementMoveAndAlignToMsg::C_ActorMovementMoveAndAlignToMsg() : 
	m_MovementFlags(0), 
	m_MovementTime(),
	m_QuatTrans()
{
}

C_ActorMovementMoveAndAlignToMsg::C_ActorMovementMoveAndAlignToMsg(const Maths::C_QuatTrans& quatTrans, const Maths::C_TimeS& movementTime, U32 movementFlags) : 
	m_MovementFlags(movementFlags), 
	m_MovementTime(movementTime),
	m_QuatTrans(quatTrans)
{
}

C_ActorMovementMoveAndAlignToMsg::~C_ActorMovementMoveAndAlignToMsg()
{
}

void C_ActorMovementMoveAndAlignToMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_ActorMovementMoveAndAlignToMsg* pMsg = Messaging::MessageCast<C_ActorMovementMoveAndAlignToMsg>(pMessage);

	stream << pMsg->m_MovementFlags;
	stream << pMsg->m_MovementTime;
	stream << pMsg->m_QuatTrans;
}

void C_ActorMovementMoveAndAlignToMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_ActorMovementMoveAndAlignToMsg* pMsg = reinterpret_cast<C_ActorMovementMoveAndAlignToMsg*>(pMessage);

	stream >> pMsg->m_MovementFlags;
	stream >> pMsg->m_MovementTime;
	stream >> pMsg->m_QuatTrans;
}





Messaging::C_MessageSerialiser C_ActorMovementMoveAndAlignToActorMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_ActorMovementMoveAndAlignToActorMsg::C_ActorMovementMoveAndAlignToActorMsg() : 
	m_ActorHandle(), 
	m_MovementFlags(0),
	m_MovementTime()
{
}

C_ActorMovementMoveAndAlignToActorMsg::~C_ActorMovementMoveAndAlignToActorMsg()
{
}

void C_ActorMovementMoveAndAlignToActorMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_ActorMovementMoveAndAlignToActorMsg* pMsg = Messaging::MessageCast<C_ActorMovementMoveAndAlignToActorMsg>(pMessage);

	stream << pMsg->m_ActorHandle;
	stream << pMsg->m_MovementFlags;
	stream << pMsg->m_MovementTime;

	stream << pMsg->m_NotificationHandle;
}

void C_ActorMovementMoveAndAlignToActorMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_ActorMovementMoveAndAlignToActorMsg* pMsg = reinterpret_cast<C_ActorMovementMoveAndAlignToActorMsg*>(pMessage);

	stream >> pMsg->m_ActorHandle;
	stream >> pMsg->m_MovementFlags;
	stream >> pMsg->m_MovementTime;

	stream >> pMsg->m_NotificationHandle;
}




Messaging::C_MessageSerialiser C_ActorMovementSplineMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_ActorMovementSplineMsg::C_ActorMovementSplineMsg() :
	m_ActorHandle(),
	m_MoveSpeed(0.0f),
	m_Rotation(0.0f),
	m_Loop(false)
{
}

C_ActorMovementSplineMsg::~C_ActorMovementSplineMsg()
{
}

void C_ActorMovementSplineMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_ActorMovementSplineMsg* pMsg = Messaging::MessageCast<C_ActorMovementSplineMsg>(pMessage);

	stream << pMsg->m_ActorHandle;
	stream << pMsg->m_MoveSpeed;
	stream << pMsg->m_Rotation;
	stream << pMsg->m_Loop;

	stream << pMsg->m_NotificationHandle;
}

void C_ActorMovementSplineMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_ActorMovementSplineMsg* pMsg = reinterpret_cast<C_ActorMovementSplineMsg*>(pMessage);

	stream >> pMsg->m_ActorHandle;
	stream >> pMsg->m_MoveSpeed;
	stream >> pMsg->m_Rotation;
	stream >> pMsg->m_Loop;

	stream >> pMsg->m_NotificationHandle;
}

Messaging::C_MessageSerialiser C_ActorMovementKinematicSplineMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_ActorMovementKinematicSplineMsg::C_ActorMovementKinematicSplineMsg() :
m_ActorHandle(),
m_StartSpeed(0.0f),
m_MinSpeed(0.0f),
m_MaxSpeed(0.0f),
m_Acceleration(0.0f),
m_BrakeAtEnd(false),
m_Loop(false),
m_StartFromNearestPosition(false)
{
}

C_ActorMovementKinematicSplineMsg::~C_ActorMovementKinematicSplineMsg()
{
}

void C_ActorMovementKinematicSplineMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_ActorMovementKinematicSplineMsg* pMsg = Messaging::MessageCast<C_ActorMovementKinematicSplineMsg>(pMessage);

	stream << pMsg->m_ActorHandle;
	stream << pMsg->m_StartSpeed;
	stream << pMsg->m_MinSpeed;
	stream << pMsg->m_MaxSpeed;
	stream << pMsg->m_Acceleration;
	stream << pMsg->m_BrakeAtEnd;
	stream << pMsg->m_Loop;
	stream << pMsg->m_StartFromNearestPosition;
	stream << pMsg->m_NotificationHandle;
}

void C_ActorMovementKinematicSplineMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_ActorMovementKinematicSplineMsg* pMsg = reinterpret_cast<C_ActorMovementKinematicSplineMsg*>(pMessage);

	stream >> pMsg->m_ActorHandle;
	stream >> pMsg->m_StartSpeed;
	stream >> pMsg->m_MinSpeed;
	stream >> pMsg->m_MaxSpeed;
	stream >> pMsg->m_Acceleration;
	stream >> pMsg->m_BrakeAtEnd;
	stream >> pMsg->m_Loop;
	stream >> pMsg->m_StartFromNearestPosition;
	stream >> pMsg->m_NotificationHandle;
}

Messaging::C_MessageSerialiser C_ActorMovementKinematicSplineTrackerMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_ActorMovementKinematicSplineTrackerMsg::C_ActorMovementKinematicSplineTrackerMsg() :
m_TrackerActorHandle(),
m_MinSpeed(0.0f),
m_MaxSpeed(0.0f),
m_Acceleration(0.0f),
m_IdealDistance(0.0f)
{
}

C_ActorMovementKinematicSplineTrackerMsg::~C_ActorMovementKinematicSplineTrackerMsg()
{
}

void C_ActorMovementKinematicSplineTrackerMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_ActorMovementKinematicSplineTrackerMsg* pMsg = Messaging::MessageCast<C_ActorMovementKinematicSplineTrackerMsg>(pMessage);

	stream << pMsg->m_TrackerActorHandle;
	stream << pMsg->m_MinSpeed;
	stream << pMsg->m_MaxSpeed;
	stream << pMsg->m_Acceleration;
	stream << pMsg->m_IdealDistance;
	stream << pMsg->m_NotificationHandle;
}

void C_ActorMovementKinematicSplineTrackerMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_ActorMovementKinematicSplineTrackerMsg* pMsg = reinterpret_cast<C_ActorMovementKinematicSplineTrackerMsg*>(pMessage);

	stream >> pMsg->m_TrackerActorHandle;
	stream >> pMsg->m_MinSpeed;
	stream >> pMsg->m_MaxSpeed;
	stream >> pMsg->m_Acceleration;
	stream >> pMsg->m_IdealDistance;
	stream >> pMsg->m_NotificationHandle;
}

Messaging::C_MessageSerialiser C_ActorMovementKinematicSplineCommandMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_ActorMovementKinematicSplineCommandMsg::C_ActorMovementKinematicSplineCommandMsg() :
m_MinSpeed(0.0f),
m_MaxSpeed(0.0f),
m_Acceleration(0.0f)
{
}

C_ActorMovementKinematicSplineCommandMsg::~C_ActorMovementKinematicSplineCommandMsg()
{
}

void C_ActorMovementKinematicSplineCommandMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_ActorMovementKinematicSplineCommandMsg* pMsg = Messaging::MessageCast<C_ActorMovementKinematicSplineCommandMsg>(pMessage);

	stream << pMsg->m_MinSpeed;
	stream << pMsg->m_MaxSpeed;
	stream << pMsg->m_Acceleration;
}

void C_ActorMovementKinematicSplineCommandMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_ActorMovementKinematicSplineCommandMsg* pMsg = reinterpret_cast<C_ActorMovementKinematicSplineCommandMsg*>(pMessage);

	stream >> pMsg->m_MinSpeed;
	stream >> pMsg->m_MaxSpeed;
	stream >> pMsg->m_Acceleration;
	
}


Messaging::C_MessageSerialiser C_ActorMovementConstrainMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

void C_ActorMovementConstrainMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_ActorMovementConstrainMsg* pMsg = Messaging::MessageCast<C_ActorMovementConstrainMsg>(pMessage);
	stream << pMsg->m_ConstrainXTrans;
	stream << pMsg->m_ConstrainYTrans;
	stream << pMsg->m_ConstrainZTrans;
	stream << pMsg->m_ConstrainHeading;
	stream << pMsg->m_ConstrainPitch;
	stream << pMsg->m_ConstrainRoll;
}


void C_ActorMovementConstrainMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_ActorMovementConstrainMsg* pMsg = reinterpret_cast<C_ActorMovementConstrainMsg*>(pMessage);
	stream >> pMsg->m_ConstrainXTrans;
	stream >> pMsg->m_ConstrainYTrans;
	stream >> pMsg->m_ConstrainZTrans;
	stream >> pMsg->m_ConstrainHeading;
	stream >> pMsg->m_ConstrainPitch;
	stream >> pMsg->m_ConstrainRoll;
}


// Xt Message Types
// ----------------

// C_ActorMovementTranslateMsg
class C_XtActorMovementTranslateMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementTranslateMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	Actors::C_XtParamVector	*		m_VectorParam;
	Actors::C_XtParamFloat *		m_Time;
	Actors::C_XtParamFloat *		m_Speed;
	EMovementType					m_MovementType;
	bool							m_MoveLocal;

	C_ActorMovementTranslateMsg		m_Msg;
};


const Messaging::C_MessageBase *	C_XtActorMovementTranslateMsg::GetMessage(const Actor::C_Actor *pActorContext)   
{ 
	assert(pActorContext);
	assert(m_VectorParam);
	assert(m_Time || m_Speed);
	m_Msg.SetTranslation(m_VectorParam->GetVector(pActorContext));
	if (m_Speed && m_Speed->GetFloat(pActorContext)>0.0f)
		m_Msg.SetMovementTime(m_Msg.Translation().Length() / m_Speed->GetFloat(pActorContext));
	else
		m_Msg.SetMovementTime(m_Time ? m_Time->GetFloat(pActorContext) : 0.0f);
	m_Msg.m_MovementFlags = MovementFlagsFromType(m_MovementType);	
	m_Msg.m_MoveLocal = m_MoveLocal;// ? m_MoveLocal->GetBool(pActorContext) : false;
	m_Msg.SetNotifier(pActorContext->GetHandle());
	return &m_Msg;
}

// C_ActorMovementRotateMsg
class C_XtActorMovementRotateMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementRotateMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);

	//data
	Actors::C_XtParamVector *		m_Axis;
	Actors::C_XtParamFloat *		m_Degrees;
	Actors::C_XtParamFloat *		m_Time;
	EMovementType					m_MovementType;
	bool							m_MoveLocal;

	C_ActorMovementRotateMsg		m_Msg;
};

const Messaging::C_MessageBase *	C_XtActorMovementRotateMsg::GetMessage(const Actor::C_Actor *pActorContext)   
{ 
	assert(pActorContext);
	assert(m_Degrees);
	assert(m_Time);
	m_Msg.SetAxis(Maths::C_Normal(m_Axis->GetVector(pActorContext)));
	m_Msg.SetMovementTime(m_Time->GetFloat(pActorContext));
	m_Msg.m_Degrees = m_Degrees->GetFloat(pActorContext);
	m_Msg.m_MovementFlags = MovementFlagsFromType(m_MovementType);
	m_Msg.m_MoveLocal = m_MoveLocal;// ? m_MoveLocal->GetBool(pActorContext) : false;
	m_Msg.SetNotifier(pActorContext->GetHandle());
	return &m_Msg;
}

// C_ActorMovementRotateMsg
class C_XtActorMovementRotateXMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementRotateXMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);

	Actors::C_XtParamFloat *		m_Degrees;
	Actors::C_XtParamFloat *		m_Time;
	EMovementType					m_MovementType;
	bool							m_MoveLocal;

	C_ActorMovementRotateMsg		m_Msg;
};
const Messaging::C_MessageBase *	C_XtActorMovementRotateXMsg::GetMessage(const Actor::C_Actor *pActorContext)   
{ 
	assert(pActorContext);
	assert(m_Degrees);
	assert(m_Time);
	m_Msg.SetAxis(Maths::C_Normal(1.0f,0,0));
	m_Msg.SetMovementTime(m_Time->GetFloat(pActorContext));
	m_Msg.m_Degrees = m_Degrees->GetFloat(pActorContext);
	m_Msg.m_MovementFlags = MovementFlagsFromType(m_MovementType);	
	m_Msg.m_MoveLocal = m_MoveLocal;// ? m_MoveLocal->GetBool(pActorContext) : false;
	m_Msg.SetNotifier(pActorContext->GetHandle());
	return &m_Msg;
}

// C_ActorMovementRotateMsg
class C_XtActorMovementRotateYMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementRotateYMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);

	Actors::C_XtParamFloat *		m_Degrees;
	Actors::C_XtParamFloat *		m_Time;
	EMovementType					m_MovementType;
	bool							m_MoveLocal;

	C_ActorMovementRotateMsg		m_Msg;
};
const Messaging::C_MessageBase *	C_XtActorMovementRotateYMsg::GetMessage(const Actor::C_Actor *pActorContext)   
{ 
	assert(pActorContext);
	assert(m_Degrees);
	assert(m_Time);
	m_Msg.SetAxis(Maths::C_Normal(0,1.0f,0));
	m_Msg.SetMovementTime(m_Time->GetFloat(pActorContext));
	m_Msg.m_Degrees = m_Degrees->GetFloat(pActorContext);
	m_Msg.m_MovementFlags = MovementFlagsFromType(m_MovementType);	
	m_Msg.m_MoveLocal = m_MoveLocal;// ? m_MoveLocal->GetBool(pActorContext) : false;
	m_Msg.SetNotifier(pActorContext->GetHandle());
	return &m_Msg;
}

// C_ActorMovementRotateMsg
class C_XtActorMovementRotateZMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementRotateZMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);

	Actors::C_XtParamFloat *		m_Degrees;
	Actors::C_XtParamFloat *		m_Time;
	EMovementType					m_MovementType;
	bool							m_MoveLocal;

	C_ActorMovementRotateMsg		m_Msg;
};

const Messaging::C_MessageBase *	C_XtActorMovementRotateZMsg::GetMessage(const Actor::C_Actor *pActorContext)   
{ 
	assert(pActorContext);
	assert(m_Degrees);
	assert(m_Time);
	m_Msg.SetAxis(Maths::C_Normal(0,0,1.0f));
	m_Msg.SetMovementTime(m_Time->GetFloat(pActorContext));
	m_Msg.m_Degrees = m_Degrees->GetFloat(pActorContext);
	m_Msg.m_MovementFlags = MovementFlagsFromType(m_MovementType);	
	m_Msg.m_MoveLocal = m_MoveLocal;// ? m_MoveLocal->GetBool(pActorContext) : false;
	m_Msg.SetNotifier(pActorContext->GetHandle());
	return &m_Msg;
}

// C_ActorMovementSpinAxisYMsg
class C_XtActorMovementSpinAxisYMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementSpinAxisYMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);

	Actors::C_XtParamFloat *				m_RPM;
	Actors::C_XtParamFloat *				m_Time;
	EMovementType					m_MovementType;

	C_ActorMovementSpinAxisYMsg		m_Msg;
};

const Messaging::C_MessageBase *	C_XtActorMovementSpinAxisYMsg::GetMessage(const Actor::C_Actor *pActorContext)   
{ 
	assert(pActorContext);
	assert(m_RPM);
	assert(m_Time);
	m_Msg.m_RevsPerMinute = m_RPM->GetFloat(pActorContext);
	m_Msg.SetSpinUpTime(m_Time->GetFloat(pActorContext));
	m_Msg.SetNotifier(pActorContext->GetHandle());
	return &m_Msg;
}

// C_ActorMovementSpinStopMsg
class C_XtActorMovementSpinStopMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementSpinStopMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);

	Actors::C_XtParamFloat *				m_StopTime;
	EMovementType					m_MovementType;

	C_ActorMovementSpinStopMsg		m_Msg;
};

const Messaging::C_MessageBase *	C_XtActorMovementSpinStopMsg::GetMessage(const Actor::C_Actor *pActorContext)   
{ 
	assert(pActorContext);
	assert(m_StopTime);
	m_Msg.SetTime(m_StopTime->GetFloat(pActorContext));
	m_Msg.SetNotifier(pActorContext->GetHandle());
	return &m_Msg;
}

//C_ActorMovementMoveAndAlignToActorMsg
class C_XtActorMovementMoveAndAlignToMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementMoveAndAlignToMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	Actors::C_XtParamActor *				m_Actor;
	Actors::C_XtParamFloat *				m_Time;
	EMovementType					m_MovementType;

	C_ActorMovementMoveAndAlignToActorMsg		m_Msg;
};

const Messaging::C_MessageBase *	C_XtActorMovementMoveAndAlignToMsg::GetMessage(const Actor::C_Actor *pActorContext)   
{ 
	assert(pActorContext);
	assert(m_Actor);
	assert(m_Time);
	m_Msg.m_MovementFlags = MovementFlagsFromType(m_MovementType);	
	m_Msg.m_ActorHandle = m_Actor->GetActorHandle(pActorContext);
	m_Msg.SetMovementTime(m_Time->GetFloat(pActorContext));
	m_Msg.SetNotifier(pActorContext->GetHandle());
	return &m_Msg;
}

// C_ActorMovementSplineMsg
class C_XtActorMovementSplineMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementSplineMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	Actors::C_XtParamActor *				m_SplineActor;
	Actors::C_XtParamFloat *				m_MoveSpeed;
	Actors::C_XtParamFloat *				m_Rotation;
	Actors::C_XtParamBool *					m_Loop;
	EMovementType							m_MovementType;	
	C_ActorMovementSplineMsg		m_Msg;
};

const Messaging::C_MessageBase *	C_XtActorMovementSplineMsg::GetMessage(const Actor::C_Actor *pActorContext)   
{ 
	assert(pActorContext);
	assert(m_SplineActor);
	assert(m_MoveSpeed);
	
	
	m_Msg.m_ActorHandle = m_SplineActor->GetActorHandle(pActorContext);
	m_Msg.SetNotifier(pActorContext->GetHandle());
	m_Msg.m_MoveSpeed = m_MoveSpeed->GetFloat(pActorContext);
	m_Msg.m_MovementFlags = MovementFlagsFromType(m_MovementType);	
	m_Loop ? m_Msg.m_Loop = m_Loop->GetBool(pActorContext) : m_Msg.m_Loop = false;
	m_Rotation ? m_Msg.m_Rotation = m_Rotation->GetFloat(pActorContext): m_Msg.m_Rotation = 0.0f;
	return &m_Msg;
}

// C_ActorMovementKinematicSplineMsg
class C_XtActorMovementKinematicSplineMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementKinematicSplineMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	Actors::C_XtParamActor *				m_SplineActor;
	Actors::C_XtParamFloat *				m_StartSpeed;
	Actors::C_XtParamFloat *				m_MinSpeed;
	Actors::C_XtParamFloat *				m_MaxSpeed;
	Actors::C_XtParamFloat *				m_Acceleration;
	bool									m_BrakeAtEnd;
	bool									m_Loop;
	bool									m_StartFromNearestPosition;
	
	C_ActorMovementKinematicSplineMsg		m_Msg;
};

const Messaging::C_MessageBase *	C_XtActorMovementKinematicSplineMsg::GetMessage(const Actor::C_Actor *pActorContext)   
{ 
	assert(pActorContext);
	assert(m_SplineActor);
	
	m_Msg.m_ActorHandle = m_SplineActor->GetActorHandle(pActorContext);
	m_Msg.SetNotifier(pActorContext->GetHandle());

	m_StartSpeed ? m_Msg.m_StartSpeed = m_StartSpeed->GetFloat(pActorContext) : m_Msg.m_StartSpeed = 0.0f;
	m_MinSpeed ? m_Msg.m_MinSpeed = m_MinSpeed->GetFloat(pActorContext) : m_Msg.m_MinSpeed = 0.0f;
	m_MaxSpeed ? m_Msg.m_MaxSpeed = m_MaxSpeed->GetFloat(pActorContext) : m_Msg.m_MaxSpeed = 20.0f;
	m_Acceleration ? m_Msg.m_Acceleration  = m_Acceleration ->GetFloat(pActorContext) : m_Msg.m_Acceleration  = 5.0f;
	m_Msg.m_BrakeAtEnd = m_BrakeAtEnd;
	m_Msg.m_Loop = m_Loop;
	m_Msg.m_StartFromNearestPosition = m_StartFromNearestPosition;
	return &m_Msg;
}

// C_ActorMovementKinematicSplineTrackerMsg
class C_XtActorMovementKinematicSplineTrackerMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementKinematicSplineTrackerMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	Actors::C_XtParamActor *				m_TrackerActor;
	Actors::C_XtParamFloat *				m_MinSpeed;
	Actors::C_XtParamFloat *				m_MaxSpeed;
	Actors::C_XtParamFloat *				m_Acceleration;
	Actors::C_XtParamFloat *				m_IdealDistance;
	
	C_ActorMovementKinematicSplineTrackerMsg		m_Msg;
};

const Messaging::C_MessageBase *	C_XtActorMovementKinematicSplineTrackerMsg::GetMessage(const Actor::C_Actor *pActorContext)   
{ 
	assert(pActorContext);
	assert(m_TrackerActor);

	m_Msg.m_TrackerActorHandle = m_TrackerActor->GetActorHandle(pActorContext);
	m_Msg.SetNotifier(pActorContext->GetHandle());

	m_MinSpeed ? m_Msg.m_MinSpeed = m_MinSpeed->GetFloat(pActorContext) : m_Msg.m_MinSpeed = 0.0f;
	m_MaxSpeed ? m_Msg.m_MaxSpeed = m_MaxSpeed->GetFloat(pActorContext) : m_Msg.m_MaxSpeed = 20.0f;
	m_Acceleration ? m_Msg.m_Acceleration  = m_Acceleration ->GetFloat(pActorContext) : m_Msg.m_Acceleration  = 5.0f;
	m_IdealDistance ? m_Msg.m_IdealDistance  = m_IdealDistance ->GetFloat(pActorContext) : m_Msg.m_IdealDistance  = 0.0f;
	return &m_Msg;
}

// C_ActorMovementKinematicSplineCommandMsg
class C_XtActorMovementKinematicSplineCommandMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementKinematicSplineCommandMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	
	Actors::C_XtParamFloat *				m_MinSpeed;
	Actors::C_XtParamFloat *				m_MaxSpeed;
	Actors::C_XtParamFloat *				m_Acceleration;
	
	C_ActorMovementKinematicSplineCommandMsg		m_Msg;
};

const Messaging::C_MessageBase *	C_XtActorMovementKinematicSplineCommandMsg::GetMessage(const Actor::C_Actor *pActorContext)   
{ 
	assert(pActorContext);

	m_MinSpeed ? m_Msg.m_MinSpeed = m_MinSpeed->GetFloat(pActorContext) : m_Msg.m_MinSpeed = 0.0f;
	m_MaxSpeed ? m_Msg.m_MaxSpeed = m_MaxSpeed->GetFloat(pActorContext) : m_Msg.m_MaxSpeed = 20.0f;
	m_Acceleration ? m_Msg.m_Acceleration  = m_Acceleration ->GetFloat(pActorContext) : m_Msg.m_Acceleration  = 5.0f;
	
	return &m_Msg;
}

// C_ActorMovementTrackMsg
class C_XtActorMovementTrackMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementTrackMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	Actors::C_XtParamActor *				m_Actor;
	Actors::C_XtParamPosition *				m_OffsetPos;
	bool									m_LocalOffset;
	bool									m_UseRetrackingSpeed;
	F32										m_RetrackingSpeed;
	bool									m_ReplaceDefaultSpeed;
	C_ActorMovementTrackMsg			m_Msg;
};

const Messaging::C_MessageBase * C_XtActorMovementTrackMsg::GetMessage(const Actor::C_Actor *pActorContext)
{ 
	assert(pActorContext);
	assert(m_Actor);
	m_Msg.m_ActorHandle = m_Actor->GetActorHandle(pActorContext);
	m_Msg.SetOffsetPosition(m_OffsetPos ? m_OffsetPos->GetPosition(pActorContext) : Maths::C_Vertex(0.0f, 0.0f, 0.0f));
	m_Msg.m_LocalOffset = m_LocalOffset;
	m_Msg.m_UseRetrackingSpeed = m_UseRetrackingSpeed;
	m_Msg.m_RetrackingSpeed = m_RetrackingSpeed;
	m_Msg.m_ReplaceDefaultSpeed = m_ReplaceDefaultSpeed;
	return &m_Msg;
}

// C_ActorMovementSetParentMsg
class C_XtActorMovementSetParentMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	C_XtActorMovementSetParentMsg(){}
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext);

	Actors::C_XtParamActor *		m_ParentActor;

	C_ActorMovementSetParentMsg		m_Msg;
};

const Messaging::C_MessageBase* C_XtActorMovementSetParentMsg::GetMessage(const Actor::C_Actor *pActorContext) 
{ 
	assert(pActorContext);
	assert(m_ParentActor);
	m_Msg.SetActorHandle(m_ParentActor->GetActorHandle(pActorContext));
	return &m_Msg;
}

// C_ActorMovementAimAtMsg
class C_XtActorMovementAimAtMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementAimAtMsg(){}
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext);

	Actors::C_XtParamPosition *				m_Position;
	Actors::C_XtParamFloat *				m_Time;
	C_ActorMovementAimAtMsg			m_Msg;
};

const Messaging::C_MessageBase* C_XtActorMovementAimAtMsg::GetMessage(const Actor::C_Actor *pActorContext)   
{ 
	assert(pActorContext);
	assert(m_Position);
	assert(m_Time);
	m_Msg.SetPosition(m_Position->GetPosition(pActorContext));
	m_Msg.m_Time = m_Time->GetFloat(pActorContext);
	return &m_Msg;
}

// C_ActorMovementAimBetweenMsg
class C_XtActorMovementAimBetweenMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementAimBetweenMsg(){}
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext);

	Actors::C_XtParamPosition *				m_Position1;
	Actors::C_XtParamPosition *				m_Position2;
	Actors::C_XtParamFloat *				m_Time;
	C_ActorMovementAimBetweenMsg	m_Msg;
};

const Messaging::C_MessageBase* C_XtActorMovementAimBetweenMsg::GetMessage(const Actor::C_Actor *pActorContext)   
{ 
	assert(pActorContext);
	assert(m_Position1);
	assert(m_Position2);
	assert(m_Time);
	m_Msg.SetPosition1(m_Position1->GetPosition(pActorContext));
	m_Msg.SetPosition2(m_Position2->GetPosition(pActorContext));
	m_Msg.m_Time = m_Time->GetFloat(pActorContext);
	return &m_Msg;
}

// C_ActorMovementTrackLagMsg
class C_XtActorMovementTrackLagMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementTrackLagMsg(){}
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext);

	Actors::C_XtParamActor *				m_Actor;
	Actors::C_XtParamPosition *				m_OffsetPos;
	bool									m_LocalOffset;
	C_ActorMovementTrackLagMsg		m_Msg;
};

const Messaging::C_MessageBase* C_XtActorMovementTrackLagMsg::GetMessage(const Actor::C_Actor *pActorContext)   
{ 
	assert(pActorContext);
	assert(m_Actor);
	m_Msg.m_ActorHandle = m_Actor->GetActorHandle(pActorContext);
	m_Msg.SetOffsetPosition(m_OffsetPos ? m_OffsetPos->GetPosition(pActorContext) : Maths::C_Vertex(0.0f, 0.0f, 0.0f));
	m_Msg.m_LocalOffset = m_LocalOffset;
	return &m_Msg;
}

// C_ActorMovementTrackPredictionMsg
class C_XtActorMovementTrackPredictionMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementTrackPredictionMsg(){}
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext);

	Actors::C_XtParamActor *				m_Actor;
	Actors::C_XtParamFloat *				m_Time;
	Actors::C_XtParamPosition *				m_OffsetPos;
	bool									m_LocalOffset;

	C_ActorMovementTrackPredictionMsg		m_Msg;
};

const Messaging::C_MessageBase* C_XtActorMovementTrackPredictionMsg::GetMessage(const Actor::C_Actor *pActorContext)   
{ 
	assert(pActorContext);
	assert(m_Actor);
	assert(m_Time);
	m_Msg.m_ActorHandle = m_Actor->GetActorHandle(pActorContext);
	m_Msg.m_Time = m_Time->GetFloat(pActorContext);
	m_Msg.SetOffsetPosition(m_OffsetPos ? m_OffsetPos->GetPosition(pActorContext) : Maths::C_Vertex(0.0f, 0.0f, 0.0f));
	m_Msg.m_LocalOffset = m_LocalOffset;
	return &m_Msg;
}



// C_ActorMovementConstrainMsg
class C_XtActorMovementConstrainMsg : public C_XtActorMovementMsgBase
{
public:
	C_XtActorMovementConstrainMsg(){}
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext)  
	{
		assert(pActorContext);

		m_Msg.m_ConstrainXTrans = m_ConstrainXTrans;
		m_Msg.m_ConstrainYTrans = m_ConstrainYTrans;
		m_Msg.m_ConstrainZTrans = m_ConstrainZTrans;
		m_Msg.m_ConstrainHeading = m_ConstrainHeading;
		m_Msg.m_ConstrainPitch = m_ConstrainPitch;
		m_Msg.m_ConstrainRoll = m_ConstrainRoll;

		return &m_Msg;
	}

	bool m_ConstrainXTrans;
	bool m_ConstrainYTrans;
	bool m_ConstrainZTrans;
	bool m_ConstrainHeading;
	bool m_ConstrainPitch;
	bool m_ConstrainRoll;

	C_ActorMovementConstrainMsg		m_Msg;
};


} //namespace ActorMsg


XT_TYPE(ActorMsg::EMovementType, "EMovementType")
	XT_OPTION(ActorMsg::EMovementType_Linear, "Linear")
	XT_OPTION(ActorMsg::EMovementType_EaseIn, "EaseIn")
	XT_OPTION(ActorMsg::EMovementType_EaseOut, "EaseOut")
	XT_OPTION(ActorMsg::EMovementType_Smooth, "Smooth")
XT_END_TYPE

XT_ABSTRACT_TYPE(ActorMsg::C_XtActorMovementMsgBase, "MovementMessageBase")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorMovementSetParentMsg, "MovementSetParentMessage")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_ParentActor, "ParentActor")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorMovementTranslateMsg, "MovementTranslateMessage")
XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
XT_FIELD(m_VectorParam, "Translation")
XT_FIELD(m_Time, "Time")
XT_FIELD(m_Speed, "Speed")
XT_FIELD(m_MovementType, "MovementType")
XT_FIELD(m_MoveLocal, "MoveLocal")
XT_END_TYPE


XT_TYPE(ActorMsg::C_XtActorMovementRotateMsg, "MovementRotateMessage")
XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
XT_FIELD(m_Axis, "Axis")
XT_FIELD(m_Degrees, "Degrees")
XT_FIELD(m_Time, "Time")
XT_FIELD(m_MovementType, "MovementType")
XT_FIELD(m_MoveLocal, "MoveLocal")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorMovementRotateXMsg, "MovementRotateXMessage")
XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
XT_FIELD(m_Degrees, "Degrees")
XT_FIELD(m_Time, "Time")
XT_FIELD(m_MovementType, "MovementType")
XT_FIELD(m_MoveLocal, "MoveLocal")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorMovementRotateYMsg, "MovementRotateYMessage")
XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
XT_FIELD(m_Degrees, "Degrees")
XT_FIELD(m_Time, "Time")
XT_FIELD(m_MovementType, "MovementType")
XT_FIELD(m_MoveLocal, "MoveLocal")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorMovementRotateZMsg, "MovementRotateZMessage")
XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
XT_FIELD(m_Degrees, "Degrees")
XT_FIELD(m_Time, "Time")
XT_FIELD(m_MovementType, "MovementType")
XT_FIELD(m_MoveLocal, "MoveLocal")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorMovementSpinAxisYMsg, "MovementSpinUpYAxis")
XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
XT_FIELD(m_RPM, "RPM")
XT_FIELD(m_Time, "Time")
XT_FIELD(m_MovementType, "MovementType")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorMovementSpinStopMsg, "MovementSpinStop")
XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
XT_FIELD(m_StopTime, "StopTime")
XT_FIELD(m_MovementType, "MovementType")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorMovementAimAtMsg, "MovementAimAt")
XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
XT_FIELD(m_Position, "Position")
XT_FIELD(m_Time, "Time")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorMovementAimBetweenMsg, "MovementAimBetween")
XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
XT_FIELD(m_Position1, "Position1")
XT_FIELD(m_Position2, "Position2")
XT_FIELD(m_Time, "Time")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorMovementTrackMsg, "MovementTrack")
	XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
	XT_FIELD(m_Actor, "Actor")
	XT_FIELD(m_OffsetPos, "Offset")
	XT_FIELD(m_LocalOffset, "LocalOffset")
	XT_FIELD(m_UseRetrackingSpeed, "UseRetrackingSpeed")
	XT_FIELD(m_RetrackingSpeed, "RetrackingSpeed")
	XT_FIELD(m_ReplaceDefaultSpeed, "ReplaceDefaultSpeed")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorMovementTrackLagMsg, "MovementTrackLag")
XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
XT_FIELD(m_Actor, "Actor")
XT_FIELD(m_OffsetPos, "Offset")
XT_FIELD(m_LocalOffset, "LocalOffset")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorMovementTrackPredictionMsg, "MovementTrackPrediction")
XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
XT_FIELD(m_Actor, "Actor")
XT_FIELD(m_Time, "Time")
XT_FIELD(m_OffsetPos, "Offset")
XT_FIELD(m_LocalOffset, "LocalOffset")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorMovementMoveAndAlignToMsg, "MovementMoveAndAlignTo")
XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
XT_FIELD(m_Actor, "Actor")
XT_FIELD(m_Time, "Time")
XT_FIELD(m_MovementType, "MovementType")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorMovementSplineMsg, "MovementSplineMessage")
XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
XT_FIELD(m_SplineActor, "SplineActor")
XT_FIELD(m_MoveSpeed, "MoveSpeed")
XT_FIELD(m_Rotation, "RotationAngle")
XT_FIELD(m_Loop, "Loop")
XT_FIELD(m_MovementType, "MovementType")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorMovementKinematicSplineMsg, "MovementKinematicSplineMessage")
	XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
	XT_FIELD(m_SplineActor, "SplineActor")
	XT_FIELD(m_StartSpeed, "StartSpeed")
	XT_FIELD(m_MinSpeed, "MinSpeed")
	XT_FIELD(m_MaxSpeed, "MaxSpeed")
	XT_FIELD(m_Acceleration, "Acceleration")
	XT_FIELD(m_BrakeAtEnd, "BrakeAtEnd")
	XT_FIELD(m_Loop, "Loop")
	XT_FIELD(m_StartFromNearestPosition, "StartFromNearestPosition")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorMovementKinematicSplineTrackerMsg, "MovementKinematicSplineTrackerMessage")
	XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
	XT_FIELD(m_TrackerActor, "TrackerActor")
	XT_FIELD(m_MinSpeed, "MinSpeed")
	XT_FIELD(m_MaxSpeed, "MaxSpeed")
	XT_FIELD(m_Acceleration, "Acceleration")
	XT_FIELD(m_IdealDistance, "IdealDistance")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorMovementKinematicSplineCommandMsg, "MovementKinematicSplineCommandMessage")
XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
	XT_FIELD(m_MinSpeed, "MinSpeed")
	XT_FIELD(m_MaxSpeed, "MaxSpeed")
	XT_FIELD(m_Acceleration, "Acceleration")
XT_END_TYPE


XT_TYPE(ActorMsg::C_XtActorMovementConstrainMsg, "MovementConstrainMessage")
XT_BASE(ActorMsg::C_XtActorMovementMsgBase)
XT_FIELD(m_ConstrainXTrans, "ConstrainXTrans")
XT_FIELD(m_ConstrainYTrans, "ConstrainYTrans")
XT_FIELD(m_ConstrainZTrans, "ConstrainZTrans")
XT_FIELD(m_ConstrainPitch, "ConstrainXRot")
XT_FIELD(m_ConstrainHeading, "ConstrainYRot")
XT_FIELD(m_ConstrainRoll, "ConstrainZRot")
XT_END_TYPE


// Register Messages
XT_LIBRARY(MovementMessages)
XT_REGISTER_TYPE(ActorMsg::EMovementType)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementMsgBase)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementSetParentMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementTranslateMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementRotateMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementRotateXMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementRotateYMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementRotateZMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementSpinAxisYMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementSpinStopMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementAimAtMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementAimBetweenMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementTrackMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementTrackLagMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementTrackPredictionMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementMoveAndAlignToMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementSplineMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementKinematicSplineMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementKinematicSplineTrackerMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementKinematicSplineCommandMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMovementConstrainMsg)
XT_END_LIBRARY

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", on)
#endif
