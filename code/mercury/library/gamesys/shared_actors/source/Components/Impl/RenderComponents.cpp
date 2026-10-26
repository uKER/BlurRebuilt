#include "SharedActorsPch.h"

#include <shared_actors/Components/Impl/RenderComponents.h>

#include <actor_system/Actor.h>
#include <actor_system/ActorDebug.h>
#include <actor_system/ActorList.h>
#include <core/xtstdx.h>
#include <game_context/RenderContext.h>
#include <game_resource/ModelData.h>
#include <misc_shared/messaging/MessageTypeList.h>
#include <portal/Blocker.h>
#include <portal/System.h>
#include <resource_utils/GameModel.h>
#include <scenegraph/RenderingData/PortalCullNode.h>
#include <scenegraph/SceneGraph.h>
#include <shared_actors/Components/Impl/BaseElementManagerComponents.h>
#include <shared_actors/Components/Interfaces/I_ElementManager.h>
#include <shared_actors/Components/Interfaces/I_ModelCustomiser.h>
#include <shared_actors/Components/Interfaces/I_ModelResource.h>
#include <shared_actors/Components/Interfaces/I_AnimatedShaderParams.h>
#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/SharedActorMessages.h>
#include <shared_actors/Utility/ModelResourceUsage.h>
#include <world_render/EnvironmentSampler.h>
#include <world_render/ModelInstance.h>
#include <world_render/MaterialParameter.h>
#include <world_render/RenderModelData.h>
#include <world_render/Scene.h>

// #include "GameActors/Components/Interfaces/I_DamageObjectComponents.h"
#include <shared_actors/Components/Messages/RenderMessages.h>
// #include "GameApp.h"
#include "shared_Actors/Utility/LightSamplerUtils.h"
// #include "GameWorld/GameWorldLayer.h"


#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", off)
#endif

Debug::Bool	gSingleModelRenderDebugRender("ActorDebug.Components.SingleModelRenderComponent.DebugRender.Enable", false);

namespace Components
{

// Render Component Base Class
// ---------------------------

bool C_RenderComponent::InitialiseImpl()
{
	const C_RenderComponentCreateInfo *pRenderComponentCreate = (const C_RenderComponentCreateInfo *)GetCreator().GetData();

	SetRenderMatrix(pRenderComponentCreate->m_WorldMatrix);

	return true;
}

bool C_RenderComponent::CreateImpl()
{
	const C_RenderComponentCreateInfo *pRenderComponentCreate = (const C_RenderComponentCreateInfo *)GetCreator().GetData();

// 	SetObjectBoundingSphere(pRenderComponentCreate->m_BoundingSphere);
// 
// 	// default to start rendering
// 	if(pRenderComponentCreate->m_bStartHidden == false)
// 		SetRender(true);

	return true;
}

ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN(C_RenderComponent)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(E_RenderMsg_Begin, E_RenderMsg_End)
ACTOR_COMPONENT_MESSAGE_TYPES_END(C_RenderComponent, I_RenderComponent)

bool C_RenderComponent::OnMessage(const Messaging::C_MessageBase *pMessage )	
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_GameActorMsg_Enable:	
	case E_RenderMsg_Show:
		if (m_bHidden)
		{
			m_bHidden = false;
			Show();
		}
		break;

	case ActorMsg::E_GameActorMsg_Disable:
	case E_RenderMsg_Hide:
		if (!m_bHidden)
		{
			Hide();
			m_bHidden = true;
		}
		break;
	case E_RenderMsg_SetMatrix:
		{
			const C_SetRenderMatrixMsg *pSetMatrixMsg = Messaging::MessageCast<C_SetRenderMatrixMsg>(pMessage);
			SetRenderMatrix(pSetMatrixMsg->GetMatrix());
			break;
		}
	
	default:
		return I_RenderComponent::OnMessage(pMessage);
		break;
	}
	return true;
}

void C_RenderComponent::Serialise(Sys::C_ContextualStream& stream) const
{

}

void C_RenderComponent::Deserialise(Sys::C_ContextualStream& stream)
{

}

void C_RenderComponent::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	I_RenderComponent::SerialiseForSave(stream);
	Serialise(stream);

	//stream << FlagRender();
	stream << m_bHidden;
}

void C_RenderComponent::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	I_RenderComponent::DeserialiseForSave(stream);
	Deserialise(stream);

	//SetRender(stream.ReadAs<bool>());
	stream >> m_bHidden;
}

void C_RenderComponent::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	I_RenderComponent::SerialiseForSync(stream);
	Serialise(stream);

	stream << m_bHidden;
}

void C_RenderComponent::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	I_RenderComponent::DeserialiseForSync(stream);
	Deserialise(stream);

	bool bHidden;
	stream >> bHidden;

	if(bHidden && !m_bHidden)
	{
		SendMessageToComponent(E_IF_Render, E_RenderMsg_Hide);
	}

	if(!bHidden && m_bHidden)
	{
		SendMessageToComponent(E_IF_Render, E_RenderMsg_Show);
	}

	m_bHidden = bHidden;
}


// Render Model Base Class
// -----------------------

C_RenderModelBase::C_RenderModelBase():m_pModelInstance(NULL), m_pEnvSampler(NULL)
{
}

C_RenderModelBase::~C_RenderModelBase()
{
	SAFE_DELETE_PTR( m_pEnvSampler );
}

bool C_RenderModelBase::CreateImpl()
{
 	if(C_RenderComponent::CreateImpl())
 	{
		m_pEnvSampler = LightSampler::NewInitialisedEnvironmentSampler(GetActor()->GetContext<GameContext::I_RenderContext>()->GetLevelSceneManager());

 		m_HasShadow = GetCreationData()->m_HasShadow;

		//m_pModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, E_RenderPass_CustomSolid);

#ifndef CFG_GOLD
		AddSubscriber(E_RenderMsg_RequestResourceUsage);
		AddSubscriber(E_RenderMsg_DumpResourceUsage);
#endif 

		return true;
 	}

	return false;
}

void C_RenderModelBase::KillImpl()
{
	if( m_pModelInstance )
	{
		SceneGraph::C_SceneGraph *pSceneGraph = GetActor()->GetContext<GameContext::I_RenderContext>()->GetSceneGraph();
		if(pSceneGraph)
			SceneGraph::Remove(pSceneGraph, m_pModelInstance);
	
		SAFE_DELETE_PTR(m_pModelInstance);
	}
}


WorldRender::MaterialParameter C_RenderModelBase::GetMaterialParameter(const Util::C_String& paramName) const
{
	return m_pModelInstance->GetRenderModel()->GetParameter(paramName);
}

WorldRender::C_ModelInstance* C_RenderModelBase::GetModelInstance() const
{
	return m_pModelInstance;
}

// message handler
ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN(C_RenderModelBase)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(ActorMsg::E_GameActorMsg_Enable, ActorMsg::E_GameActorMsg_Disable)
ACTOR_COMPONENT_MESSAGE_TYPES_END(C_RenderModelBase, C_RenderComponent)

bool C_RenderModelBase::OnMessage(const Messaging::C_MessageBase *pMessage )	
{
	switch(pMessage->Type())
	{
		case E_RenderMsg_SetMatrix:
		{
			C_RenderComponent::OnMessage(pMessage);

			if(m_pEnvSampler && m_pModelInstance)
			{
				m_pEnvSampler->SetPosition( m_pModelInstance->GetWorldSphere().Centre());//m_WorldMatrix.AW() );
				m_pEnvSampler->Update();

				m_pEnvSampler->ApplyToModelInstance(m_pModelInstance);
			}

		}
		break;
	
		case E_RenderMsg_DisablePortalCulling:
			//TODO: Implement when Portal system has been implemented
			//DisablePortalCulling();
			break;

		case E_RenderMsg_SetSubtechnique:
			{
				U32 subtechniqueHandle = Messaging::MessageCast<C_RenderSetSubtechniqueMsg>(pMessage)->GetValue();
				m_pModelInstance->SetSubtechniqueHandle(subtechniqueHandle);
			}			
			break;

		case E_RenderMsg_UpdateAnimatedShaderParams:
			{
				const I_AnimatedShaderParams* pShaderParams = GetActor()->QueryInterface<I_AnimatedShaderParams>();
				if (pShaderParams)
				{
					pShaderParams->SetMaterialParameters(m_pModelInstance);
				}
			}
			break;

		case E_RenderMsg_SetMaterialFloatParam:
			if (m_pModelInstance)
			{
				const C_RenderSetMaterialFloatParamMsg* msg = Messaging::MessageCast<C_RenderSetMaterialFloatParamMsg>(pMessage);
				m_pModelInstance->SetParameter(msg->m_Param, &msg->m_Storage);
			}
			break;

		case E_RenderMsg_EnableVisibilityFlags:
			if (m_pModelInstance)
				m_pModelInstance->EnableVisibilityFlags(Messaging::MessageCast<C_RenderEnableVisibilityFlagsMsg>(pMessage)->GetValue());
			break;

		case E_RenderMsg_DisableVisibilityFlags:
			if (m_pModelInstance)
				m_pModelInstance->DisableVisibilityFlags(Messaging::MessageCast<C_RenderDisableVisibilityFlagsMsg>(pMessage)->GetValue());
			break;

#ifndef CFG_GOLD
		case E_RenderMsg_RequestResourceUsage:
			if (m_pModelInstance != NULL)
			{
				const I_ModelResource* pModelResource = GetActor()->QueryInterface<I_ModelResource>();

				if (pModelResource != NULL)
				{
					const C_RenderNotifyResourceUsage msg(m_pModelInstance->GetWorldSphere(), pModelResource->HasModelResourceUsageDetails(), GetActorHandle(), GetInterfaceID());
					GetActorList()->BroadcastMessage(&msg);
				}
			}
			break;

		case E_RenderMsg_RequestResourceTotal:
			if (m_pModelInstance != NULL)
			{
				const I_ModelResource* pModelResource = GetActor()->QueryInterface<I_ModelResource>();

				if (pModelResource != NULL)
				{
					const WorldRender::C_RenderModelData* pRenderModelData = pModelResource->GetRenderModelData();

					if (pRenderModelData != NULL)
					{
						C_ModelResourceUsage resourceUsage;
						AccumulateModelResourceUsage(resourceUsage, *pRenderModelData);
						const C_RenderNotifyResourceTotalMsg msg(m_pModelInstance->GetWorldSphere().Centre(), resourceUsage.CalcTotalGraphicsBytes(), 0);
						GetActorList()->BroadcastMessage(&msg);
					}
				}
			}
			break;

		case E_RenderMsg_DrawResourceUsage:
			{
				const I_ModelResource* pModelResource = GetActor()->QueryInterface<I_ModelResource>();

				if (pModelResource != NULL)
				{
					const WorldRender::C_RenderModelData* pRenderModelData = pModelResource->GetRenderModelData();

					if (pRenderModelData != NULL)
					{
						C_ModelResourceUsage resourceUsage;
						AccumulateModelResourceUsage(resourceUsage, *pRenderModelData);

						const C_RenderDrawResourceUsageMsg* pMsg = Messaging::MessageCast<C_RenderDrawResourceUsageMsg>(pMessage);

						if (pMsg->GetDebugRender() != NULL)
						{
							resourceUsage.DebugRender(*pMsg->GetDebugRender());
						}
					}
				}

			}
			break;

		case E_RenderMsg_DumpResourceUsage:
			{
				const I_ModelResource* pModelResource = GetActor()->QueryInterface<I_ModelResource>();

				if (pModelResource != NULL)
				{
					const WorldRender::C_RenderModelData* pRenderModelData = pModelResource->GetRenderModelData();

					if (pRenderModelData != NULL)
					{
						C_ModelResourceUsage resourceUsage;
						AccumulateModelResourceUsage(resourceUsage, *pRenderModelData);

						const C_RenderDumpResourceUsageMsg* pMsg = Messaging::MessageCast<C_RenderDumpResourceUsageMsg>(pMessage);

						if (pMsg->GetDebugLogger() != NULL)
						{
							resourceUsage.DebugDump(*pMsg->GetDebugLogger());
						}

						if (pMsg->GetFile() != NULL)
						{
							resourceUsage.DebugDump(*pMsg->GetFile());
						}
					}
				}
			}
			break;
#endif // CFG_GOLD

		default:
			return C_RenderComponent::OnMessage(pMessage);
	}

	return true;
}


void C_RenderModelBase::Show(void)
{
	if (m_pModelInstance != NULL)
	{
		SceneGraph::C_SceneGraph *pSceneGraph = GetActor()->GetContext<GameContext::I_RenderContext>()->GetSceneGraph();
		if(pSceneGraph)
		{
			SceneGraph::Insert(pSceneGraph, m_pModelInstance);
			// Set Initial Render Matrix

			C_SetRenderMatrixMsg setRenderMatrixMsg(m_WorldMatrix);
			SendMessageToComponent(E_IF_Render, &setRenderMatrixMsg);
			//UpdateElementMatrix();

			//SetWorldSphere( m_LocalBoundingSphere * GetRenderMatrix());
			m_pModelInstance->SetNearFarDistance(0.0f, GetCreationData()->m_DrawDistance);
		}

	}
}

void C_RenderModelBase::Hide(void)
{				
	if (m_pModelInstance != NULL)
	{
		SceneGraph::C_SceneGraph *pSceneGraph = GetActor()->GetContext<GameContext::I_RenderContext>()->GetSceneGraph();
		if(pSceneGraph)
			SceneGraph::Remove(pSceneGraph, m_pModelInstance);
	}
}



void C_RenderModelBase::Serialise(Sys::C_ContextualStream& stream) const
{
	//stream << m_ModelHandle;			
	stream << m_HasShadow;

}

void C_RenderModelBase::Deserialise(Sys::C_ContextualStream& stream)
{
	//stream >> m_ModelHandle;			
	stream >> m_HasShadow;
}

void C_RenderModelBase::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_RenderComponent::SerialiseForSave(stream);
	Serialise(stream);
}

void C_RenderModelBase::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_RenderComponent::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_RenderModelBase::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_RenderComponent::SerialiseForSync(stream);
	Serialise(stream);
}

void C_RenderModelBase::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_RenderComponent::DeserialiseForSync(stream);
	Deserialise(stream);
}

void C_RenderModelBase::CustomiseModel(WorldRender::C_ModelInstance* pModelInstance, const GameResource::C_Model* pModelData, const WorldRender::C_Model* pRenderData) const
{
	const I_ModelCustomiser* pModelCustomiser = GetActor()->QueryInterface<I_ModelCustomiser>();

	if (pModelCustomiser != NULL)
	{
		pModelCustomiser->CustomiseModel(pModelInstance, pModelData, pRenderData);
	}
}

void C_RenderModelBase::SetWorldSphere(const Maths::C_Sphere& worldSphere)
{
	if (m_pModelInstance != NULL)
	{
		m_pModelInstance->SetWorldSphere(worldSphere);

		// if we ignore portals then we must remove the spatial probe(s) that were added when we set the world bounds
		if (GetCreationData()->m_IgnorePortals)
		{
			SceneGraph::I_ModelInstanceCullNode* pCullNode = m_pModelInstance->GetCullNode();

			if (pCullNode->GetType() == SceneGraph::I_ModelInstanceCullNode::MODELINSTANCECULLNODE_TYPE_PORTAL)
			{
				SceneGraph::C_PortalCullNode* pPortalCullNode = static_cast<SceneGraph::C_PortalCullNode*>(pCullNode);
				pPortalCullNode->ClearSpatialProbes();
			}
		}
	}
}


// Single Element Model Renderer
// -----------------------------

C_ModelSingleElementRender::C_ModelSingleElementRender()
{

}

void C_ModelSingleElementRender::KillImpl()
{
	C_RenderModelBase::KillImpl();
	return;
}


// Component creator
bool C_ModelSingleElementRender::CreateImpl()
{
	const I_ModelResource *pModelResource = (const I_ModelResource *)GetActor()->QueryInterface(Components::E_IF_ModelResource);

	if(C_RenderModelBase::CreateImpl() && pModelResource)
	{
		S32 modelIndex = GetCreationData()->m_ModelIndex;
		S32 modelElementIndex = GetCreationData()->m_ModelElementIndex == -1 ? 0 : GetCreationData()->m_ModelElementIndex;
		SceneGraph::C_SceneGraph *pSceneGraph = GetActor()->GetContext<GameContext::I_RenderContext>()->GetSceneGraph();
		BIZ_ASSERT(pSceneGraph);

		if((pModelResource->GetModelData() == NULL) || (pModelResource->GetRenderModelData() == NULL))
			return false;

		// setup initial values
		m_RenderOffset = Maths::Load<Maths::HVec3>(&GetCreationData()->m_RenderOffset);

		// Initialise model instance
		const GameResource::C_Model* pModel = pModelResource->GetModelData()->GetModel(modelIndex);

		if(pModel == NULL)
		{
			LOGERROR(Actor::GetDebugLogAsset(), DBSTR"%s: Model Index %d NOT found", GetActor()->GetActorName(), modelIndex);
			return false;
		}
		const GameResource::C_ModelElement* pModelElement = pModel->GetElement(modelElementIndex);
		//m_LocalBoundingSphere = pModel->GetLocalBounds().BoundingSphere(); 
		m_LocalBoundingSphere = pModel->GetBoundingSphere();

		if(pModelResource->GetRenderModelData() == NULL || 
			pModelResource->GetRenderModelData()->GetNumModels() <= modelIndex)
			return false;

		MEMORY_TRACKER_PUSH("worldrender.modelinstance");
		m_pModelInstance = new WorldRender::C_ModelInstance(pModelResource->GetRenderModelData(), modelIndex, pSceneGraph);
		MEMORY_TRACKER_POP("worldrender.modelinstance");
		
		// Setup model instance to render just one element
		for(S32 elementNo = 0; elementNo < m_pModelInstance->GetNumElements(); elementNo++)
			m_pModelInstance->SetElementVisibility(elementNo, false);
		

		// there's some bad platform data with no visible elements??
		if (m_pModelInstance->GetNumElements() > 0 && m_pModelInstance->GetNumElements() >= modelElementIndex)
			m_pModelInstance->SetElementVisibility(modelElementIndex, true);
		else
			LOGWARN(Actor::GetDebugLogAsset(), DBSTR"%s: actor has no visible elements in render component", GetActor()->GetActorName());

		// this can cause scenegraph insertion to fail
		if(m_pModelInstance->GetRenderListNode() == NULL)
			return false;

		// customise model
		CustomiseModel(m_pModelInstance, pModel, &pModelResource->GetRenderModelData()->GetModel(modelIndex));

		// Insert model instance into the scene graph
		if(m_bHidden == false)
		{
			SceneGraph::Insert(pSceneGraph, m_pModelInstance);

			// Set Initial Render Matrix
			UpdateElementMatrix();

			//SetWorldSphere( m_LocalBoundingSphere * GetRenderMatrix());
			m_pModelInstance->SetNearFarDistance(0.0f, Maths::Min(GetCreationData()->m_DrawDistance, 10000.0f));
		}

		return true;
	}
	else
		return false;
}


void C_ModelSingleElementRender::UpdateElementMatrix(void)
{
	Maths::HMat43 renderMat(GetRenderMatrix());
	renderMat.AW(m_RenderOffset * renderMat);
	if(m_pModelInstance)
	{
		m_pModelInstance->SetElementMatrix(GetCreationData()->m_ModelElementIndex, renderMat);
		if(!m_bHidden)
		{
			SetWorldSphere( m_LocalBoundingSphere * GetRenderMatrix());
		}
	}
}

// message handler
bool C_ModelSingleElementRender::OnMessage(const Messaging::C_MessageBase *pMessage )	
{
	switch(pMessage->Type())
	{
	case E_RenderMsg_SetMatrix:
		{
			C_RenderModelBase::OnMessage(pMessage);
			UpdateElementMatrix();
			break;
		}
	case E_RenderMsg_SetRenderOffset:
		{
			const C_SetRenderOffsetMsg *pSetOffsetMsg = Messaging::MessageCast<C_SetRenderOffsetMsg>(pMessage);
			m_RenderOffset = pSetOffsetMsg->GetOffset();
			UpdateElementMatrix();
			break;
		}
	
	default:
		return C_RenderModelBase::OnMessage(pMessage);
		break;
	}
	return true;
}

void C_ModelSingleElementRender::Serialise(Sys::C_ContextualStream& stream) const
{
	//stream << m_ModelElement;
	stream << m_RenderOffset;

}

void C_ModelSingleElementRender::Deserialise(Sys::C_ContextualStream& stream)
{
	//stream >> m_ModelElement;
	stream >> m_RenderOffset;
}

void C_ModelSingleElementRender::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_RenderModelBase::SerialiseForSave(stream);
	Serialise(stream);
}

void C_ModelSingleElementRender::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_RenderModelBase::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_ModelSingleElementRender::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_RenderModelBase::SerialiseForSync(stream);
	Serialise(stream);
}

void C_ModelSingleElementRender::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_RenderModelBase::DeserialiseForSync(stream);
	Deserialise(stream);
}

// Debug Render
#ifndef CFG_GOLD
void C_ModelSingleElementRender::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if(gSingleModelRenderDebugRender == false)
		return;


	C_RenderModelBase::DebugRender(debugRenderPtr);

	Util::C_String	debugString;

	if(m_pModelInstance == NULL)
		debugString  += Util::C_String("NULL Model Instance\n");

	// attributes
	debugString  += Util::C_String("Attributes:");

	if(m_bHidden)
		debugString += Util::C_String(" (Hidden)");
	if(m_HasShadow)
		debugString += Util::C_String(" (Shadow)");
	if(m_pEnvSampler)
		debugString += Util::C_String(" (EnvSampler)");


	debugString  += Util::C_String("\n");

	debugRenderPtr->Draw3dText(GetRenderMatrix().AW(),1.0f,debugString);

	// render bounding sphere
	debugRenderPtr->Draw3dSphere(m_LocalBoundingSphere, GetRenderMatrix(), Maths::RGBA(255,255,255));
}

void C_ModelSingleElementRender::AccumulateModelResourceUsage(C_ModelResourceUsage& modelResourceUsage, const WorldRender::C_RenderModelData& renderModelData) const
{
	modelResourceUsage.AddModelResource(&renderModelData, GetCreationData()->m_ModelIndex);
}
#endif

// Simple Multi Element Render Model
// ---------------------------------
// Just renders elements in a model based on messages sent
C_ModelSimpleMultiElementRender::C_ModelSimpleMultiElementRender() 
{
	// intentionally blank
}

C_ModelSimpleMultiElementRender::~C_ModelSimpleMultiElementRender()
{
}

void C_ModelSimpleMultiElementRender::KillImpl()
{
	C_RenderModelBase::KillImpl();
	return;
}

// Component creator
bool C_ModelSimpleMultiElementRender::CreateImpl()
{
	const I_ModelResource *pModelResource = (const I_ModelResource *)GetActor()->QueryInterface(Components::E_IF_ModelResource);

	if(C_RenderModelBase::CreateImpl() && pModelResource)
	{
		S32 modelIndex = GetCreationData()->m_ModelIndex;
		//S32 modelElementIndex = GetCreationData()->m_ModelElementIndex;
		SceneGraph::C_SceneGraph *pSceneGraph = GetActor()->GetContext<GameContext::I_RenderContext>()->GetSceneGraph();
		BIZ_ASSERT(pSceneGraph);

		if(pModelResource->GetModelData())
		{
			// Initialise model instance
			const GameResource::C_Model* pModel = pModelResource->GetModelData()->GetModel(modelIndex);
			//m_LocalBoundingSphere = pModel->GetLocalBounds().BoundingSphere();
			m_LocalBoundingSphere = pModel->GetBoundingSphere();

			MEMORY_TRACKER_PUSH("worldrender.modelinstance");
			m_pModelInstance = new WorldRender::C_ModelInstance(pModelResource->GetRenderModelData(), modelIndex, pSceneGraph);
			MEMORY_TRACKER_POP("worldrender.modelinstance");
			
			// setup all elements
			m_ElementMatrices.Reserve(pModel->GetElementCount());
			m_ElementMatrices.Clear();

			for(S32 elementNo = 0; elementNo < m_pModelInstance->GetNumElements(); elementNo++)
			{
				m_pModelInstance->SetElementVisibility(elementNo, false);
				m_ElementMatrices.PushBack(pModel->GetElement(elementNo)->GetMatrix());
			}

			if(m_pModelInstance->GetRenderListNode() == NULL)
				return false;

			// customise model
			CustomiseModel(m_pModelInstance, pModel, &pModelResource->GetRenderModelData()->GetModel(modelIndex));

			// Insert model instance into the scene graph
			if(m_bHidden == false)
				SceneGraph::Insert(pSceneGraph, m_pModelInstance);

			SetWorldSphere( m_LocalBoundingSphere * GetRenderMatrix());
			m_pModelInstance->SetNearFarDistance(0.0f, GetCreationData()->m_DrawDistance);

			return true;
		}
	}

	return false;
}

void C_ModelSimpleMultiElementRender::UpdateModelInstance()
{
	// Setup individual render matrices:
	Maths::HMat43 parentMatrix = GetRenderMatrix();

	if (m_pModelInstance)
	{
		// Iterate through and setup element matrices
		for(S32 elementNo = 0; elementNo < m_pModelInstance->GetNumElements(); elementNo++)
		{
			if(m_pModelInstance->GetElementVisibility(elementNo))	// set element matrix if visible
				m_pModelInstance->SetElementMatrix(elementNo,m_ElementMatrices[elementNo] * m_WorldMatrix);
		}

		if(!m_bHidden)
			SetWorldSphere( m_LocalBoundingSphere * GetRenderMatrix());
	}
}

// message handler
bool C_ModelSimpleMultiElementRender::OnMessage(const Messaging::C_MessageBase *pMessage )	
{
	switch(pMessage->Type())
	{
	case E_RenderMsg_SetMatrix:
		{
			C_RenderModelBase::OnMessage(pMessage);
			UpdateModelInstance();			
			break;
		}

	case E_RenderMsg_ShowElement:
		if (m_pModelInstance)
		{
			m_pModelInstance->SetElementVisibility(Messaging::MessageCast<C_RenderShowElementMsg>(pMessage)->GetValue(), true);
		}
		break;

	case E_RenderMsg_HideElement:
		if (m_pModelInstance)
		{
			m_pModelInstance->SetElementVisibility(Messaging::MessageCast<C_RenderHideElementMsg>(pMessage)->GetValue(), false);
		}
		break;

	case E_RenderMsg_SetElementMatrix:
		if (m_pModelInstance)
		{
			const C_RenderSetElementMatrixMsg* msg = Messaging::MessageCast<C_RenderSetElementMatrixMsg>(pMessage);
			m_pModelInstance->SetElementMatrix(msg->GetIndex(), msg->GetMatrix());
		}
		break;

	default:
		return C_RenderModelBase::OnMessage(pMessage);
	}
	return true;
}

#ifndef CFG_GOLD
void C_ModelSimpleMultiElementRender::AccumulateModelResourceUsage(C_ModelResourceUsage& modelResourceUsage, const WorldRender::C_RenderModelData& renderModelData) const
{
	modelResourceUsage.AddModelResource(&renderModelData, GetCreationData()->m_ModelIndex);
}
#endif

//////////////////////////////////////////////////////////////////////////
// Multi Element Render Model
// --------------------------

C_ElementManagerObjectRender::C_ElementManagerObjectRender() 
{
	// intentionally blank
}

C_ElementManagerObjectRender::~C_ElementManagerObjectRender()
{
}

void C_ElementManagerObjectRender::KillImpl()
{
	C_RenderModelBase::KillImpl();
	return;
}

// Component creator
bool C_ElementManagerObjectRender::CreateImpl()
{
	const I_ModelResource *pModelResource = (const I_ModelResource *)GetActor()->QueryInterface(Components::E_IF_ModelResource);

	if(C_RenderModelBase::CreateImpl() && pModelResource)
	{
		S32 modelIndex = GetCreationData()->m_ModelIndex;
		//S32 modelElementIndex = GetCreationData()->m_ModelElementIndex;
		SceneGraph::C_SceneGraph *pSceneGraph = GetActor()->GetContext<GameContext::I_RenderContext>()->GetSceneGraph();
		BIZ_ASSERT(pSceneGraph);

		if(pModelResource->GetModelData())
		{
			// Initialise model instance
			const GameResource::C_Model* pModel = pModelResource->GetModelData()->GetModel(modelIndex);
			//m_LocalBoundingSphere = pModel->GetLocalBounds().BoundingSphere();
			m_LocalBoundingSphere = pModel->GetBoundingSphere();

			MEMORY_TRACKER_PUSH("worldrender.modelinstance");
			m_pModelInstance = new WorldRender::C_ModelInstance(pModelResource->GetRenderModelData(), modelIndex, pSceneGraph);
			MEMORY_TRACKER_POP("worldrender.modelinstance");

			// set all elements to invisible to begin
			//m_pModelInstance->SetElementVisibility(-1, false);
			for(S32 elementNo = 0; elementNo < m_pModelInstance->GetNumElements(); elementNo++)
				m_pModelInstance->SetElementVisibility(elementNo, false);

			// customise model
			CustomiseModel(m_pModelInstance, pModel, &pModelResource->GetRenderModelData()->GetModel(modelIndex));

			// Insert model instance into the scene graph
			if(m_bHidden == false)
			{
				SceneGraph::Insert(pSceneGraph, m_pModelInstance);

				// note: these function must only be called if the object is in the scenegraph
				SetWorldSphere( m_LocalBoundingSphere * GetRenderMatrix());
				m_pModelInstance->SetNearFarDistance(0.0f, GetCreationData()->m_DrawDistance);
			}

			return true;
		}
	}
	
	return false;
}


// Recursive function to build matrix list
void C_ElementManagerObjectRender::BuildElementChildMatrices(const C_ElementState &element,Maths::HMat43 &mat)
{
	for(S32 childNo = 0; childNo < element.GetChildCount(); childNo++)
	{
		const C_ElementState &child = element.GetChild(childNo);

		Maths::HMat43 newMat = mat * child.m_Matrix;	// calculate new

		m_pModelInstance->SetElementVisibility(element.m_ModelElement, true);
		m_pModelInstance->SetElementMatrix(element.m_ModelElement, newMat);

		//outputList.PushBack(C_ModelElementRender(newMat,child.m_ModelElement));	// add new matrix

		if(child.GetChildCount() > 0)	// recurse though children
			BuildElementChildMatrices(child,newMat);
	}
}


// message handler
bool C_ElementManagerObjectRender::OnMessage(const Messaging::C_MessageBase *pMessage )	
{
	switch(pMessage->Type())
	{
	case E_RenderMsg_SetMatrix:
		{
			C_RenderModelBase::OnMessage(pMessage);

			const Components::I_DamageableElementManager* pDamageObject = (const Components::I_DamageableElementManager*)QueryInterface(Components::E_IF_DamageableElementManager);
			if (!pDamageObject)	// no damage object
			{
				return true;
			}

			// Setup individual render matrices:
			Maths::HMat43 parentMatrix = GetRenderMatrix();
			bool pivot = GetCreationData()->m_Pivoted;

			if (m_pModelInstance)
			{
				for(S32 elementNo = 0; elementNo < m_pModelInstance->GetNumElements(); elementNo++)
				{
					m_pModelInstance->SetElementVisibility(elementNo, false);
				}

				if(!m_bHidden)
					SetWorldSphere( m_LocalBoundingSphere * GetRenderMatrix());
			}

			for(S32 elementNo = 0; elementNo < pDamageObject->GetElementCount(); elementNo++)
			{
				const C_ElementState &element = pDamageObject->GetElement(elementNo);
				Maths::HMat43 newMat = element.m_Matrix * parentMatrix;//pivot ? (element.m_Matrix * parentMatrix) : parentMatrix;
				
				if (m_pModelInstance != NULL)
				{
					m_pModelInstance->SetElementVisibility(element.m_ModelElement, true);
					m_pModelInstance->SetElementMatrix(element.m_ModelElement, newMat);
				}

				if(element.GetChildCount()>0)
					BuildElementChildMatrices(element,newMat);
			}

			break;
		}


	default:
		return C_RenderModelBase::OnMessage(pMessage);
	}
	return true;
}

#ifndef CFG_GOLD
void C_ElementManagerObjectRender::AccumulateModelResourceUsage(C_ModelResourceUsage& modelResourceUsage, const WorldRender::C_RenderModelData& renderModelData) const
{
	modelResourceUsage.AddModelResource(&renderModelData, GetCreationData()->m_ModelIndex);
}
#endif


// bool C_DamageObjectRender::OnMessage(const Messaging::C_MessageBase *pMessage )	
// {
// 	switch(pMessage->Type())
// 	{
// 		case E_RenderMsg_SetMatrix:
// 		{
// 			C_RenderModelBase::OnMessage(pMessage);
// 
// 			const Components::I_DamageableElementManager* pDamageObject = (const Components::I_DamageableElementManager*)QueryInterface(Components::E_IF_DamageableElementManager);
// 			if (!pDamageObject)	// no damage object
// 			{
// 				return true;
// 			}
// 
// 			// Setup individual render matrices:
// 			Maths::HMat43 parentMatrix = GetRenderMatrix();
// 			bool pivot = GetCreationData()->m_Pivoted;
// 
// 			if (m_pModelInstance)
// 			{
// 				for(S32 elementNo = 0; elementNo < m_pModelInstance->GetNumElements(); elementNo++)
// 				{
// 					m_pModelInstance->SetElementVisibility(elementNo, false);
// 				}
// 			}
// 
// 			for(S32 elementNo = 0; elementNo < pDamageObject->GetElementCount(); elementNo++)
// 			{
// 				const C_ElementState &element = pDamageObject->GetElement(elementNo);
// 				Maths::HMat43 newMat = element.m_Matrix * parentMatrix;//pivot ? (element.m_Matrix * parentMatrix) : parentMatrix;
// 				
// 				if (m_pModelInstance != NULL)
// 				{
// 					m_pModelInstance->SetElementVisibility(element.m_ModelElement, true);
// 					m_pModelInstance->SetElementMatrix(element.m_ModelElement, newMat);
// 				}
// 			}
// 
// 			break;
// 		}
// 
// 		default:
// 			return C_ModelMultiElementRender::OnMessage(pMessage);
// 	}
// 	return true;
// }


#if 0
// Text Render Component
// ---------------------
bool C_TextRender::CreateImpl()
{
	return C_RenderComponent::CreateImpl();
}

void C_TextRender::Render(const U32 renderPass, const U32 renderStates)	// render function
{
	Display::FontRender()->Draw3D(GetCreationData()->m_Text.Chars(), GetRenderMatrix().AW(), GetCreationData()->m_TextSize, GetCreationData()->m_TextColour, Display::E_FontRenderAlign_Ct);
}
#endif


// Portal Blocker
// --------------

C_PortalBlocker::C_PortalBlocker()
	: I_PortalBlocker()
	, m_pPortalBlockerHandle(NULL)
	, m_bOpen(false)
{
}

C_PortalBlocker::~C_PortalBlocker()
{
	BIZ_ASSERT(m_pPortalBlockerHandle == NULL);
}

bool C_PortalBlocker::InitialiseImpl()
{
	if (!I_PortalBlocker::InitialiseImpl())
		return false;

	const C_PortalBlockerCreate* pBlockerCreate = GetCreationData();

	m_bOpen = pBlockerCreate->m_bStartClosed ? false : true;

	return true;
}

bool C_PortalBlocker::CreateImpl()
{
	if (!I_PortalBlocker::CreateImpl())
		return false;

	const C_PortalBlockerCreate* pBlockerCreate = GetCreationData();

	const Maths::HVec3 halfExtents = Load(pBlockerCreate->m_BoundingBox.size) * Maths::Constants<Maths::HF32>::Half();
	Maths::HMat43 boxBasis = Load(pBlockerCreate->m_BoundingBox.matrix) * GetActor()->GetMatrix();
	boxBasis.AW(boxBasis.AW() + (halfExtents * boxBasis.Rotation()));

	m_pPortalBlockerHandle = new Portal::BlockerHandle(Portal::C_System::GetDeferredBlocker().CreateBlocker(boxBasis, halfExtents));

	if (m_bOpen)
		m_pPortalBlockerHandle->Off();
	else
		m_pPortalBlockerHandle->On();

	return true;
}

void C_PortalBlocker::KillImpl()
{
	if (m_pPortalBlockerHandle != NULL)
	{
		Portal::C_System::GetDeferredBlocker().RemoveBlocker(*m_pPortalBlockerHandle);
		SAFE_DELETE_PTR(m_pPortalBlockerHandle);
	}

	I_PortalBlocker::KillImpl();
}

bool C_PortalBlocker::OnMessage(const Messaging::C_MessageBase *pMessage )
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_GameActorMsg_Open:
		m_bOpen = true;
		if (m_pPortalBlockerHandle != NULL)
			m_pPortalBlockerHandle->Off();
		return true;

	case ActorMsg::E_GameActorMsg_Close:
		m_bOpen = false;
		if (m_pPortalBlockerHandle != NULL)
			m_pPortalBlockerHandle->On();
		return true;

	default:
		return I_PortalBlocker::OnMessage(pMessage);
	}
}

void C_PortalBlocker::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_bOpen;
}

void C_PortalBlocker::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_bOpen;
}

void C_PortalBlocker::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	I_PortalBlocker::SerialiseForSave(stream);
	Serialise(stream);
}

void C_PortalBlocker::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	I_PortalBlocker::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_PortalBlocker::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	I_PortalBlocker::SerialiseForSync(stream);
	Serialise(stream);
}

void C_PortalBlocker::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	I_PortalBlocker::DeserialiseForSync(stream);
	Deserialise(stream);

	if (m_bOpen)
		m_pPortalBlockerHandle->Off();
	else
		m_pPortalBlockerHandle->On();
}

} //namespace Components

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", on)
#endif
