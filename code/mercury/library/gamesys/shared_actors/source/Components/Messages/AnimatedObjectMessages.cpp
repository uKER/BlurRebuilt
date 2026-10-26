#include "SharedActorsPch.h"
#include <shared_actors/Components/Messages/AnimatedObjectMessages.h>
#include <resource_utils/CycleResourceDef.h>




namespace ActorMsg
{

//
// C_XtAnimatedObjectPlayCycleMsg
//
class C_XtAnimatedObjectPlayCycleMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	//	C_XtAnimatedObjectPlayCycleMsg() {}

	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext)
	{
		m_Msg.m_CycleIndex = m_CycleIndex;
		m_Msg.m_Speed = m_Speed;
		m_Msg.m_FadeInTime = m_FadeInTime;
		m_Msg.m_LoopCount = m_LoopCount;
		return &m_Msg;
	}

	S32		m_CycleIndex;
	F32		m_Speed;
	U32		m_LoopCount;
	F32		m_FadeInTime;

	C_AnimatedObjectPlayCycleMsg					m_Msg;
};

//
// C_XtAnimatedObjectLoadCycleMsg
//
class C_XtAnimatedObjectLoadCycleMsg: public ActorMsg::C_XtActorMessageBase
{
public:

	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor* pActorContext)
	{
		m_Msg.SetValue(m_CycleIndex);
		return &m_Msg;
	}

	S32	m_CycleIndex;

private:
	
	C_AnimatedObjectLoadCycleMsg		m_Msg;
};


} // end ActorMsg

XT_TYPE(ActorMsg::C_XtAnimatedObjectPlayCycleMsg, "AnimatedObjectPlayCycleMsg")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_CycleIndex, "CycleIndex")
XT_FIELD(m_Speed, "Speed")
XT_FIELD(m_LoopCount, "LoopCount")
XT_FIELD(m_FadeInTime, "FadeInTime")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtAnimatedObjectLoadCycleMsg, "AnimatedObjectLoadCycleMsg")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_CycleIndex, "CycleIndex")
XT_END_TYPE

XT_LIBRARY(AnimatedObjectMessages)
XT_REGISTER_TYPE(ActorMsg::C_XtAnimatedObjectPlayCycleMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtAnimatedObjectLoadCycleMsg)
XT_END_LIBRARY

