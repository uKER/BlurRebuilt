#include "stdafx.h"


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Impl/MovementComponents.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>

#include <Dynamic/Particle/EventInstance.h>
#include <Dynamic/Particle/EventInstanceHandle.h>
#include <Dynamic/Common.h>
#include <Dynamic/Renderer.h>
#include <Dynamic/Particle/Manager.h>

#include <GameLogic/ActorStudioActors/ActorUtils.h>

#include <GameLogic/ActorStudioActors/EffectGeneratorActor.h>

#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>

#include <Game/GameUtil/ResourceBag.h>

#include <btl/containers/algorithm.h>

#include <actor_studio_msg_game/ActorStudioMessaging.h>

#include <GameApp.h>

#include <misc_shared\Messaging\GameMessaging.h>
#include <shared_actors\Components\Messages\MovementMessages.h>
#include <shared_actors/Utility/VisibilityDetection.h>

Debug::Bool g_debugChangeEffectsOnTheFly("DynamicObjects.ChangeEffectsOnTheFly", false);


//=========================================================================
//		Sorcery definitions
//=========================================================================


XT_TYPE(Actors::E_EFFECT_LOOP_TYPE,					"SFXLoopTypes")
	XT_OPTION(Actors::E_EFFECT_LOOP_TYPE_NONE,		"None")
	XT_OPTION(Actors::E_EFFECT_LOOP_TYPE_PLAYONCE,	"PlayOnce")
	XT_OPTION(Actors::E_EFFECT_LOOP_TYPE_LOOPED,	"Looped")
	XT_OPTION(Actors::E_EFFECT_LOOP_TYPE_PINGPONG,	"PingPong")
XT_END_TYPE


//	Actor creation definition (Sorcery)

XT_TYPE(Actors::EffectGeneratorCreationData,	"EffectGenerator")
	XT_BASE(Actors::C_PosActorCreate)

	XT_FIELD(m_Alignment,						"Alignment")
	XT_FIELD(m_StartDeactivated,				"Deactivate")

	XT_FIELD(m_SpecialEffect,					"SpecialEffect")
	XT_FIELD(m_T1LoopStart,						"T1LoopStart")
	XT_FIELD(m_T2LoopEnd,						"T2LoopEnd")
	XT_FIELD(m_StartTime,						"StartTime")
	XT_FIELD(m_LoopType,						"LoopType")

	XT_FIELD(m_pSorceryMovementComponentCreate,	"MovementComponent")
XT_END_TYPE



//=========================================================================
//					EffectGenerator Actor Core Routines
//=========================================================================



//	Component Creation and Setup


void Actors::EffectGeneratorCreationData::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_PosActorCreate::DoSorceryCallback(pCallbackFn);

	if (m_pSorceryMovementComponentCreate != NULL)
		DoComponentSorceryCallback(pCallbackFn, *m_pSorceryMovementComponentCreate, &m_pMovementCreator);
}



namespace Actors
{
//#pragma optimize("", off)


//=========================================================================
//						STANDARD FUNCTIONS
//=========================================================================

EffectGeneratorActor::EffectGeneratorActor()
{ 
	m_pMovementComponent = NULL;
	m_pSFX = NULL;

	m_pVisDetect = NULL;
	

	m_Activated = false;
	m_bVisible	= false;

	m_PreviousTime = 0.f;
}

EffectGeneratorActor::~EffectGeneratorActor()
{
	DeactivateEffect();
	SAFE_DELETE_PTR(m_pSFX);
	Actors::C_VisibilityDetector::ReleaseVisDetect(m_pVisDetect);
}

bool EffectGeneratorActor::InitialiseImpl()
{
	m_WorldMatrix = GetCreationData()->GetCreationMatrix();				// Need this before components are created
	
	if (GetCreationData()->GetMovementCreator() != NULL)				// Create movement component
	{
		if (!InitialiseComponent(GetCreationData()->m_pMovementCreator))
			return(false);
	}

	return(true);
}

//=========================================================================

bool EffectGeneratorActor::CreateImpl()
{
	pCreationData = GetCreationData();

	m_pMovementComponent = QueryInterface<Components::I_Movement>();

	m_T1LoopStart = pCreationData->m_T1LoopStart;					// All these could change after creation using messages
	m_T2LoopEnd = pCreationData->m_T2LoopEnd;
	m_LoopType = pCreationData->m_LoopType;
	m_FramePos = pCreationData->m_StartTime;
	CheckLoopTimes();

	m_Reverse = false;
	m_Paused = true;
	m_Selected = false;

	C_ActorUtils::Instance().AlignToScene(&m_WorldMatrix, pCreationData->m_Alignment, 3.0f, GetPhysicsScene());		// In case we want to automatically position on the ground

	// m_pSFX might have already been set by our children, so don't override it
	if (!m_pSFX && pCreationData->m_SpecialEffect.Object())
	{
		m_pVisDetect = Actors::C_VisibilityDetector::CreateVisDetect(GetHandle());
		m_pSFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pCreationData->m_SpecialEffect, GetActorList()->GetDefaultContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr(), m_pVisDetect);
	}

	if (!pCreationData->m_StartDeactivated)
		ActivateEffect();

	
	AddSubscriber( ActorMsg::E_DynamicObjMsg_SorceryUpdate );
//	AddSubscriber( ActorMsg::E_DynamicObjMsg_SetAlphaValue );				// This will be targetted at actor and not broadcast

	return(true);
}


//=========================================================================

void EffectGeneratorActor::ActivateEffect()
{
	PERF_EVENT("EffectGeneratorActor.ActivateEffect");

	if (m_pSFX)
	{
		DeactivateEffect();					// Make sure it's off
		
		CheckLoopTimes();
		m_pSFX->Start();
		m_pSFX->UpdateWorldMatrix(m_WorldMatrix);
		m_pSFX->Update(0);

		m_Reverse = false;
		m_Paused = false;

		//Reset time
		m_FramePos = m_T1LoopStart;

//		AddSubscriber( GameMessaging::E_GameMsg_Update );
		AddSubscriber( GameMessaging::E_GameMsg_PostUpdate );		
		m_Activated = true;
	}
}

//=========================================================================

void EffectGeneratorActor::DeactivateEffect()
{
	PERF_EVENT("EffectGeneratorActor.DeactivateEffect");

	if (!m_Paused)
	{
		if (m_pSFX)
			m_pSFX->Stop();
//		RemoveSubscriber( GameMessaging::E_GameMsg_Update );
		RemoveSubscriber( GameMessaging::E_GameMsg_PostUpdate );
		m_Paused = true;
		m_Activated = false;
	}
}

//=========================================================================

bool EffectGeneratorActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	PERF_EVENT("EffectGeneratorActor.MessageHandler");

	switch(pMessage->Type())
	{
	case ::Actors::E_VisibilityMsg_OnHidden:
		if (m_Activated)
		{
			//RemoveSubscriber(ActorMsg::E_GameActorMsg_Activate);
			RemoveSubscriber(GameMessaging::E_GameMsg_PostUpdate);
		}
		m_bVisible = false;
		break;
	case ::Actors::E_VisibilityMsg_OnVisible:
		if (m_Activated)
		{
			//AddSubscriber(ActorMsg::E_GameActorMsg_Activate);
			AddSubscriber(GameMessaging::E_GameMsg_PostUpdate);
		}
		m_bVisible = true;
		break;

//		case GameMessaging::E_GameMsg_Update:
		case GameMessaging::E_GameMsg_PostUpdate:
			{
				const Components::I_Movement* pMovementComponent = m_pMovementComponent;
				if (pMovementComponent)
				{
					if (pMovementComponent->IsAnimating() && pMovementComponent->IsEnabled())
					{
						// Defer model update until the movement component has set our world matrix
						// via AnimateTransform or SetTransform messages.
						return true;
					}
				}

				const GameMessaging::C_PostUpdateMsg* pMsg = Messaging::MessageCast<GameMessaging::C_PostUpdateMsg>(pMessage);
				Update(pMsg->GetValue());
			}
			break;

		case ActorMsg::E_GameActorMsg_Disable:
			DeactivateEffect();
			
			SAFE_DELETE_PTR(m_pSFX);
			
			//Kill();
			break;

		case ActorMsg::E_GameActorMsg_Activate:
			ActivateEffect();
			break;

		case ActorMsg::E_GameActorMsg_Deactivate:
			DeactivateEffect();
			break;

		case ActorMsg::E_DynamicObjMsg_ChangeSpecialEffect:
			{
				const ActorMsg::C_ChangeSpecialEffectMsg* msg = Messaging::MessageCast<ActorMsg::C_ChangeSpecialEffectMsg>(pMessage);

//				DeactivateEffect();
//				m_pSFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pCreationData->m_SpecialEffect, GetSceneGraph());
				m_T1LoopStart = msg->m_T1LoopStart;			// NEED TO CHECK THESE ARE VALID LIKE ABOVE
				m_T2LoopEnd = msg->m_T2LoopEnd;
				m_LoopType = msg->m_LoopType;
				m_FramePos = msg->m_StartTime;
				CheckLoopTimes();
//				m_pSFX->Start();
				m_Reverse = false;
				//m_Paused = false;
				Update(0);
			}
			break;


		case ActorMsg::E_GameActorMsg_SetTransform:
			{
				const ActorMsg::C_ActorSetTransformMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorSetTransformMsg>(pMessage);
				m_WorldMatrix = msg->Transform().Matrix();
				Update(C_GameApp::GetInstance()->GetTimeStep());
			}
			break;

		case ActorMsg::E_GameActorMsg_AnimateTransform:
			{
				const ActorMsg::C_ActorAnimateTransformMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorAnimateTransformMsg>(pMessage);
				m_WorldMatrix = msg->Transform().Matrix();
				Update(C_GameApp::GetInstance()->GetTimeStep());
			}
			break;

		case ActorMsg::E_DynamicObjMsg_SorceryUpdate:
			if (g_debugChangeEffectsOnTheFly)
			{
				DeactivateEffect();
				ActivateEffect();
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

		case ActorMsg::E_DynamicObjMsg_SetAlphaValue:
		{
			const ActorMsg::C_SetAlphaValueMsg* msg = Messaging::MessageCast<ActorMsg::C_SetAlphaValueMsg>(pMessage);
			if (m_Activated)			// Only change if currently active
				m_pSFX->UpdateIntensity(msg->m_alpha);
			break;
		}


		case ActorStudioMsgShared::E_ActorStudioMsg_Drag:
		{
			const ActorStudioMsgGame::C_DragMsg* msg = Messaging::MessageCast<ActorStudioMsgGame::C_DragMsg>(pMessage);
			m_WorldMatrix = msg->GetValue();
		}
		break;

		//case ActorStudioMsgShared::E_ActorStudioMsg_EndDrag:
		//{
		//	
		//}
		//break;

		case ActorStudioMsgShared::E_ActorStudioMsg_Selected:
		{
			m_Selected = true;
		}
		break;

		case ActorStudioMsgShared::E_ActorStudioMsg_Deselected:
		{
			m_Selected = false;
		}
		break;

		default:
			return C_GameActor::OnMessage(pMessage);
	}

	return(true);
}

//=========================================================================
//						ACTOR SPECIFIC
//=========================================================================


void EffectGeneratorActor::Update(float frameTime)					// Update logic called each frame
{
	PERF_EVENT("EffectGeneratorActor.Update");

	if (!m_Paused)					// Don't do anything if no effect
	{
		//if (m_Paused)
		//	frameTime = 0.0f;

		if(m_PreviousTime)
		{
			frameTime = C_GameApp::GetInstance()->GetTimer().GetElapsedTime() - m_PreviousTime;
			frameTime = fmod(frameTime, (m_T2LoopEnd - m_T1LoopStart));
		}

		switch(m_LoopType)
		{
			case Actors::E_EFFECT_LOOP_TYPE_NONE:			// Plays all the way through then deactivates
				m_FramePos += frameTime;
				break;

			case Actors::E_EFFECT_LOOP_TYPE_PLAYONCE:		// Plays until loop end then waits
				m_FramePos += frameTime;
				if (m_FramePos >= m_T2LoopEnd)
				{
					DeactivateEffect();
					m_FramePos = m_T2LoopEnd;
				}
				break;

			case Actors::E_EFFECT_LOOP_TYPE_LOOPED:			// Repeatedly plays from start to end
				m_FramePos += frameTime;
				if (m_FramePos >= m_T2LoopEnd)
					m_FramePos -= m_T2LoopEnd - m_T1LoopStart;
				break;

			case Actors::E_EFFECT_LOOP_TYPE_PINGPONG:		// Ping Pongs between start and end
				if (m_Reverse)
				{
					m_FramePos -= frameTime;
					if (m_FramePos <= m_T1LoopStart)
						m_Reverse = !m_Reverse;
				}
				else
				{
					m_FramePos += frameTime;
					if (m_FramePos >= m_T2LoopEnd)
						m_Reverse = !m_Reverse;
				}
				break;
		}

		if (m_pSFX)
		{
			m_pSFX->UpdateWorldMatrix(m_WorldMatrix);
	//		m_pSFX->Update( m_FramePos );
			if (!m_pSFX->Update( m_FramePos ))
			{
				//if (m_LoopType == Actors::E_EFFECT_LOOP_TYPE_NONE)
				DeactivateEffect();
				//else
				//	m_Paused = true;
			}
		}

		m_PreviousTime=C_GameApp::GetInstance()->GetTimer().GetElapsedTime();
	}

	ActorMsg::C_ActorMovementUpdateMsg msg(frameTime, Maths::C_QuatTrans(m_WorldMatrix));
	for (S32 i = 0; i < m_Children.Size(); i++)
		SendMessageToActor(m_Children[i], &msg);
}


//=========================================================================


void EffectGeneratorActor::CheckLoopTimes()					// Because sometimes the effect is slightly shorter than expected
{
	PERF_EVENT("EffectGeneratorActor.CheckLoopTimes");

	F32		duration;

	if (m_pSFX)
	{
		duration = m_pSFX->GetDuration();
		if (m_T1LoopStart > duration)
			m_T1LoopStart = duration;
		if (m_T2LoopEnd > duration)
			m_T2LoopEnd = duration;
		if (m_FramePos > duration)
			m_FramePos = duration - 0.05f;					// Position it just before end otherwise it will instantly be switched off
	}
}

//#pragma optimize("", on)

void EffectGeneratorActor::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	debugRenderPtr->Draw3dMatrix(m_WorldMatrix, 0.5f);
	if(m_Selected)
		debugRenderPtr->Draw3dText(Maths::HVec3(m_WorldMatrix.Translation()), "Selected");
}

}// End namespace Actors



//=========================================================================
//						Might be useful one day
//=========================================================================




