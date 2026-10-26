#include "stdafx.h"

#include <Game/Game.h>

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>

#include <GameLogic\ActorStudioActors\CutSceneMessages.h>

#include <RaceBook\RaceBookManager.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <Game\GameActors\Drivers\DriverManager.h>
#include <Game\GameActors\Vehicles\VehicleActor.h>

#include "VehicleProxyActor.h"

#include <vehicle\VehicleModel.h>

#include <AI\AI_Messages.h>
#include <shared_actors/Components/Messages/MovementMessages.h>
#include <btl/containers/algorithm.h>

#include <actor_studio_msg_game/ActorStudioMessaging.h>



//=========================================================================
//		Sorcery definitions
//=========================================================================

//#pragma optimize("", off)


//	Actor creation definition (Sorcery)

XT_TYPE(Actors::VehicleProxyCreationData,		"CutSceneVehicleProxyActor")
	XT_BASE(Actors::C_PosActorCreate)

	XT_FIELD(mCharacter,				"Character")
	XT_FIELD(mVehicleIndex,				"VehicleIndex")

//	XT_FIELD(m_pSorceryMovementComponentCreate,	"MovementComponent")
XT_END_TYPE


//=========================================================================
//					Component Creation Setup
//=========================================================================

/*
void Actors::VehicleProxyCreationData::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_GameActorCreate::DoSorceryCallback(pCallbackFn);
//	if (m_pSorceryMovementComponentCreate != NULL)
//		DoComponentSorceryCallback(pCallbackFn, *m_pSorceryMovementComponentCreate, &m_pMovementCreator);
}
*/


namespace Actors
{

#ifndef CFG_GOLD
// Global list of Vehicle Proxy Actors, a workaround for Photo Capture for cut scenes, since you can't iterate through actors:
Btl::Vector<VehicleProxyActor*> g_VPAList;
#endif

//=========================================================================
//					VehicleProxy Actor Core Routines
//=========================================================================






VehicleProxyActor::VehicleProxyActor()
{
}

//=========================================================================

VehicleProxyActor::~VehicleProxyActor()
{
#ifndef CFG_GOLD
	// Remove from the list of Vehicle Proxy Actors on destroy:
	Btl::Vector<VehicleProxyActor*>::Iterator result = Btl::Find(g_VPAList, this);
	if (result != g_VPAList.End())
		g_VPAList.Erase(result);
#endif
}

//=========================================================================

bool VehicleProxyActor::InitialiseImpl()					// Called first
{
//	S32		random_entry;


//	LOGINFO( Resource::g_Log, "Vehicle Proxy init");


	const VehicleProxyCreationData *pCreate = GetCreationData();
	m_WorldMatrix = pCreate->GetCreationMatrix();
	m_VehicleIndex = pCreate->mVehicleIndex;

	pRLC = NULL;
	pVehicleActor = NULL;
//	applyhandbrake = false;
	relative_actor = NULL;
	aggression = 1.0f;
	directcontrol = false;
	manual_controls.Clear();

	HingedPart_Init(&bonnet);
	HingedPart_Init(&door_left);
	HingedPart_Init(&door_right);
	HingedPart_Init(&window_left);
	HingedPart_Init(&window_right);

	enabled = false;

//	if (pCreate->GetMovementCreator() != NULL)				// Create movement component
//		if (!InitialiseComponent(pCreate->m_pMovementCreator))
//			return(false);

	if (!C_GameActor::InitialiseImpl())
		return(false);

	return(true);
}


//=========================================================================

bool VehicleProxyActor::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);

	//AddSubscriber(GameMessaging::E_GameMsg_PostUpdate);			// Only if enabled
	//AddSubscriber(Game::E_GameMsg_PostCreateInit);
	AddSubscriber(ActorMsg::E_CutSceneMsg_InitialiseProxy);	
	AddSubscriber(ActorMsg::E_CutSceneMsg_StartTheRace);	
	AddSubscriber( Game::E_GameflowMessage_ClearStoredActorPtrs );

#ifndef CFG_GOLD
	// Add to the list of Vehicle Proxy Actors:
	g_VPAList.PushBack(this);
#endif
	
	return(true);
}

//=========================================================================

void VehicleProxyActor::ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
{
	switch(result)
	{
		case Resource::E_Notify_DataReady:
			break;
		
		case Resource::E_Notify_DataError:
			break;
		
		case Resource::E_Notify_NoData:
			break;
	}
}

//=========================================================================
//						MESSAGE HANDLERS
//=========================================================================


bool VehicleProxyActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
//	S32		newindex;

	switch(pMessage->Type())
	{		
		case Game::E_GameflowMessage_ClearStoredActorPtrs:
		{
			pRLC = NULL;
		}
		break;

		case ActorMsg::E_CutSceneMsg_StartTheRace:
			directcontrol = false;				// Actually under AI or player control
			break;
	
		case GameMessaging::E_GameMsg_PostUpdate:
		{
			F32 period = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();

//			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(pMessage);		// Pass on this message to rest of game logic
			// can't do this otherwise every proxy will send out another postupdate message!

			Game::Actors::RaceLogicCore* pRLC = GetRLC();
			const Game::Actors::C_VehicleActor *pActor = pRLC ? pRLC->GetVehicleActor(m_VehicleIndex) : 0;
			if (pActor)
				m_WorldMatrix = pActor->GetMatrix();

			// Now update all children:
			ActorMsg::C_ActorMovementUpdateMsg msg(period, Maths::C_QuatTrans(m_WorldMatrix));
			for (S32 i = 0; i < m_Children.Size(); i++)
				SendMessageToActor(m_Children[i], &msg);

			if (directcontrol)
				if (pVehicleActor)
					pVehicleActor->SetOverideControls(manual_controls);

			if (relative_actor)
			{
				Maths::HMat43	parentmat = relative_actor->GetMatrix();
				if (relative_speed_only)
				{
					SetSpeedRelativeToPoint(parentmat.Translation());				// Also send speed message
				}
				else
				{
					AIAmax::AIDriveTowardsPoint	 ai_msg;
					ai_msg.m_pVehicleActor = pVehicleActor;
					ai_msg.m_TargetPoint = Maths::HVec3(parentmat.Translation() + (parentmat.AX() * relative_offset.X()) + (parentmat.AY() * relative_offset.Y()) + (parentmat.AZ() * relative_offset.Z()));
					Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&ai_msg);

					SetSpeedRelativeToPoint(ai_msg.m_TargetPoint);				// Also send speed message
				}
			}

			if (pVehicleActor && HingedPart_Update(&bonnet, period))
				pVehicleActor->SetBonnetPosition(bonnet.current_angle);

			if (pVehicleActor && HingedPart_Update(&door_left, period) || HingedPart_Update(&door_right, period))
				pVehicleActor->SetDoorPosition(door_left.current_angle, door_right.current_angle);

			if (HingedPart_Update(&window_left, period) || HingedPart_Update(&window_right, period))		// Windows don't currently animate
				pVehicleActor->SetWindowPosition(window_left.current_angle, window_right.current_angle);

			break;
		}

		case ActorMsg::E_CutSceneMsg_InitialiseProxy:		// Now does this automatically once everything is loaded
		{
			InitialiseProxy();
			break;
		}

		case ActorMsg::E_CutSceneMsg_EnableDisableProxy:
		{
			ActorMsg::C_EnableDisableProxyMsg *msg=(ActorMsg::C_EnableDisableProxyMsg *)pMessage;
			enabled = msg->mEnable;
			if (enabled)
			{
				InitialiseProxy();
				AddSubscriber(GameMessaging::E_GameMsg_PostUpdate);
			}
			else
			{
				RemoveSubscriber(GameMessaging::E_GameMsg_PostUpdate);
			}
			break;
		}

		case ActorMsg::E_CutSceneMsg_PlaceCarOnPoint:
		{
			ActorMsg::C_PlaceCarOnPointMsg *msg=(ActorMsg::C_PlaceCarOnPointMsg *)pMessage;
			Game::Actors::RaceLogicCore* pRLC = GetRLC();
			if (pVehicleActor)
			{
				if (msg->mUseStartPosition)
				{
					RLStartPosition startPos;
					startPos.Initialise();
					startPos = pRLC->GetStartPosition(m_VehicleIndex);
					Game::Actors::C_VehicleResetAtWorldPositionMsg resetMsg(pVehicleActor->GetHandle(), m_VehicleIndex, startPos.mWorldMatrix, startPos.mWorldMatrix.AZ() * (msg->m_Speed * 0.44704f));			// mph -> m/s
					pVehicleActor->SendMessageToActor(pVehicleActor->GetHandle(), &resetMsg);
				}
				else
				{
					Maths::HMat43 mat;
					mat.SetAxes(	Maths::HVec4(msg->mPoint->m_WorldMatrix.ax.vx, msg->mPoint->m_WorldMatrix.ax.vy, msg->mPoint->m_WorldMatrix.ax.vz, 0.0f),
									Maths::HVec4(msg->mPoint->m_WorldMatrix.ay.vx, msg->mPoint->m_WorldMatrix.ay.vy, msg->mPoint->m_WorldMatrix.ay.vz, 0.0f),
									Maths::HVec4(msg->mPoint->m_WorldMatrix.az.vx, msg->mPoint->m_WorldMatrix.az.vy, msg->mPoint->m_WorldMatrix.az.vz, 0.0f),
									Maths::HVec4(msg->mPoint->m_WorldMatrix.aw.vx, msg->mPoint->m_WorldMatrix.aw.vy, msg->mPoint->m_WorldMatrix.aw.vz, 1.0f));

					Game::Actors::C_VehicleResetAtWorldPositionMsg resetMsg(pVehicleActor->GetHandle(), m_VehicleIndex, mat, mat.AZ() * (msg->m_Speed * 0.44704f));			// mph -> m/s
					pVehicleActor->SendMessageToActor(pVehicleActor->GetHandle(), &resetMsg);
				}
			}
			
			if (msg->m_Speed == 0.0f)					// Automatically put the handbrake on if we're not moving
				manual_controls.handbrake = 1.0f;

			directcontrol = true;
			break;
		}

		case ActorMsg::E_CutSceneMsg_SetAIOnSpline:
		{
			ActorMsg::C_SetAIOnSplineMsg *msg=(ActorMsg::C_SetAIOnSplineMsg *)pMessage;

			if (msg->mSplineToFollow)
			{
				if (msg->mSplineToFollow->m_SplineData.knots.count < 4)
					LOGWARN( Resource::g_Log, "AI splines need to have more than 4 knots");

				AIAmax::AISetIntroOutroSpline set_intro_spline_msg;
				set_intro_spline_msg.m_pVehicleActor=pVehicleActor;
				set_intro_spline_msg.m_SplineName=Btl::String32(msg->mSplineToFollow->GetName());
				Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&set_intro_spline_msg);

				AIAmax::AISetIntroOutroSpeed set_intro_speed_msg;
				set_intro_speed_msg.m_pVehicleActor=pVehicleActor;
				set_intro_speed_msg.m_speed=(F32)msg->mSplineToFollow->m_MPH;
				Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&set_intro_speed_msg);

				if (pVehicleActor)				// Clear any vehicle control settings just to be sure
				{
					manual_controls.Clear();
					pVehicleActor->SetOverideControls(manual_controls);
				}
				directcontrol = false;
			}
			else
			{
				LOGWARN( Resource::g_Log, "Attempt to place a vehicle on a null spline");
			}
			break;
		}

		case ActorMsg::E_CutSceneMsg_SetAISpeed:
		{
			ActorMsg::C_SetAISpeedMsg *msg = (ActorMsg::C_SetAISpeedMsg *)pMessage;

			AIAmax::AISetIntroOutroSpeed set_intro_speed_msg;
			set_intro_speed_msg.m_pVehicleActor = pVehicleActor;
			set_intro_speed_msg.m_speed = msg->m_Speed;
			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&set_intro_speed_msg);

			relative_actor = NULL;
			break;
		}

		case ActorMsg::E_CutSceneMsg_SetAIRelativeSpeed:
		{
			ActorMsg::C_SetAIRelativeSpeedMsg *msg = (ActorMsg::C_SetAIRelativeSpeedMsg *)pMessage;

			if (msg->m_RelativeActor)
			{
				Actor::C_ActorHandle handle = msg->m_RelativeActor->GetActorHandle();
				if (handle)
					relative_actor = (Actor::C_Actor*)GetActorList()->GetActorPtr(handle);
				aggression = msg->m_Aggression;
				if (aggression == 0.0f)					// Use default if they've not set aggression up
					aggression = 1.0f;

				relative_speed_only = true;

				if (pVehicleActor)					// Clear any vehicle control settings just to be sure
				{
					manual_controls.Clear();
					pVehicleActor->SetOverideControls(manual_controls);
				}
				directcontrol = false;
			}

			break;
		}


		case ActorMsg::E_CutSceneMsg_ApplyImpulse:
			{
				if (!pVehicleActor)
					break;
				ActorMsg::C_CSApplyImpulseMsg *msg = (ActorMsg::C_CSApplyImpulseMsg *)pMessage;

				CheckReferences();

				if (msg->m_ApplyLinearSpeed)
					pVehicleActor->GetCollisionBody()->LinearVelocity(Maths::HVec3(msg->m_LinearSpeed.vx, msg->m_LinearSpeed.vy, msg->m_LinearSpeed.vz));
				if (msg->m_ApplyAngularSpeed)
					pVehicleActor->GetCollisionBody()->AngularVelocity(Maths::HVec3(msg->m_AngularSpeed.vx, msg->m_AngularSpeed.vy, msg->m_AngularSpeed.vz));
			}
			break;

		case ActorMsg::E_CutSceneMsg_RepairAllDamage:
			{
				CheckReferences();
			}
			break;


		case ActorMsg::E_CutSceneMsg_SetDriverAnim:
			{
				if (!pVehicleActor)
					break;
				ActorMsg::C_SetDriverAnimMsg *msg=(ActorMsg::C_SetDriverAnimMsg *)pMessage;
				C_DriverManager::Instance()->OnPlayDriverAnimation(pVehicleActor->GetVehicleIndex(), msg);
			}
			break;

		case ActorMsg::E_CutSceneMsg_SetCarBehaviour:
			{
				if (!pVehicleActor)
					break;
				ActorMsg::C_SetCarBehaviourMsg *msg=(ActorMsg::C_SetCarBehaviourMsg *)pMessage;

				if(msg->mDoCarModelBehaviour.mHeadlights.mDoIt)
				{
					pVehicleActor->GetVehicleModel()->GetModelInput()->m_bHeadLights.m_Current=msg->mDoCarModelBehaviour.mHeadlights.mValue ? true : false;
				}
				if(msg->mDoCarModelBehaviour.mHeadlightCones.mDoIt)
				{
					pVehicleActor->GetVehicleModel()->GetModelInput()->m_bHeadLightCones.m_Current=msg->mDoCarModelBehaviour.mHeadlightCones.mValue ? true : false;
				}
				if(msg->mDoCarModelBehaviour.mBrake.mDoIt)
				{
					pVehicleActor->GetVehicleModel()->GetModelInput()->m_fBrake=msg->mDoCarModelBehaviour.mBrake.mValue;
				}
				if(msg->mDoCarModelBehaviour.mFoglights.mDoIt)
				{
					pVehicleActor->GetVehicleModel()->GetModelInput()->m_bFogLights.m_Current=msg->mDoCarModelBehaviour.mFoglights.mValue ? true : false;
				}
				if(msg->mDoCarModelBehaviour.mHandbrake.mDoIt)
				{
					pVehicleActor->GetVehicleModel()->GetModelInput()->m_bHandbrake=msg->mDoCarModelBehaviour.mHandbrake.mValue ? true : false;
				}
				if(msg->mDoCarModelBehaviour.mRearlights.mDoIt)
				{
					pVehicleActor->GetVehicleModel()->GetModelInput()->m_bRearLights.m_Current=msg->mDoCarModelBehaviour.mRearlights.mValue ? true : false;
				}
				if(msg->mDoCarModelBehaviour.mRevs.mDoIt)
				{
					pVehicleActor->GetVehicleModel()->GetModelInput()->m_fRevs=msg->mDoCarModelBehaviour.mRevs.mValue;
				}
				if(msg->mDoCarModelBehaviour.mSteering.mDoIt)
				{
					pVehicleActor->GetVehicleModel()->GetModelInput()->m_fSteering=msg->mDoCarModelBehaviour.mSteering.mValue;
				}
				if(msg->mDoCarModelBehaviour.mThrottle.mDoIt)
				{
					pVehicleActor->GetVehicleModel()->GetModelInput()->m_fThrottle=msg->mDoCarModelBehaviour.mThrottle.mValue;
				}
			}

			break;

		case ActorMsg::E_MovementMsg_AddChild:
			{
				const ActorMsg::C_ActorMovementAddChildMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementAddChildMsg>(pMessage);
				assert(Btl::Find(m_Children, msg->GetActorHandle()) == m_Children.End());	// Already in list!
				m_Children.PushBack(msg->GetActorHandle());
				break;
			}

		case ActorMsg::E_MovementMsg_RemoveChild:
			{
				const ActorMsg::C_ActorMovementRemoveChildMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementRemoveChildMsg>(pMessage);
				Btl::Vector<Actor::C_ActorHandle>::Iterator it = Btl::Find(m_Children, msg->GetActorHandle());
				if (it != m_Children.End())
					m_Children.Erase(it);
				break;
			}


// NEW MESSAGES

		case ActorMsg::E_CutSceneMsg_Damage_EnableDisable:
		{
			if (!pVehicleActor)
				break;
			const ActorMsg::C_Damage_EnableDisableMsg* msg = Messaging::MessageCast<ActorMsg::C_Damage_EnableDisableMsg>(pMessage);
			pVehicleActor->Damage_EnableDisable( msg->mRecieveDamage );
			break;
		}

		case ActorMsg::E_CutSceneMsg_Damage_Apply:
		{
			if (!pVehicleActor)
				break;
			const ActorMsg::C_Damage_ApplyMsg* msg = Messaging::MessageCast<ActorMsg::C_Damage_ApplyMsg>(pMessage);
			pVehicleActor->Damage_Apply(msg->mDamageState);
			break;
		}

		case ActorMsg::E_CutSceneMsg_Damage_Repair:
		{
			if (!pVehicleActor)
				break;
//			const ActorMsg::C_Damage_RepairMsg* msg = Messaging::MessageCast<ActorMsg::C_Damage_RepairMsg>(pMessage);
			pVehicleActor->Damage_Repair();
			break;
		}

//-------------

		case ActorMsg::E_CutSceneMsg_DriveRelative:
		{
			const ActorMsg::C_CutScene_DriveRelativeMsg* msg = Messaging::MessageCast<ActorMsg::C_CutScene_DriveRelativeMsg>(pMessage);

			if (msg->mParentActor)
			{
				Actor::C_ActorHandle handle = msg->mParentActor->GetActorHandle();
				if (handle)
					relative_actor = (Actor::C_Actor*)GetActorList()->GetActorPtr(handle);
				relative_offset = Maths::HVec3(msg->mParentOffset.vx, msg->mParentOffset.vy, msg->mParentOffset.vz);
				aggression = msg->mAggression;
				if (aggression == 0.0f)					// Use default if they've not set aggression up
					aggression = 1.0f;

				relative_speed_only = false;

				if (pVehicleActor)					// Clear any vehicle control settings just to be sure
				{
					manual_controls.Clear();
					pVehicleActor->SetOverideControls(manual_controls);
				}
				directcontrol = false;
			}
			break;
		}

		case ActorMsg::E_CutSceneMsg_DisableAI:
		{
//			const ActorMsg::C_CutScene_DetachAIFromSplineMsg* msg = Messaging::MessageCast<ActorMsg::C_CutScene_DetachAIFromSplineMsg>(pMessage);

			AIAmax::AIDisableAI	 ai_msg;
			ai_msg.m_pVehicleActor = pVehicleActor;
			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&ai_msg);
			relative_actor = NULL;
//			directcontrol = true;
//			manual_controls.Clear();
			break;
		}

//-------------

		case ActorMsg::E_CutSceneMsg_SetVehicleControl:
		{
			const ActorMsg::C_CutScene_SetVehicleControlMsg* msg = Messaging::MessageCast<ActorMsg::C_CutScene_SetVehicleControlMsg>(pMessage);
			if (msg->mDirectControl)
				directcontrol = true;
			else
				directcontrol = false;
			break;
		}
		
		case ActorMsg::E_CutSceneMsg_SetVehicleHandbrake:
		{
			const ActorMsg::C_CutScene_SetVehicleHandbrakeMsg* msg = Messaging::MessageCast<ActorMsg::C_CutScene_SetVehicleHandbrakeMsg>(pMessage);
//			applyhandbrake =  msg->mApplyHandbrake;
			if (msg->mApplyHandbrake)
				manual_controls.handbrake = 1.0f;
			else
				manual_controls.handbrake = 0.0f;
			directcontrol = true;
			break;
		}

/*
		case ActorMsg::E_CutSceneMsg_SetVehicleSpeed:
		{
			const ActorMsg::C_CutScene_SetVehicleSpeedMsg* msg = Messaging::MessageCast<ActorMsg::C_CutScene_SetVehicleSpeedMsg>(pMessage);
//			pVehicleActor->SetVehicleSpeed( msg->mSpeed );
			directcontrol = true;
			break;
		}
*/

		case ActorMsg::E_CutSceneMsg_SetVehicleRevs:
		{
			const ActorMsg::C_CutScene_SetVehicleRevsMsg* msg = Messaging::MessageCast<ActorMsg::C_CutScene_SetVehicleRevsMsg>(pMessage);
//			pVehicleActor->SetVehicleRevs( msg->mRevs );
			manual_controls.throttle = msg->mRevs;
			directcontrol = true;
			break;
		}

		case ActorMsg::E_CutSceneMsg_SetVehicleBrake:
		{
			const ActorMsg::C_CutScene_SetVehicleBrakeMsg* msg = Messaging::MessageCast<ActorMsg::C_CutScene_SetVehicleBrakeMsg>(pMessage);
//			pVehicleActor->SetVehicleBrake( msg->mBrake );
			manual_controls.brake = msg->mBrake;
			directcontrol = true;
			break;
		}

		case ActorMsg::E_CutSceneMsg_SetVehicleSteering:
		{
			const ActorMsg::C_CutScene_SetVehicleSteeringMsg* msg = Messaging::MessageCast<ActorMsg::C_CutScene_SetVehicleSteeringMsg>(pMessage);
//			pVehicleActor->SetVehicleSteering( msg->mSteer );
			manual_controls.steer = msg->mSteer;
			directcontrol = true;
			break;
		}

//-------------

		case ActorMsg::E_CutSceneMsg_SetDoorPosition:
		{
			const ActorMsg::C_CutScene_SetDoorPositionMsg* msg = Messaging::MessageCast<ActorMsg::C_CutScene_SetDoorPositionMsg>(pMessage);
			HingedPart_Move(&door_left, msg->mLeftPosition, msg->mLeftPositionAnimTime);
			HingedPart_Move(&door_right, msg->mRightPosition, msg->mRightPositionAnimTime);
			break;
		}

		case ActorMsg::E_CutSceneMsg_SetWindowPosition:
		{
			const ActorMsg::C_CutScene_SetWindowPositionMsg* msg = Messaging::MessageCast<ActorMsg::C_CutScene_SetWindowPositionMsg>(pMessage);
			HingedPart_Move(&window_left, msg->mLeftPosition, msg->mLeftPositionAnimTime);
			HingedPart_Move(&window_right, msg->mRightPosition, msg->mRightPositionAnimTime);
			break;
		}

		case ActorMsg::E_CutSceneMsg_SetBonnetPosition:
		{
			const ActorMsg::C_CutScene_SetBonnetPositionMsg* msg = Messaging::MessageCast<ActorMsg::C_CutScene_SetBonnetPositionMsg>(pMessage);
			HingedPart_Move(&bonnet, msg->mBonnetPosition, msg->mBonnetPositionAnimTime);
			break;
		}

		case ActorMsg::E_CutSceneMsg_SetLightState:
		{
			if (!pVehicleActor)
				break;
			const ActorMsg::C_CutScene_SetLightStateMsg* msg = Messaging::MessageCast<ActorMsg::C_CutScene_SetLightStateMsg>(pMessage);
			pVehicleActor->SetLightState( msg->mLeftIndicatorOn, msg->mRightIndicatorOn, msg->mHeadlightsOn, msg->mBrakeLightsOn, msg->mReverseLightsOn);
			break;
		}

		case ActorMsg::E_CutSceneMsg_DisplayVehicle:
		{
			if (!pVehicleActor)
				break;
			const ActorMsg::C_CutScene_DisplayVehicleMsg* msg = Messaging::MessageCast<ActorMsg::C_CutScene_DisplayVehicleMsg>(pMessage);
			pVehicleActor->DisplayVehicle( msg->mDisplayVehicle, true);
			break;
		}

		case ActorMsg::E_CutSceneMsg_DisplayDriver:
		{
			if (!pVehicleActor)
				break;
			const ActorMsg::C_CutScene_DisplayDriverMsg* msg = Messaging::MessageCast<ActorMsg::C_CutScene_DisplayDriverMsg>(pMessage);
			pVehicleActor->DisplayDriver( msg->mDisplayDriver, true);
			break;
		}

		case ActorMsg::E_CutSceneMsg_CreateVehicleEffects:
		{
			if (!pVehicleActor)
				break;
//			const ActorMsg::C_CutScene_CreateVehicleEffectsMsg* msg = Messaging::MessageCast<ActorMsg::C_CutScene_CreateVehicleEffectsMsg>(pMessage);
			pVehicleActor->CreateVehicleEffects();
			break;
		}

		case ActorMsg::E_CutSceneMsg_SwapVehicleModel:
		{
			if (!pVehicleActor)
				break;
//			const ActorMsg::C_CutScene_SwapVehicleModelMsg* msg = Messaging::MessageCast<ActorMsg::C_CutScene_SwapVehicleModelMsg>(pMessage);
			pVehicleActor->SwapVehicleModel();
			break;
		}

//-------------

		case ActorStudioMsgShared::E_ActorStudioMsg_Drag:
		case ActorStudioMsgShared::E_ActorStudioMsg_EndDrag:
		case ActorStudioMsgShared::E_ActorStudioMsg_Selected:
		case ActorStudioMsgShared::E_ActorStudioMsg_Deselected:
			if (!pVehicleActor)
				break;
			SendMessageToActor(pVehicleActor->GetHandle(), pMessage);
			break;


		default:
			break;
	}

	return(true);
}

//=========================================================================


void VehicleProxyActor::InitialiseProxy()
{
	S32		newindex;

	pRLC = GetRLC();
	const VehicleProxyCreationData *pCreate = GetCreationData();
	if (pCreate->mCharacter)
	{
		RaceBook::C_RaceBookProfile* prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)pCreate->mCharacter->ID);
		if(prof)
		{
			newindex = prof->mCurrentCarIdx;
			if (newindex == -1)
			{
				if (m_VehicleIndex < RaceBook::C_RaceBookManager::Instance().GetNumVehicles())		// Don't warn if this is a standard script that always needs 20 vehicles
					LOGWARN( Resource::g_Log, "Vehicle Proxy %d - Unknown profile", m_VehicleIndex);
			}
			else
			{
				m_VehicleIndex = newindex;
			}
		}
	}
	pVehicleActor =  (Game::Actors::C_VehicleActor *) pRLC->GetVehicleActor(m_VehicleIndex);

	if (pVehicleActor)
	{
		pVehicleActor->SetProxyActor(GetHandle());
		m_WorldMatrix = pVehicleActor->GetMatrix();
		ActorMsg::C_ActorMovementUpdateMsg msg(0.001f, Maths::C_QuatTrans(m_WorldMatrix));
		for (S32 i = 0; i < m_Children.Size(); i++)
			SendMessageToActor(m_Children[i], &msg);
	}
//	else
//	{					// Always triggers for races with less than 20 vehicles
//		LOGERROR( Resource::g_Log, "Vehicle Proxy %d can't find it's vehicle actor", m_VehicleIndex);
//	}
}


//=========================================================================

Game::Actors::RaceLogicCore* VehicleProxyActor::GetRLC()
{
	return C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
}

//=========================================================================

void VehicleProxyActor::CheckReferences()	// TEMP WARNING
{
	if (!pRLC || !pVehicleActor)
	{ 
		pRLC = GetRLC();				// Done everyframe until proxys get initialised correctly in every cutscene
		pVehicleActor =  (Game::Actors::C_VehicleActor *) pRLC->GetVehicleActor(m_VehicleIndex);
		LOGWARN( Resource::g_Log, "VEHICLE PROXY %d HASN'T BEEN INITIALISED", m_VehicleIndex);
	}
}

//=========================================================================

void VehicleProxyActor::SetSpeedRelativeToPoint(Maths::HVec3 pos)
{
	Maths::HVec3	reloffset	= pos - m_WorldMatrix.Translation();
	F32				actualdist	= reloffset.Length();
	F32				dotprod = reloffset | m_WorldMatrix.AZ();			// Are we pointing towards the point


	if ((dotprod < 0) || (actualdist < 2.0f))		// 2m null zone to stop overshhots if we're slightly off target
	{
		manual_controls.brake = 1.0f;			// Overshot so take control and slam the brakes on
		manual_controls.handbrake = 1.0f;
		manual_controls.steer = 0.0f;
		directcontrol = true;

		AIAmax::AIDisableAI	 ai_msg;
		ai_msg.m_pVehicleActor = pVehicleActor;
		Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&ai_msg);
		relative_actor = NULL;
	}
	else
	{
		AIAmax::AISetIntroOutroSpeed speed_msg;
		speed_msg.m_pVehicleActor = pVehicleActor;
		speed_msg.m_speed = actualdist * aggression;			// Quick speed calc for now (at 10m away go at 10m/s * any aggression)
		Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&speed_msg);
	}
}

/*
void VehicleProxyActor::SetSpeedRelativeToPoint(Maths::HVec3 pos)
{
	Maths::HVec3	reloffset	= pos - m_WorldMatrix.Translation();
	F32				actualdist	= reloffset.Length();
	F32				dotprod = reloffset | m_WorldMatrix.AZ();			// Are we pointing towards the point


	AIAmax::AISetIntroOutroSpeed speed_msg;
	speed_msg.m_pVehicleActor = pVehicleActor;
	if ((dotprod < 0) || (actualdist < 2.0f))		// 2m null zone to stop overshhots if we're slightly off target
	{
		speed_msg.m_speed = 0.0f;					// Just roll to a stop if we're ahead
	}
	else
	{
		speed_msg.m_speed = actualdist * aggression;			// Quick speed calc for now (at 10m away go at 10m/s * any aggression)
	}
	Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&speed_msg);
}
*/

//=========================================================================
/*
void VehicleProxyActor::Update()					// Update logic called each frame
{
}

//=========================================================================

void VehicleProxyActor::KillThis()					// Cleans up any children etc before killing itself
{
	Kill();
}
*/

//=========================================================================
//						Hinged Parts
//=========================================================================

void VehicleProxyActor::HingedPart_Init(VehicleProxyHingedPart *part)
{
	part->animating = false;
	part->current_angle = 0.0f;
}

//=========================================================================

void VehicleProxyActor::HingedPart_Move(VehicleProxyHingedPart *part, F32 deg, F32 duration)
{
	part->animating = true;
	if (duration <= 0.0f)
	{
		part->current_angle = deg;
		part->time_left = 0.0f;
		part->angle_mod = 0.0f;
		return;
	}

	part->time_left = duration;
	part->angle_mod = (deg - part->current_angle) / duration;
}

//=========================================================================

bool VehicleProxyActor::HingedPart_Update(VehicleProxyHingedPart *part, F32 period)
{
	if (!part->animating)
		return(false);

	part->current_angle += part->angle_mod * period;
	part->time_left -= period;
	if (part->time_left <= 0.0f)
	{
//		if (part->current_angle < 10.0f)		// Correct for any rounding errors and make sure the part is fully closed
//			part->current_angle = 0.0f;			// Had to remove because of windows being 0 to 1
		part->animating = false;
	}

	return(true);								// Return true, even if just stopped, so new angle gets sent to vehicle
}



}// End namespace Actors



//=========================================================================
//						Might be useful one day
//=========================================================================

//#pragma optimize("", on)

//		LOGINFO( Resource::g_Log, "THIS IS A DEBUG MESSAGE\n");

/*
	Game::Actors::C_VehicleForceAIControlMsg controlMsg(pActor->mVehicleIndex);
	pActor->SendMessageToActor(pActor->GetHandle(), &controlMsg);
*/

