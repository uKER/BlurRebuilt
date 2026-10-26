#ifndef DYNAMICACTORCOMPONENTS_H
#define DYNAMICACTORCOMPONENTS_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>
#include <shared_actors/XtActorMessages.h>
#include <GameLogic/ActorStudioActors/ResourceTypes.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>

#include <misc_shared/Messaging/MessageBase.h>

#include <shared_actors/Components/Impl/MovementComponents.h>
#include <shared_actors/Components/Messages/MovementMessages.h>
#include <shared_actors/Components/Creation/MovementComponentsCreate.h>

#include <Shared_Actors/ActorImpl/SplineActor.h>
#include <Shared_Actors/TypeWrappers/SplineVec3Wrapper.h>


namespace Actors
{
	class C_SplineActor;
}


//=========================================================================
//						COMPONENT TYPES
//=========================================================================

namespace Components
{


enum E_SPLINEMOVEMENTTYPE
{
	E_SPLINEMOVEMENTTYPE_SINGLE,
	E_SPLINEMOVEMENTTYPE_LOOP,
	E_SPLINEMOVEMENTTYPE_PINGPONG,
	E_SPLINEMOVEMENTTYPE_PINGPONG_REVERSE,
};

enum E_KNOTDATADIRECTIONTYPE
{
	E_KNOTDATADIRECTIONTYPE_BOTH,
	E_KNOTDATADIRECTIONTYPE_FORWARDONLY,
	E_KNOTDATADIRECTIONTYPE_REVERSEONLY,
};


enum E_AmaxSharedComponent
{
	E_AmaxSharedComponent_HiBits = FOURCC_HI20('ASAC'), // Amax Shared Actor Component

	E_MovementIntelligentSplineFollower,
};


class C_MovementControlData : public XtKnotData
{
public:
	E_KNOTDATADIRECTIONTYPE							m_Direction;
	XtArray<ActorMsg::C_XtActorMessageBase*>		m_ControlMessages;
	XtArray<ActorMsg::C_XtSendMessage>				m_EventMessages;
};

void RegisterDynamicActorComponents(void);



//=========================================================================
//					INTELLIGENT SPLINE FOLLOWER COMPONENT
//=========================================================================

class C_MovementIntelligentSplineFollowerCreateInfo : public C_MovementCreateInfo
{
public:
	S32 GetComponentTypeID() const {return Components::E_MovementIntelligentSplineFollower;}		// From C_MovementCreateInfo

public:
	const ::Actors::C_SplineActorCreate*	m_SplineActor;
	E_SPLINEMOVEMENTTYPE					m_MovementType;
	F32										m_StartSpeed;
	F32										m_MaxSpeed;
	F32										m_Acceleration;
	F32										m_SlowDownAtSplineEnds;
	F32										m_WaitAtSplineEnds;
	F32										m_StartBankAngle;
	bool									m_StartFromNearestPoint;				// Snap actor to nearest spline position and move from that offset.

	const ::Actors::C_PosActorCreate*		m_PullingActor;
	bool									m_UsePivot;								// For articulated actors
	F32										m_OriginToPivotDistForPuller;
	F32										m_OriginToPivotDistForThis;

	F32										m_RotationDamping;
	bool									m_AlignToGround;						// Cast a ray to get ground level
	bool									m_FlipModelOrientation;					// Turn model through 180 deg
};

//=========================================================================


class C_MovementIntelligentSplineFollower : public C_MovementBase
{
public:
	ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE()

public:
	C_MovementIntelligentSplineFollower() {}

public: // From C_MovementBase
	static Component::C_ComponentTypeId						ClassType() {return Components::E_MovementIntelligentSplineFollower;}		// E_MovementKinematicSpline
	virtual Component::C_ComponentTypeId					GetComponentTypeID() const {return Components::E_MovementIntelligentSplineFollower;} // from Component::I_GameComponent
	bool													OnMessage(const Messaging::C_MessageBase* pMessage);
	const C_MovementIntelligentSplineFollowerCreateInfo*	GetCreationData(void) const {return static_cast<const C_MovementIntelligentSplineFollowerCreateInfo *>(GetCreator().GetData());}

	virtual void SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void DeserialiseForSync(Sys::C_ContextualStream& stream);
#ifndef CFG_GOLD
	virtual void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif


protected:
	Btl::Vector<Actor::C_ActorHandle>	m_Slaves;

	bool								m_SnappedToSpline;

	const ::Actors::C_SplineActor*		m_SplineActor;
	E_SPLINEMOVEMENTTYPE				m_MovementType;
	F32									m_MaxSpeed;
	F32									m_Acceleration;
	F32									m_SlowDownAtSplineEnds;
	F32									m_WaitAtSplineEnds;
	F32									m_BankAngle;

	Actor::C_ActorHandle				m_PullingActorHandle;				// Object thats pulling us
	bool								m_UsePivot;								// For articulated actors
	F32									m_OriginToPivotDistForPuller;
	F32									m_OriginToPivotDistForThis;

	F32									m_RotationDamping;
	bool								m_AlignToGround;						// Cast a ray to get ground level
	bool								m_FlipModelOrientation;				// Snap actor to nearest spline position and move from that offset.


	F32									m_ArcDistance;
	F32									m_SplineLength;
	F32									m_Velocity;

	S32									num_control_elements;
	S32									current_control_element;
	bool								reversing;
	bool								flipdirection;
	
	F32									waittime;

	bool								beingpulled;		// True if being pulled

	bool								pulling;			// Object we're pulling
	F32									pulloffset;
	Actor::C_ActorHandle				pullhandle;


	virtual bool	InitialiseImpl();							// from Component::I_GameComponent
	virtual bool	CreateImpl();								// from Component::I_GameComponent
	virtual void	ExternalDependenciesCreatedImpl();			// from Component::I_GameComponent
	virtual void	Animate(const Maths::C_TimeS& timeStep);	// From C_MovementBase

	void CheckControlElement();									// Check if we've just passed some control data (Note: Control data needs correctly ordering for this to work)
	void ProcessControlElement();								// Action any control data in the current element

	void CheckAlignToGround(Maths::C_QuatTrans *qtran);

	S32						GetControlDataCount()		{ return(m_SplineActor->GetCreationData()->m_SplineData.knotData.count); }
	C_MovementControlData*	GetControlData(S32 index)	{ return((C_MovementControlData*)m_SplineActor->GetCreationData()->m_SplineData.knotData[index]); }

	void GetSplineActor();
	void MoveToNearestKnot();
	void UpdateSlaves(const Maths::C_TimeS& timeStep);

	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);
};


//=========================================================================
//					INTELLIGENT SPLINE FOLLOWER MESSAGES
//=========================================================================


}// End namespace Components


#endif // DYNAMICACTORCOMPONENTS_H
