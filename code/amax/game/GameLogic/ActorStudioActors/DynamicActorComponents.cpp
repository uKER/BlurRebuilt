#include "stdafx.h"


#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
#include <GameLogic/ActorStudioActors/ActorUtils.h>

#include <btl\Containers\algorithm.h>
#include <xt/XtGame.h>
#include <xt/XtStd.h>

#include <gravity/narrowphase/PrimitiveTest.h>

#include <Misc_shared/Messaging/MessageTypeList.h>
#include <Misc_shared/NewCurveHermite.h>
#include <Misc_Shared/NewCurve/Actor/ActorSplineTypes.h>
#include <Misc_Shared/NewCurve/Actor/ActorHelpers.h>

#include <Gravity/NarrowPhase/Primitive.h>
#include <Gravity/NarrowPhase/CollisionResult.h>
#include <Gravity/NarrowPhase/PrimitiveTest.h>
#include <Gravity/NarrowPhase/Primitive.inl>

#include <shared_actors/Components/SharedComponents.h>
#include <Shared_Actors/Components/Creation/MovementComponentsCreate.h>
#include <shared_actors/Components/Interfaces/I_PhysicsBodyComponents.h>
#include <Shared_Actors/ActorImpl/SplineActor.h>
#include <Shared_Actors/TypeWrappers/SplineVec3Wrapper.h>

#include <actor_system/Actor.h>
#include <actor_system/ActorList.h>
#include <actor_system/ActorDebug.h>
#include <actor_system/GameComponentFactory.h>

#include <core/maths/hvec/New/IncludeGeneral.h>
#include <core/maths/hvec/New/IncludeGeneral.inl>

#include "DynamicActorComponents.h"





//Debug::Float g_debug_DynObj_SplineEndCorrection("DynamicObjects.SplineEndCorrection", 0.2f);


//#pragma optimize("", off)


//=========================================================================
//		Sorcery definitions
//=========================================================================


XT_TYPE(Components::E_SPLINEMOVEMENTTYPE,							"SplineMovementType")
	XT_OPTION(Components::E_SPLINEMOVEMENTTYPE_SINGLE,				"Single")
	XT_OPTION(Components::E_SPLINEMOVEMENTTYPE_LOOP,				"Loop")
	XT_OPTION(Components::E_SPLINEMOVEMENTTYPE_PINGPONG,			"PingPong")
	XT_OPTION(Components::E_SPLINEMOVEMENTTYPE_PINGPONG_REVERSE,	"PingPongReverse")
XT_END_TYPE

XT_TYPE(Components::E_KNOTDATADIRECTIONTYPE,						"KnotDataDirection")
	XT_OPTION(Components::E_KNOTDATADIRECTIONTYPE_BOTH,				"Both")
	XT_OPTION(Components::E_KNOTDATADIRECTIONTYPE_FORWARDONLY,		"ForwardOnly")
	XT_OPTION(Components::E_KNOTDATADIRECTIONTYPE_REVERSEONLY,		"ReverseOnly")
XT_END_TYPE

XT_TYPE(Components::C_MovementIntelligentSplineFollowerCreateInfo, "IntelligentSplineFollowerMovementComponent")
	XT_BASE(Components::C_MovementCreateInfo)
	XT_FIELD(m_SplineActor,					"Spline")
	XT_FIELD(m_MovementType,				"MovementType")
	XT_FIELD(m_StartSpeed,					"StartSpeed")
	XT_FIELD(m_MaxSpeed,					"MaxSpeed")
	XT_FIELD(m_Acceleration,				"Acceleration")
	XT_FIELD(m_SlowDownAtSplineEnds,		"SlowDownAtSplineEnds")
	XT_FIELD(m_WaitAtSplineEnds,			"WaitAtSplineEnds")
	XT_FIELD(m_StartBankAngle,				"StartBankAngle")
	XT_FIELD(m_StartFromNearestPoint,		"StartFromNearestPoint")

	XT_FIELD(m_PullingActor,				"PullingActor")
	XT_FIELD(m_UsePivot,					"UsePivot")
	XT_FIELD(m_OriginToPivotDistForPuller,	"OriginToPivotDistForPuller")
	XT_FIELD(m_OriginToPivotDistForThis,	"OriginToPivotDistForThis")

	XT_FIELD(m_RotationDamping,				"RotationDamping")
	XT_FIELD(m_AlignToGround,				"AlignToGround")
	XT_FIELD(m_FlipModelOrientation,		"FlipModelOrientation")
XT_END_TYPE


XT_TYPE(Components::C_MovementControlData,	"MovementControlData")
	XT_BASE(XtKnotData)
	XT_FIELD(m_Direction,					"Direction")
	XT_FIELD(m_ControlMessages,				"ControlMessages")
	XT_FIELD(m_EventMessages,				"EventMessages")
XT_END_TYPE


XT_LIBRARY(AmaxActorComponents)
	XT_REGISTER_TYPE(Components::E_SPLINEMOVEMENTTYPE)
	XT_REGISTER_TYPE(Components::E_KNOTDATADIRECTIONTYPE)
	XT_REGISTER_TYPE(Components::C_MovementIntelligentSplineFollowerCreateInfo)
	XT_REGISTER_TYPE(Components::C_MovementControlData)
XT_END_LIBRARY


//=========================================================================
//					AMAX COMPONENT TYPES
//=========================================================================



namespace Components
{

using namespace Maths;

ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN(C_MovementIntelligentSplineFollower)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(ActorMsg::E_DynamicObjMsg_Begin, ActorMsg::E_DynamicObjMsg_End)
ACTOR_COMPONENT_MESSAGE_TYPES_END(C_MovementIntelligentSplineFollower, C_MovementBase)


//=========================================================================


void RegisterDynamicActorComponents(void)
{
	Component::C_GameComponentFactory *factory = Component::C_GameComponentFactory::GetInstance();
	assert(factory);
	factory->Register<C_MovementIntelligentSplineFollower>();
}

//=========================================================================
//					INTELLIGENT SPLINE FOLLOWER
//=========================================================================


bool C_MovementIntelligentSplineFollower::InitialiseImpl()
{
	m_SplineActor = NULL;

	m_SnappedToSpline = false;

	m_ArcDistance = 0.0f;
	m_Velocity = GetCreationData()->m_StartSpeed;
	m_MaxSpeed = GetCreationData()->m_MaxSpeed;
	m_Acceleration = GetCreationData()->m_Acceleration;
	m_MovementType = GetCreationData()->m_MovementType;
	m_SlowDownAtSplineEnds = GetCreationData()->m_SlowDownAtSplineEnds;
	m_WaitAtSplineEnds = GetCreationData()->m_WaitAtSplineEnds;
	m_BankAngle = GetCreationData()->m_StartBankAngle;

	m_UsePivot = GetCreationData()->m_UsePivot;
	m_OriginToPivotDistForPuller = GetCreationData()->m_OriginToPivotDistForPuller;
	m_OriginToPivotDistForThis = GetCreationData()->m_OriginToPivotDistForThis;

	m_RotationDamping = GetCreationData()->m_RotationDamping;
	m_AlignToGround = GetCreationData()->m_AlignToGround;
	m_FlipModelOrientation = GetCreationData()->m_FlipModelOrientation;


	num_control_elements = 0;
	current_control_element = 0;
	reversing = false;
	flipdirection = false;
	waittime = 0.0f;
	beingpulled = false;
	pulling = false;
	pulloffset = 0.0f;

	return C_MovementBase::InitialiseImpl();
}

//=========================================================================

bool C_MovementIntelligentSplineFollower::CreateImpl()
{
//	GetSplineActor();
	return(C_MovementBase::CreateImpl());
}

//=========================================================================

void C_MovementIntelligentSplineFollower::ExternalDependenciesCreatedImpl()
{
	GetSplineActor();									// Needed here to make sure other actors are loaded
	C_MovementBase::ExternalDependenciesCreatedImpl();
}

//=========================================================================

void C_MovementIntelligentSplineFollower::GetSplineActor()
{
	m_ParentOffset = Maths::C_QuatTrans();

	if (GetCreationData()->m_SplineActor)
	{
		Actor::C_ActorHandle splineactorhandle = GetCreationData()->m_SplineActor->GetActorHandle();
		if (splineactorhandle)
		{
			m_SplineActor = ::Actor::ActorCast_Safe< ::Actors::C_SplineActor>(GetActorList()->GetActorPtr(splineactorhandle));
			if (m_SplineActor)
			{
				m_SplineLength = m_SplineActor->GetSplineWrapper()->GetCurveEval().GetArcLength();
//				if (GetCreationData()->m_StartFromNearestPoint)				// NOW DONE IN FIRST FRAME OF UPDATE SO ATTACHERS HAVE HAD CHANCE TO WORK OUT RELATIVE POSITIONS
//					MoveToNearestKnot();
//				m_SplineActor->GetSplineWrapper()->GetForwardFacingSplinePos(m_WorldTransform, m_ArcDistance);

				SetIsAnimating(true);					// It's always animating even when it's not moving!

				num_control_elements = GetControlDataCount();
				current_control_element = 0;

				CheckControlElement();
			}
		}

		AddSubscriber( ActorMsg::E_DynamicObjMsg_SplineCtrl_SetSpeed );
	}

	if (GetCreationData()->m_PullingActor)				// Notify pulling actor that we're attached
	{
		m_PullingActorHandle = GetCreationData()->m_PullingActor->GetActorHandle();
		if (m_PullingActorHandle)
		{
			beingpulled = true;

			ActorMsg::C_ActorLogicMsg_AttachToPuller	msg;
			msg.m_handle = GetActor()->GetHandle();
			msg.m_worldpos = GetActor()->GetCreationData()->GetCreationMatrix().Translation();
			SendMessageToActor(m_PullingActorHandle, &msg);

//			ActorMsg::C_DynamicObjectSetPhysicsTypeMsg	msg2;			// All trailers are dynamic so they can be hit and moved
//			msg2.m_PhysType = Game::E_PHYSICS_BEHAVIOUR_DYNAMIC;		// Doesn't work because spline following and ground alignment overrides impact effects
//			SendMessageToActor(GetActor()->GetHandle(), &msg2);
		}
	}
}

//=========================================================================

void C_MovementIntelligentSplineFollower::MoveToNearestKnot()
{
	m_ArcDistance = 0.0f;

	Maths::HVec3 closestPos;
	const F32 length = m_SplineActor->GetSplineWrapper()->GetCurveEval().GetArcLength();

	m_ArcDistance = Curve::Actor::FindNearestPoint(
		*m_SplineActor->GetSplineWrapper()->GetCurveEval().GetCurve(),
		m_SplineActor->GetSplineWrapper()->GetCurveEval().GetReparam(),
		m_WorldTransform.Translation(),
		closestPos);
			
	ActorMsg::C_ActorMovementSetSlaveMasterOffset msg;
	msg.SetValue(m_ArcDistance);
	for(S32 c=0;c<m_Slaves.Size();c++)
		SendMessageToActor(m_Slaves[c], &msg);
}

//=========================================================================

void C_MovementIntelligentSplineFollower::Animate(const Maths::C_TimeS& timeStep)
{
	Maths::C_QuatTrans	newtransform;
	F32					maxspeed = m_MaxSpeed;						// Local version before any temp mods

	if (!m_SplineActor)									// Forget if no spline to follow
		return;

	if (!m_SnappedToSpline)								// Snap here so children know where we are
	{
		if (GetCreationData()->m_StartFromNearestPoint)
			MoveToNearestKnot();
		m_SplineActor->GetSplineWrapper()->GetForwardFacingSplinePos(m_WorldTransform, m_ArcDistance);
		m_SnappedToSpline = true;
	}

	if (beingpulled)									// If being pulled then don't animate here - Wait for E_ActorLogicMsg_PullerUpdate
		return;

	if (waittime > 0.0f)								// Temporary pause in movement
	{
		waittime -= timeStep.Get();
		maxspeed = 0.0f;
	}
	else
	{
		if (m_SlowDownAtSplineEnds > 0.0f)				// Slow down at ends of spline
		{
			if (m_ArcDistance < m_SlowDownAtSplineEnds)
				maxspeed *= m_ArcDistance / m_SlowDownAtSplineEnds;
			else if ((m_SplineLength-m_ArcDistance) < m_SlowDownAtSplineEnds)
				maxspeed *= (m_SplineLength-m_ArcDistance) / m_SlowDownAtSplineEnds;

			if ((maxspeed < 0.5f) && (m_MaxSpeed > 0.5f))			// Make sure we actually get to the end if accelerating/decelerating (and we're not stationary)
				maxspeed = 0.5f;
		}
	}

	if (m_Velocity > maxspeed)							// Handle any acceleration / Deceleration
	{
		m_Velocity -= m_Acceleration * timeStep.Get();
		if (m_Velocity < maxspeed)
			m_Velocity = maxspeed;
	}
	else if (m_Velocity < maxspeed)
	{
		m_Velocity += m_Acceleration * timeStep.Get();
		if (m_Velocity > maxspeed)
			m_Velocity = maxspeed;
	}

	if (reversing)
		m_ArcDistance -= m_Velocity * timeStep.Get();
	else
		m_ArcDistance += m_Velocity * timeStep.Get();


	if (m_ArcDistance < 0.0f)
	{
		if ((m_MovementType == E_SPLINEMOVEMENTTYPE_PINGPONG) || (m_MovementType == E_SPLINEMOVEMENTTYPE_PINGPONG_REVERSE))
		{
			reversing = false;
			flipdirection = false;
			m_ArcDistance = -m_ArcDistance;
			current_control_element++;
		}

		if (m_WaitAtSplineEnds > 0.0f)
		{
			waittime = m_WaitAtSplineEnds;
			m_Velocity = maxspeed = 0.0f;
		}
	}
//	else if (m_ArcDistance >= m_SplineLength)
	else if (m_ArcDistance > m_SplineLength)
	{
		switch(m_MovementType)
		{
			case E_SPLINEMOVEMENTTYPE_SINGLE:
				m_ArcDistance = m_SplineLength;
				m_MaxSpeed = 0.0f;
				break;

			case E_SPLINEMOVEMENTTYPE_LOOP:
				m_ArcDistance -= m_SplineLength;
				current_control_element = 0;
				break;

			case E_SPLINEMOVEMENTTYPE_PINGPONG:
				m_ArcDistance = m_SplineLength;
				reversing = true;
				flipdirection = true;
				current_control_element--;
				break;

			case E_SPLINEMOVEMENTTYPE_PINGPONG_REVERSE:
				m_ArcDistance = m_SplineLength;
				reversing = true;
				current_control_element--;
				break;

		
		}

		if (m_WaitAtSplineEnds > 0.0f)
		{
			waittime = m_WaitAtSplineEnds;
			m_Velocity = maxspeed = 0.0f;
		}
	}


	m_SplineActor->GetSplineWrapper()->GetForwardFacingSplinePos(newtransform, m_ArcDistance);

	if (m_AlignToGround)
		CheckAlignToGround(&newtransform);

	if (flipdirection ^ m_FlipModelOrientation)						// Display pointing backwards down the spline
		newtransform = MakeQuatTrans( (Quaternion)newtransform.Quaternion() * MakeUnitQuaternion( Const<Normal3>::AxisY(), Const<VF32>::Pi() ), newtransform.Translation() );

	if (m_BankAngle)
	{
		F32	bankangle = Maths::DegToRad(m_BankAngle);
		Maths::VF32 castval = Load<VF32>(&bankangle);
		newtransform = MakeQuatTrans( MakeUnitQuaternion( Const<Normal3>::AxisZ(), castval ) * (Quaternion)newtransform.Quaternion(), newtransform.Translation() );
	}

	if ((m_RotationDamping > 0.0f) && (m_RotationDamping < 1.0f))	// Stop any jiggling
		m_LocalTransform = NearestSlerp( m_LocalTransform, newtransform, Load<VF32>(&m_RotationDamping));
	else
		m_LocalTransform = newtransform;

	m_WorldTransform = m_LocalTransform;			// Done in movement component update!!

	UpdateSlaves(timeStep);

	if (pulling)						// Tell any actors attached to this to move as well
	{
		ActorMsg::C_ActorLogicMsg_PullerUpdate	msg;
		msg.m_splinedistance = m_ArcDistance - pulloffset;
		if (msg.m_splinedistance < 0.0f)
			msg.m_splinedistance = 0.0f;
		msg.m_worldmat = m_LocalTransform.Matrix();			// Quattran to HMat43
		SendMessageToActor(pullhandle, &msg);
	}


	CheckControlElement();
}

//=========================================================================

void C_MovementIntelligentSplineFollower::CheckControlElement()
{
	if (!num_control_elements)				// Just exit if no control data
		return;

	if ((current_control_element < 0) || (current_control_element >= num_control_elements))		// Exit if no more control data in our direction of travel
		return;

	F32		nextdist = GetControlData(current_control_element)->distance;

	if (reversing)
	{
		if (m_ArcDistance <= nextdist)
		{
			ProcessControlElement();
			current_control_element--;
		}
	}
	else
	{
		if (m_ArcDistance >= nextdist)
		{
			ProcessControlElement();
			current_control_element++;
		}
	}
}

//=========================================================================

void C_MovementIntelligentSplineFollower::ProcessControlElement()
{
	S32		a;

	C_MovementControlData*	mcd = GetControlData(current_control_element);

	if (reversing)
	{
		if (mcd->m_Direction == E_KNOTDATADIRECTIONTYPE_FORWARDONLY)
			return;
	}
	else
	{
		if (mcd->m_Direction == E_KNOTDATADIRECTIONTYPE_REVERSEONLY)
			return;
	}

	if (mcd->m_ControlMessages)
		for(a=0;a<mcd->m_ControlMessages.count;a++)
		{
//			OnMessage(mcd->m_ControlMessages[a]->GetMessage(NULL));
			SendMessageToActor(GetActor()->GetHandle(), mcd->m_ControlMessages[a]->GetMessage(NULL));			
		}

	if (mcd->m_EventMessages)
		for(a=0;a<mcd->m_EventMessages.count;a++)
			mcd->m_EventMessages[a].Send(GetActor());
}

//=========================================================================

void C_MovementIntelligentSplineFollower::UpdateSlaves(const Maths::C_TimeS& timeStep)
{
	S32 count = m_Slaves.Size();

	if (count > 0)
	{
		ActorMsg::C_ActorMovementUpdateSlaveMsg msg(timeStep.Get(), m_ArcDistance);
		for(S32 c=0;c<count;c++)
			SendMessageToActor(m_Slaves[c], &msg);
	}
}

//=========================================================================

bool C_MovementIntelligentSplineFollower::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	using namespace Maths;

	switch (pMessage->Type())
	{
		case ActorMsg::E_DynamicObjMsg_SplineCtrl_StopAndWait:
		{
			const ActorMsg::C_SplineCtrl_StopAndWaitMsg* msg = Messaging::MessageCast<ActorMsg::C_SplineCtrl_StopAndWaitMsg>(pMessage);
			waittime = msg->m_WaitTime;
			break;
		}

		case ActorMsg::E_DynamicObjMsg_SplineCtrl_SetSpeed:
		{
			const ActorMsg::C_SplineCtrl_SetSpeedMsg* msg = Messaging::MessageCast<ActorMsg::C_SplineCtrl_SetSpeedMsg>(pMessage);
			if (reversing)
				m_MaxSpeed = msg->m_ReturnSpeed;
			else
				m_MaxSpeed = msg->m_Speed;
			break;
		}

		case ActorMsg::E_DynamicObjMsg_SplineCtrl_ReverseDirection:
		{
//			const ActorMsg::C_SplineCtrl_ReverseDirectionMsg* msg = Messaging::MessageCast<ActorMsg::C_SplineCtrl_ReverseDirectionMsg>(pMessage);

			if (reversing)
			{
				reversing = false;
				current_control_element++;
			}
			else
			{
				reversing = true;
				current_control_element--;
			}
			break;
		}

		case ActorMsg::E_DynamicObjMsg_SplineCtrl_SetBankAngle:
		{
			const ActorMsg::C_SplineCtrl_SetBankAngleMsg* msg = Messaging::MessageCast<ActorMsg::C_SplineCtrl_SetBankAngleMsg>(pMessage);
			if (reversing)
				m_BankAngle = msg->m_ReturnAngle;
			else
				m_BankAngle = msg->m_Angle;
			break;
		}


		case ActorMsg::E_MovementMsg_AddSplineSlave:
		{
			const ActorMsg::C_ActorMovementAddSplineSlaveMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementAddSplineSlaveMsg>(pMessage);
			assert(Btl::Find(m_Slaves, msg->GetActorHandle()) == m_Slaves.End());	// Already in list!
			m_Slaves.PushBack(msg->GetActorHandle());
			break;
		}

		case ActorMsg::E_MovementMsg_RemoveSplineSlave:
		{
			const ActorMsg::C_ActorMovementRemoveSplineSlaveMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementRemoveSplineSlaveMsg>(pMessage);
			Btl::Vector<Actor::C_ActorHandle>::Iterator it = Btl::Find(m_Slaves, msg->GetActorHandle());
			if (it != m_Slaves.End())
				m_Slaves.Erase(it);
			break;
		}

		case ActorMsg::E_ActorLogicMsg_AttachToPuller:		// Another component has attached to this one
		{
			const ActorMsg::C_ActorLogicMsg_AttachToPuller* msg = Messaging::MessageCast<ActorMsg::C_ActorLogicMsg_AttachToPuller>(pMessage);

			pulling = true;
			Maths::HVec3 offset = msg->m_worldpos - GetActor()->GetCreationData()->GetCreationMatrix().Translation();
			pulloffset = offset.Length();
			pullhandle = msg->m_handle;
			break;
		}

		case ActorMsg::E_ActorLogicMsg_PullerUpdate:		// This actor has been told to move by a pulling actor
		{
			const ActorMsg::C_ActorLogicMsg_PullerUpdate* msg = Messaging::MessageCast<ActorMsg::C_ActorLogicMsg_PullerUpdate>(pMessage);
			Maths::C_QuatTrans	newtransform;

			if (m_UsePivot)
			{
				Maths::HVec3 pivotpos = msg->m_worldmat.Translation() - (msg->m_worldmat.AZ() * m_OriginToPivotDistForPuller);
				Maths::HVec3 pulldir = pivotpos - m_LocalTransform.Translation();
				pulldir.Normalise();
				Maths::HVec3 offset = pulldir * m_OriginToPivotDistForThis;

				Maths::HMat43	mat;
				mat.AZ(pulldir);
				mat.AY(Maths::HVec3(0.0f, 1.0f, 0.0f));
				::Actors::C_ActorUtils::Instance().NormaliseMatXYZ(&mat);
				newtransform = MakeQuatTrans( MakeUnitQuaternion(mat.Rotation()), (HVec3)( pivotpos - offset ) );

			}
			else
			{
				m_SplineActor->GetSplineWrapper()->GetForwardFacingSplinePos(newtransform, msg->m_splinedistance);
			}

			if (m_AlignToGround)
				CheckAlignToGround(&newtransform);

			if (flipdirection)												// Display pointing backwards down the spline
				newtransform = MakeQuatTrans( (Quaternion)newtransform.Quaternion() * MakeUnitQuaternion( Const<Normal3>::AxisY(), Const<VF32>::Pi() ), newtransform.Translation() );

			if ((m_RotationDamping > 0.0f) && (m_RotationDamping < 1.0f))	// Stop any jiggling
				m_LocalTransform = NearestSlerp( m_LocalTransform, newtransform, Load<VF32>(&m_RotationDamping));
			else
				m_LocalTransform = newtransform;

			if (pulling)
			{
				ActorMsg::C_ActorLogicMsg_PullerUpdate	msg2;		// Pass on down the chain
				msg2.m_splinedistance = msg->m_splinedistance - pulloffset;
				if (msg2.m_splinedistance < 0.0f)
					msg2.m_splinedistance = 0.0f;
				msg2.m_worldmat = m_LocalTransform.Matrix();			// Quattran to HMat43
				SendMessageToActor(pullhandle, &msg2);
			}
			break;
		}

		case ActorMsg::E_DynamicObjMsg_StartDynamicObjectAnimation:		// These messages need passing on
		{
			if (pulling)								// Pass on these messages to objects we're pulling
				SendMessageToActor(pullhandle, pMessage);
			break;
		}		

		default:
			return(C_MovementBase::OnMessage(pMessage));
	}



	return(true);
}

//=========================================================================

void C_MovementIntelligentSplineFollower::CheckAlignToGround(Maths::C_QuatTrans *qtran)
{
	Maths::HVec3  start = qtran->Translation() + Maths::HVec3(0.0f, 2.0f, 0.0f);
	Maths::HVec3  end = qtran->Translation() + Maths::HVec3(0.0f, -2.0f, 0.0f);
	Maths::HVec3  collision;
	if (::Actors::C_ActorUtils::Instance().CheckCollision(&start, &end, &collision))
		qtran->Translation(collision);
}


//=========================================================================


void C_MovementIntelligentSplineFollower::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << GetCreationData()->m_SplineActor->GetActorHandle();
//	stream << m_MovementType;		// DONT KNOW WHY THIS DOESN'T COMPILE BUT DON'T NEED FOR ANY OF THIS FOR AMAX :)
	stream << m_MaxSpeed;
	stream << m_Acceleration;		
	stream << m_Velocity;
	stream << m_ArcDistance;
	stream << m_SplineLength;
}

//=========================================================================

void C_MovementIntelligentSplineFollower::Deserialise(Sys::C_ContextualStream& stream)
{
	Actor::C_ActorHandle		handle;

	stream >> handle;
	m_SplineActor = ::Actor::ActorCast_Safe< ::Actors::C_SplineActor>(GetActorList()->GetActorPtr(handle));
//	stream >> m_MovementType;		
	stream >> m_MaxSpeed;
	stream >> m_Acceleration;		
	stream >> m_Velocity;
	stream >> m_ArcDistance;
	stream >> m_SplineLength;
}

//=========================================================================

void C_MovementIntelligentSplineFollower::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSave(stream);
	Serialise(stream);
}

//=========================================================================

void C_MovementIntelligentSplineFollower::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSave(stream);
	Deserialise(stream);
}

//=========================================================================

void C_MovementIntelligentSplineFollower::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSync(stream);
	Serialise(stream);
}

//=========================================================================

void C_MovementIntelligentSplineFollower::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSync(stream);
	Deserialise(stream);
}

//=========================================================================

#ifndef CFG_GOLD
void C_MovementIntelligentSplineFollower::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
/*
	if (!g_MovementKinematicSplineTrackerDebugRender)
		return;

	const Actors::C_SplineActor* pSplineActor = Actor::ActorCast_Safe<Actors::C_SplineActor>(GetActorList()->GetActorPtr(m_SplineActorHandle));

	if (!pSplineActor)
		return;

	C_MovementKinematicSpline::DebugRender(debugRenderPtr);

	if (m_TrackerActorHandle)
	{
		Maths::HVec3 trackerPos = pSplineActor->GetSplineWrapper()->GetCurveEval().EvaluateArcLength(m_TrackerActorPos);
		debugRenderPtr->Draw3dPoint(trackerPos, 1.0f, Maths::RGBA(0xff, 0xff, 0, 0xff));
		Util::C_String text;
		F32 relDist = m_ArcDistance - m_TrackerActorPos;
		text.Printf("rel%0.1f des%0.1f\n", relDist, m_IdealDistance);
		text += Util::C_String().Printf("vel%0.1f rd%0.1f", m_TrackerVel, m_ReactionDistance);
		debugRenderPtr->Draw3dText(trackerPos+ Maths::HVec3(0.0f, 2.0f, 0.0f), 0.50f, text);
	}
*/
}
#endif


}// End namespace Components



//#pragma optimize("", on)



//=========================================================================
//							GRAVEYARD
//=========================================================================




