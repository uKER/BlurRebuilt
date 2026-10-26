#include "stdafx.h"

#include "RenderComponents.h"

#include <actor_system/Actor.h>
#include <core/BizAssert.h>

#include <world_render/Scene.h>
#include <world_render/RenderModelData.h>
#include <world_render/ModelInstance.h>
#include <world_render/MaterialParameter.h>
#include <resource_utils/GameModel.h>
#include <scenegraph/SceneGraph.h>

#include <game_resource/LevelSceneManager.h>
#include <actor_system/Actor.h>

#include <game_context/RenderContext.h>

//#include <Render/Render/WorldRenderTypes.h>
#include <shared_actors/SharedActorMessages.h>
//#include "GameWorld/GameWorld.h"
#include "Game/ActorComponents/Messages/RenderMessages.h"
#include "shared_actors/Components/Interfaces/I_ElementManager.h"
//#include <common/memory/Memory.h>

#include "GameApp.h"
#include "Game/GameWorld/GameWorldLayer.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize("", off)
#endif

namespace Actors
{
	extern Debug::Logger& g_Log;
}

namespace Components
{
#if 0
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

bool C_RenderComponent::OnMessage(const Messaging::C_MessageBase *pMessage )	
{
	switch(pMessage->Type())
	{
	case E_RenderMsg_SetMatrix:
		{
			const C_SetRenderMatrixMsg *pSetMatrixMsg = Messaging::MessageCast<C_SetRenderMatrixMsg>(pMessage);
			SetRenderMatrix(pSetMatrixMsg->GetMatrix());
			break;
		}
	case E_RenderMsg_Show:
		//TODO: SetRender(true);
		break;
	case E_RenderMsg_Hide:
		//TODO: SetRender(false);
		break;
	default:
		return I_RenderComponent::OnMessage(pMessage);
		break;
	}
	return true;
}

void C_RenderComponent::Serialise(Sys::C_ContextualStream& stream) const
{
	//stream << FlagRender();
}

void C_RenderComponent::Deserialise(Sys::C_ContextualStream& stream)
{
	//SetRender(stream.ReadAs<bool>());
}

void C_RenderComponent::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	I_RenderComponent::SerialiseForSave(stream);
	Serialise(stream);
}

void C_RenderComponent::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	I_RenderComponent::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_RenderComponent::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	I_RenderComponent::SerialiseForSync(stream);
	Serialise(stream);
}

void C_RenderComponent::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	I_RenderComponent::DeserialiseForSync(stream);
	Deserialise(stream);
}


// Render Model Base Class
// -----------------------

C_RenderModelBase::C_RenderModelBase():m_pModelInstance(NULL), m_pLightSampler(NULL)
{
}

C_RenderModelBase::~C_RenderModelBase()
{
	SAFE_DELETE_PTR( m_pLightSampler );
}

bool C_RenderModelBase::CreateImpl()
{
 	if(C_RenderComponent::CreateImpl())
 	{
		m_pLightSampler = NULL;//LightSampler::NewInitialisedLightSampler(GetActor()->GetContext<GameContext::I_RenderContext>()->GetLevelSceneManager());

 		m_HasShadow = GetCreationData()->m_HasShadow;

		//m_pModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, E_RenderPass_CustomSolid);

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
	
		delete m_pModelInstance;
		m_pModelInstance = NULL;
	}
}

// message handler
bool C_RenderModelBase::OnMessage(const Messaging::C_MessageBase *pMessage )	
{
	switch(pMessage->Type())
	{
		case E_RenderMsg_SetMatrix:
		{
			C_RenderComponent::OnMessage(pMessage);

			if(m_pLightSampler && m_pModelInstance)
			{
				m_pLightSampler->SetPosition( m_WorldMatrix.AW() );
				m_pLightSampler->Update();

				WorldRender::MaterialParameter handle = m_pModelInstance->GetRenderModel()->GetParameter("g_SHNode");
				if (handle.index != -1)
					m_pModelInstance->SetParameter(handle, &m_pLightSampler->GetLightSampleData().m_SampleData[0]);	
			}

		}
		break;
	
		
		default:
			return C_RenderComponent::OnMessage(pMessage);
	}

	return true;
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
		S32 modelElementIndex = GetCreationData()->m_ModelElementIndex;
		SceneGraph::C_SceneGraph *pSceneGraph = GetActor()->GetContext<GameContext::I_RenderContext>()->GetSceneGraph();
		BIZ_ASSERT(pSceneGraph);

		// setup initial values
		m_RenderOffset = Maths::Load<Maths::HVec3>(&GetCreationData()->m_RenderOffset);

		// Initialise model instance
		const GameResource::C_Model* pModel = pModelResource->GetModelData()->GetModel(modelIndex);
		const GameResource::C_ModelElement* pModelElement = pModel->GetElement((modelElementIndex == -1) ? 0:modelElementIndex);
		m_LocalBoundingSphere = pModel->GetLocalBounds().BoundingSphere();

		m_pModelInstance = new WorldRender::C_ModelInstance(pModelResource->GetRenderModelData(), modelIndex, pSceneGraph);
		m_pModelInstance->SetWorldSphere( m_LocalBoundingSphere * GetRenderMatrix());
		
		// Setup model instance to render just one element
		//m_pModelInstance->SetElementVisibility(-1, false);
		m_pModelInstance->SetElementVisibility(modelElementIndex, true);

		// Set Initial Render Matrix
		UpdateElementMatrix();

		// Insert model instance into the scene graph
		SceneGraph::Insert(pSceneGraph, m_pModelInstance);

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
		m_pModelInstance->SetElementMatrix(GetCreationData()->m_ModelElementIndex, renderMat);
}


// message handler
bool C_ModelSingleElementRender::OnMessage(const Messaging::C_MessageBase *pMessage )	
{
	switch(pMessage->Type())
	{
	case E_RenderMsg_SetMatrix:
		{
			//const C_SetRenderMatrixMsg *pSetMatrixMsg = Messaging::MessageCast<C_SetRenderMatrixMsg>(pMessage);
			C_RenderModelBase::OnMessage(pMessage);
			//SetRenderMatrix(pSetMatrixMsg->GetMatrix());
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


// Multi Element Render Model
// --------------------------

C_ModelMultiElementRender::C_ModelMultiElementRender() 
{
	// intentionally blank
}

C_ModelMultiElementRender::~C_ModelMultiElementRender()
{
}

void C_ModelMultiElementRender::KillImpl()
{
	C_RenderModelBase::KillImpl();
	return;
}

// Component creator
bool C_ModelMultiElementRender::CreateImpl()
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
			const GameResource::C_ModelElement* pModelElement = pModel->GetElement(0);
			m_LocalBoundingSphere = pModelElement->GetLocalBounds().BoundingSphere();

			m_pModelInstance = new WorldRender::C_ModelInstance(pModelResource->GetRenderModelData(), modelIndex, pSceneGraph);
			m_pModelInstance->SetWorldSphere( m_LocalBoundingSphere * pModelElement->GetMatrix() * GetRenderMatrix());

			// set all elements to invisible to begin
			//m_pModelInstance->SetElementVisibility(-1, false);

			// Insert model instance into the scene graph
			SceneGraph::Insert(pSceneGraph, m_pModelInstance);

			return true;
		}
	}
	
	return false;
}


// Recursive function to build matrix list
void C_ModelMultiElementRender::BuildElementChildMatrices(const C_ElementState &element,Maths::HMat43 &mat)
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
bool C_ModelMultiElementRender::OnMessage(const Messaging::C_MessageBase *pMessage )	
{
	switch(pMessage->Type())
	{
	case E_RenderMsg_SetMatrix:
		{
			C_RenderModelBase::OnMessage(pMessage);

			I_ElementManager* pElementManager = NULL;//(I_ElementManager*)QueryInterface(Components::E_IF_DamageObject);
			if (!pElementManager)	// no element manager
			{
				//TODO
// 				static int count=0;
// 				if (count++ < 5)
// 					LOGWARN(Actors::g_Log, DBSTR"%s: no element manager component found", GetActor()->GetActorName());
				return true;
			}

			// Setup individual render matrices:
			Maths::HMat43 parentMatrix = GetRenderMatrix();
			bool pivot = GetCreationData()->m_Pivoted;
			//m_pModelInstance->SetElementVisibility(-1, false);

			//m_pElementsToRender->Clear();
			for(S32 elementNo = 0; elementNo < pElementManager->GetElementCount(); elementNo++)
			{
				const C_ElementState &element = pElementManager->GetElement(elementNo);
				Maths::HMat43 newMat = pivot ? (element.m_Matrix * parentMatrix) : parentMatrix;

				if (m_pModelInstance != NULL)
				{
					m_pModelInstance->SetElementVisibility(element.m_ModelElement, true);
					m_pModelInstance->SetElementMatrix(element.m_ModelElement, newMat);
				}
				//m_pElementsToRender->PushBack(C_ModelElementRender(newMat,element.m_ModelElement));	// add new matrix

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
#endif


// render function
#if 0
void C_ModelMultiElementRender::Render(const U32 renderPass, const U32 renderStates)
{
#ifdef CFG_PROFILE
	PERF_EVENT("Components::C_SimpleRenderModel::Render");
#endif

	if (renderPass & Render::Scene::E_RenderPass_Depth && !m_HasShadow)
		return;

	// Render Model
	if (m_ModelHandle)
	{
		Model::C_ModelPtr modelPtr;
		m_ModelHandle->GetResource(modelPtr);
		if (modelPtr)
		{		
			if (!(renderPass & Render::Scene::E_RenderPass_Depth))
			{
				if (m_FluxSampler.IsReady())
				{
					m_FluxSampler.Update( Maths::HVec3(GetRenderMatrix().AW()) );
					m_ModelStyle.SetLightSample( m_FluxSampler.GetSHInfo() );
				}
				else
				{
					m_ModelStyle.ClearLightSample();
				}
			}

			// Render Elements Here
			for (C_ModelElementRender::C_List::ConstIterator it = m_pElementsToRender->Begin(); it != m_pElementsToRender->End(); ++it)
				m_ModelStyle.RenderElementSpace(renderPass, GetRenderFlags(), m_ModelHandle, it->m_Matrix, it->m_ElementIndex, 0);
		}
	}
}
#endif



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


// Portal Blocker
// --------------

C_PortalBlocker::C_PortalBlocker():m_pBlocker(NULL)
{
}

C_PortalBlocker::~C_PortalBlocker()
{
	SAFE_DELETE_PTR(m_pBlocker);
}

bool C_PortalBlocker::CreateImpl(void)
{
	const C_PortalBlockerCreate	*pBlockerCreate = GetCreationData();

	m_pBlocker = C_GameWorld::GetInstance()->GetScene().GetPortalSystem().NewBlocker();
	m_pBlocker->SetWorldMatrix(GetActor()->GetMatrix());

	Maths::C_AxisBox localBoundingBox(Maths::HVec3(0,0,0),Load(pBlockerCreate->m_BoundingBox.m_Size));
//	m_LocalMatrix = Load(pBlockerCreate->m_BoundingBox.m_Matrix);

	m_pBlocker->SetLocalBoundingBox( localBoundingBox );

	m_pBlocker->UpdatePortals();
	m_bOpen = pBlockerCreate->m_bStartClosed ? false : true;
	m_pBlocker->SetOpen(m_bOpen);	// default to open - move to post create init?

	return true;
}

bool C_PortalBlocker::OnMessage(const Messaging::C_MessageBase *pMessage )
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_GameActorMsg_Open:
		m_bOpen = true;
		m_pBlocker->SetOpen(m_bOpen);
		break;
	case ActorMsg::E_GameActorMsg_Close:
		m_bOpen = false;
		m_pBlocker->SetOpen(m_bOpen);
		break;
	default:
		return I_PortalBlocker::OnMessage(pMessage);
	}

	return true;
}

void C_PortalBlocker::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_bOpen;
}

void C_PortalBlocker::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_bOpen;
	m_pBlocker->SetOpen(m_bOpen);
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
}
#endif

} //namespace Components

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize("", on)
#endif
