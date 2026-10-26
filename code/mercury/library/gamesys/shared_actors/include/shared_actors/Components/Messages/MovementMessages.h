#ifndef MOVEMENT_MESSAGES_H
#define MOVEMENT_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

//#include <GameDef/GameDefTypes.h>
#include <misc_shared/uid/FourCC.h>
#include <shared_actors/SharedActorMessages.h>
#include <shared_actors/SharedMessageValues.h>
#include <shared_actors/XtActorMessages.h>


// Movement Component enums - here for now
namespace Components
{

// Movement Component Transform enum
enum E_MovementTransform
{
	E_MovementTransform_None = 0,
	E_MovementTransform_Update,
	E_MovementTransform_Finalise
};

} //namespace Components

// Movement messages
// -----------------
namespace ActorMsg
{

enum E_MovementMsg
{
	E_MovementMsg_HiBits = FOURCC_HI20('MVMT'), 

	E_MovementMsg_SetParent,
	E_MovementMsg_ClearParent,
	E_MovementMsg_IncMovementCount,
	E_MovementMsg_DecMovementCount,
	E_MovementMsg_AddChild,
	E_MovementMsg_RemoveChild,
	E_MovementMsg_Update,
	E_MovementMsg_Enable,
	E_MovementMsg_Disable,
	E_MovementMsg_Translate,
	E_MovementMsg_Rotate,
	E_MovementMsg_SpinAxisY,
	E_MovementMsg_SpinStop,
	E_MovementMsg_MoveAndAlignTo,
	E_MovementMsg_MoveAndAlignToActor,
	E_MovementMsg_Spline,
	E_MovementMsg_KinematicSpline,
	E_MovementMsg_KinematicSplineTracker,
	E_MovementMsg_KinematicSplineCommand,
	E_MovementMsg_AddSplineSlave,
	E_MovementMsg_RemoveSplineSlave,
	E_MovementMsg_SetSlaveMasterOffset,
	E_MovementMsg_SetLastSlaveOffset,
	E_MovementMsg_UpdateSlave,
	E_MovementMsg_AimAt,
	E_MovementMsg_AimBetween,
	E_MovementMsg_Track,
	E_MovementMsg_TrackLag,
	E_MovementMsg_TrackPrediction,
	E_MovementMsg_Constrain,
	

	E_MovementMsg_End, 
	E_MovementMsg_Begin = E_MovementMsg_HiBits + 1, 
	E_MovementMsg_Count = E_MovementMsg_End - E_MovementMsg_Begin
};

enum E_MovementNotificationMsg
{
	E_MovementNotificationMsg_HiBits = FOURCC_HI20('MVNT'), 

	E_MovementNotificationMsg_MovementComplete,
	E_MovementNotificationMsg_AimerAligned,
	E_MovementNotificationMsg_AimerLostAlignment,

	E_MovementNotificationMsg_End, 
	E_MovementNotificationMsg_Begin = E_MovementNotificationMsg_HiBits + 1, 
	E_MovementNotificationMsg_Count = E_MovementNotificationMsg_End - E_MovementNotificationMsg_Begin
};

enum EMovementType
{
	EMovementType_Linear,
	EMovementType_EaseIn,
	EMovementType_EaseOut,
	EMovementType_Smooth
};

enum TMotionTypeFlags
{
	E_MovementFlag_Invalid	= 0,
	E_MovementFlag_Linear	= 0x01,
	E_MovementFlag_EaseIn	= 0x02,
	E_MovementFlag_EaseOut	= 0x04,

	E_MovementFlag_Smooth	= E_MovementFlag_EaseIn | E_MovementFlag_EaseOut,
};


// Intermediate class for movement messages with a notification handle.
class C_MovementNotifierMessageBase : public Messaging::C_MessageBase
{
public:
	const Actor::C_ActorHandle Notifier() const {return m_NotificationHandle;}
	void SetNotifier(const Actor::C_ActorHandle& notifier) {m_NotificationHandle = notifier;}

protected:
	C_MovementNotifierMessageBase(Messaging::TMsgType msgType, const U32 msgSize) : Messaging::C_MessageBase(msgType, msgSize) {}

protected:
	Actor::C_ActorHandle m_NotificationHandle;
};


template <class MessageClass, Messaging::TMsgType TypeID>
class C_MovementNotifierMessageDerived : public	C_MovementNotifierMessageBase
{
public:
	static Messaging::TMsgType StaticType() {return TypeID;} ///< @returns type identifier for this message class.

protected:
	C_MovementNotifierMessageDerived() : C_MovementNotifierMessageBase(TypeID, sizeof(MessageClass)) {}
	explicit C_MovementNotifierMessageDerived(const Actor::C_ActorHandle& notifier) : C_MovementNotifierMessageBase(TypeID, sizeof(MessageClass)) {SetNotifier(notifier);}
};


// Xt Movement Message Base
class C_XtActorMovementMsgBase : public ActorMsg::C_XtActorMessageBase
{
public:
};

typedef Actor::C_MessageActorHandle<E_MovementMsg_SetParent> C_ActorMovementSetParentMsg;
typedef Actor::C_MessageActorHandle<E_MovementMsg_AddChild> C_ActorMovementAddChildMsg;
typedef Actor::C_MessageActorHandle<E_MovementMsg_RemoveChild> C_ActorMovementRemoveChildMsg;

class C_ActorMovementUpdateMsg : public Messaging::C_MessageDerived<C_ActorMovementUpdateMsg, E_MovementMsg_Update>
{
public:
// 	enum E_Transform
// 	{
// 		E_NoTransform = 0,
// 		E_UpdateTransform,
// 		E_FinaliseTransform
// 	};

	//bool m_RecalcTransform;
	Components::E_MovementTransform	m_Transform;

	//explicit C_ActorMovementUpdateMsg(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans parentWorldTransform = Maths::C_QuatTrans(), bool recalcTransform = true);
	explicit C_ActorMovementUpdateMsg(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans parentWorldTransform = Maths::C_QuatTrans(), const Components::E_MovementTransform transform = Components::E_MovementTransform_Update);
	~C_ActorMovementUpdateMsg();

	Maths::C_TimeS TimeStep() const {return m_TimeStep.Get();}
	Maths::C_QuatTrans ParentWorldTransform() const {return m_QuatTrans.Get();}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	Messaging::C_MessageValueStorage<Maths::C_TimeS> m_TimeStep;
	Messaging::C_MessageValueStorage<Maths::C_QuatTrans> m_QuatTrans;

	static Messaging::C_MessageSerialiser s_Serialiser;

};


class C_ActorMovementTranslateMsg : public C_MovementNotifierMessageDerived<C_ActorMovementTranslateMsg, E_MovementMsg_Translate>
{
public:
	U32 m_MovementFlags;	// See C_MovementComponentBase::TMotionTypeFlags
	bool m_MoveLocal;

	C_ActorMovementTranslateMsg();
	C_ActorMovementTranslateMsg(const Maths::C_Vector& translation, const Maths::C_TimeS& movementTime, U32 movementFlags);
	~C_ActorMovementTranslateMsg();

	Maths::C_TimeS	MovementTime() const {return m_MovementTime.Get();}
	void			SetMovementTime(const Maths::C_TimeS& movementTime) {m_MovementTime.Set(movementTime);}
	Maths::C_Vector Translation() const {return m_Translation.Get();}
	void			SetTranslation(const Maths::C_Vector& translation) {m_Translation.Set(translation);}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	Messaging::C_MessageValueStorage<Maths::C_TimeS> m_MovementTime;
	Messaging::C_MessageValueStorage<Maths::C_Vector> m_Translation;

	static Messaging::C_MessageSerialiser s_Serialiser;

};

class C_ActorMovementRotateMsg : public C_MovementNotifierMessageDerived<C_ActorMovementRotateMsg, E_MovementMsg_Rotate>
{
public:
	F32 m_Degrees;
	U32 m_MovementFlags;	// See C_MovementComponentBase::TMotionTypeFlags
	bool m_MoveLocal;

	C_ActorMovementRotateMsg();
	C_ActorMovementRotateMsg(F32 degrees, const Maths::C_Normal& axis, const Maths::C_TimeS& movementTime, U32 movementFlags);
	~C_ActorMovementRotateMsg();

	Maths::C_TimeS	MovementTime() const {return m_MovementTime.Get();}
	void			SetMovementTime(const Maths::C_TimeS& movementTime) {m_MovementTime.Set(movementTime);}
	Maths::C_Normal	Axis() const {return m_Axis.Get();}
	void			SetAxis(const Maths::C_Normal& axis) {m_Axis.Set(axis);}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	Messaging::C_MessageValueStorage<Maths::C_TimeS> m_MovementTime;
	Messaging::C_MessageValueStorage<Maths::C_Normal> m_Axis;

	static Messaging::C_MessageSerialiser s_Serialiser;

};

class C_ActorMovementSpinAxisYMsg : public C_MovementNotifierMessageDerived<C_ActorMovementSpinAxisYMsg, E_MovementMsg_SpinAxisY>
{
public:
	F32				m_RevsPerMinute;

	C_ActorMovementSpinAxisYMsg();
	C_ActorMovementSpinAxisYMsg(F32 revsPerMinute, const Maths::C_TimeS& spinupTime);
	~C_ActorMovementSpinAxisYMsg();

	Maths::C_TimeS	SpinUpTime() const {return m_SpinupTime.Get();}
	void			SetSpinUpTime(const Maths::C_TimeS& movementTime) {m_SpinupTime.Set(movementTime);}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	Messaging::C_MessageValueStorage<Maths::C_TimeS> m_SpinupTime;

	static Messaging::C_MessageSerialiser s_Serialiser;

};

class C_ActorMovementSpinStopMsg : public C_MovementNotifierMessageDerived<C_ActorMovementTranslateMsg, E_MovementMsg_SpinStop>
{
public:
	C_ActorMovementSpinStopMsg() {}
	C_ActorMovementSpinStopMsg(const Maths::C_Vector& translation, const Maths::C_TimeS& movementTime, U32 movementFlags);

	Maths::C_TimeS	Time() const {return m_Time.Get();}
	void			SetTime(const Maths::C_TimeS& time) {m_Time.Set(time);}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	Messaging::C_MessageValueStorage<Maths::C_TimeS> m_Time;
	static Messaging::C_MessageSerialiser s_Serialiser;
};

// Aim at a specific point in the world
class C_ActorMovementAimAtMsg : public Messaging::C_MessageDerived<C_ActorMovementAimAtMsg, E_MovementMsg_AimAt>
{
public:
	F32						m_Time;

	C_ActorMovementAimAtMsg();
	~C_ActorMovementAimAtMsg();

	Maths::C_Vertex			GetPosition() const {return m_Position.Get();}
	void					SetPosition(const Maths::C_Vertex& position) {m_Position.Set(position);}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	Messaging::C_MessageValueStorage<Maths::C_Vertex> m_Position;

	static Messaging::C_MessageSerialiser s_Serialiser;

};

// Aim between 2 points in the world
class C_ActorMovementAimBetweenMsg : public Messaging::C_MessageDerived<C_ActorMovementAimBetweenMsg, E_MovementMsg_AimBetween>
{
public:
	F32						m_Time;

	C_ActorMovementAimBetweenMsg();
	~C_ActorMovementAimBetweenMsg();

	Maths::C_Vertex			GetPosition1() const {return m_Pos1.Get();}
	void					SetPosition1(const Maths::C_Vertex& position) {m_Pos1.Set(position);}
	Maths::C_Vertex			GetPosition2() const {return m_Pos2.Get();}
	void					SetPosition2(const Maths::C_Vertex& position) {m_Pos2.Set(position);}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	Messaging::C_MessageValueStorage<Maths::C_Vertex> m_Pos1;
	Messaging::C_MessageValueStorage<Maths::C_Vertex> m_Pos2;

	static Messaging::C_MessageSerialiser s_Serialiser;

};




// Track an actor
class C_ActorMovementTrackMsg : public Messaging::C_MessageDerived<C_ActorMovementTrackMsg, E_MovementMsg_Track>
{
public:
	Actor::C_ActorHandle	m_ActorHandle;
	bool					m_LocalOffset;
	bool					m_UseRetrackingSpeed;
	F32						m_RetrackingSpeed;
	bool					m_ReplaceDefaultSpeed;

	C_ActorMovementTrackMsg();
	~C_ActorMovementTrackMsg();

	Maths::C_Vertex			GetOffsetPosition() const {return m_OffsetPos.Get();}
	void					SetOffsetPosition(const Maths::C_Vertex& position) {m_OffsetPos.Set(position);}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	Messaging::C_MessageValueStorage<Maths::C_Vertex> m_OffsetPos;

	static Messaging::C_MessageSerialiser s_Serialiser;

};

// track an actor with Lag
class C_ActorMovementTrackLagMsg : public Messaging::C_MessageDerived<C_ActorMovementTrackLagMsg, E_MovementMsg_TrackLag>
{
public:
	Actor::C_ActorHandle	m_ActorHandle;
	bool					m_LocalOffset;

	C_ActorMovementTrackLagMsg();
	~C_ActorMovementTrackLagMsg();

	Maths::C_Vertex			GetOffsetPosition() const {return m_OffsetPos.Get();}
	void					SetOffsetPosition(const Maths::C_Vertex& position) {m_OffsetPos.Set(position);}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	Messaging::C_MessageValueStorage<Maths::C_Vertex> m_OffsetPos;

	static Messaging::C_MessageSerialiser s_Serialiser;

};

// track an actor with prediction
class C_ActorMovementTrackPredictionMsg : public Messaging::C_MessageDerived<C_ActorMovementTrackPredictionMsg, E_MovementMsg_TrackPrediction>
{
public:
	Actor::C_ActorHandle	m_ActorHandle;
	bool					m_LocalOffset;
	F32						m_Time;

	C_ActorMovementTrackPredictionMsg();
	~C_ActorMovementTrackPredictionMsg();

	Maths::C_Vertex			GetOffsetPosition() const {return m_OffsetPos.Get();}
	void					SetOffsetPosition(const Maths::C_Vertex& position) {m_OffsetPos.Set(position);}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	Messaging::C_MessageValueStorage<Maths::C_Vertex> m_OffsetPos;

	static Messaging::C_MessageSerialiser s_Serialiser;

};


typedef Actor::C_MessageActorHandle<E_MovementNotificationMsg_AimerAligned> C_ActorMovementNotificationAimerAligned;


typedef Actor::C_MessageActorHandle<E_MovementNotificationMsg_AimerLostAlignment> C_ActorMovementNotificationAimerLostAlignment;


class C_ActorMovementMoveAndAlignToMsg : public Messaging::C_MessageDerived<C_ActorMovementMoveAndAlignToMsg, E_MovementMsg_MoveAndAlignTo>
{
public:
	U32 m_MovementFlags;	// See C_MovementComponentBase::TMotionTypeFlags

	C_ActorMovementMoveAndAlignToMsg();
	C_ActorMovementMoveAndAlignToMsg(const Maths::C_QuatTrans& quatTrans, const Maths::C_TimeS& movementTime, U32 movementFlags);
	~C_ActorMovementMoveAndAlignToMsg();

	Maths::C_TimeS MovementTime() const {return m_MovementTime.Get();}
	void SetMovementTime(const Maths::C_TimeS& movementTime) {m_MovementTime.Set(movementTime);}
	Maths::C_QuatTrans Transform() const {return m_QuatTrans.Get();}
	void StoreTransform(const Maths::C_QuatTrans& quatTrans) {m_QuatTrans.Set(quatTrans);}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	Messaging::C_MessageValueStorage<Maths::C_TimeS> m_MovementTime;
	Messaging::C_MessageValueStorage<Maths::C_QuatTrans> m_QuatTrans;

	static Messaging::C_MessageSerialiser s_Serialiser;
	
};


class C_ActorMovementMoveAndAlignToActorMsg : public C_MovementNotifierMessageDerived<C_ActorMovementMoveAndAlignToActorMsg, E_MovementMsg_MoveAndAlignToActor>
{
public:
	Actor::C_ActorHandle m_ActorHandle;
	U32 m_MovementFlags;	// See C_MovementComponentBase::TMotionTypeFlags

	C_ActorMovementMoveAndAlignToActorMsg();
	~C_ActorMovementMoveAndAlignToActorMsg();
	
	Maths::C_TimeS MovementTime() const {return m_MovementTime.Get();}
	void SetMovementTime(const Maths::C_TimeS& movementTime) {m_MovementTime.Set(movementTime);}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	Messaging::C_MessageValueStorage<Maths::C_TimeS> m_MovementTime;

	static Messaging::C_MessageSerialiser s_Serialiser;
	
};

class C_ActorMovementSplineMsg : public C_MovementNotifierMessageDerived<C_ActorMovementSplineMsg, E_MovementMsg_Spline>
{
public:
	Actor::C_ActorHandle m_ActorHandle;
	F32 m_MoveSpeed;
	F32 m_Rotation;
	bool m_Loop;
	U32 m_MovementFlags;

	C_ActorMovementSplineMsg();
	~C_ActorMovementSplineMsg();

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	Messaging::C_MessageValueStorage<Maths::C_TimeS> m_MovementTime;

	static Messaging::C_MessageSerialiser s_Serialiser;
	
};

class C_ActorMovementKinematicSplineMsg : public C_MovementNotifierMessageDerived<C_ActorMovementKinematicSplineMsg, E_MovementMsg_KinematicSpline>
{
public:
	Actor::C_ActorHandle m_ActorHandle;
	F32 m_StartSpeed;
	F32 m_MinSpeed;
	F32	m_MaxSpeed;
	F32 m_Acceleration;
	bool m_BrakeAtEnd;
	bool m_Loop;
	bool m_StartFromNearestPosition;
	

	C_ActorMovementKinematicSplineMsg();
	~C_ActorMovementKinematicSplineMsg();

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	
	static Messaging::C_MessageSerialiser s_Serialiser;

};

class C_ActorMovementKinematicSplineTrackerMsg : public C_MovementNotifierMessageDerived<C_ActorMovementKinematicSplineTrackerMsg, E_MovementMsg_KinematicSplineTracker>
{
public:
	Actor::C_ActorHandle m_TrackerActorHandle;
	F32 m_MinSpeed;
	F32	m_MaxSpeed;
	F32 m_Acceleration;
	F32	m_IdealDistance;
	
	C_ActorMovementKinematicSplineTrackerMsg();
	~C_ActorMovementKinematicSplineTrackerMsg();

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	static Messaging::C_MessageSerialiser s_Serialiser;

};

class C_ActorMovementKinematicSplineCommandMsg : public C_MovementNotifierMessageDerived<C_ActorMovementKinematicSplineCommandMsg, E_MovementMsg_KinematicSplineCommand>
{
public:
	F32 m_MinSpeed;
	F32	m_MaxSpeed;
	F32 m_Acceleration;
	

	C_ActorMovementKinematicSplineCommandMsg();
	~C_ActorMovementKinematicSplineCommandMsg();

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	static Messaging::C_MessageSerialiser s_Serialiser;

};

typedef Actor::C_MessageActorHandle<E_MovementMsg_AddSplineSlave> C_ActorMovementAddSplineSlaveMsg;
typedef Actor::C_MessageActorHandle<E_MovementMsg_RemoveSplineSlave> C_ActorMovementRemoveSplineSlaveMsg;
typedef Messaging::C_MessageValue<F32, E_MovementMsg_SetSlaveMasterOffset> C_ActorMovementSetSlaveMasterOffset;
typedef Messaging::C_MessageValue<F32, E_MovementMsg_SetLastSlaveOffset> C_ActorMovementSetLastSlaveOffset;

class C_ActorMovementUpdateSlaveMsg : public Messaging::C_MessageDerived<C_ActorMovementUpdateSlaveMsg, E_MovementMsg_UpdateSlave>
{
public:
	
	C_ActorMovementUpdateSlaveMsg(F32 timeStep, F32 progression) : m_TimeStep(timeStep), m_SlaveMasterProgression(progression) {}
	F32 m_TimeStep;
	F32	m_SlaveMasterProgression;
};

class C_ActorMovementConstrainMsg : public Messaging::C_MessageDerived<C_ActorMovementConstrainMsg, E_MovementMsg_Constrain>
{
public:
	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

public:
	bool m_ConstrainXTrans;
	bool m_ConstrainYTrans;
	bool m_ConstrainZTrans;
	bool m_ConstrainHeading;
	bool m_ConstrainPitch;
	bool m_ConstrainRoll;

	static Messaging::C_MessageSerialiser s_Serialiser;
};


} //namespace ActorMsg


#endif

