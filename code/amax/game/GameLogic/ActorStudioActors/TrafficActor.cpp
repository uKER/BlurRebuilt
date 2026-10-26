#include "stdafx.h"
#include <world_render/ModelInstance.h>	// [Blur2PC]

#include <Game/Game.h>

#include <GameApp.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>

#include <Game/Messaging\GameFlowMessages.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/ActorStudioActors/ActorUtils.h>

#include "TrafficActor.h"

//=========================================================================
//		Sorcery definitions
//=========================================================================

//	Actor creation definition (Sorcery)

XT_TYPE(::Actors::TrafficActorCreationData,		"TrafficActor")
	XT_BASE(::Actors::C_PosActorCreate)

	XT_FIELD(m_Spline,							"Spline")
	XT_FIELD(m_TrafficGroup,					"TrafficGroup")
	XT_FIELD(m_Speed,							"Speed")
	XT_FIELD(m_Spacing,							"Spacing")
	XT_FIELD(m_SpacingVariation,				"SpacingVariation")
	XT_FIELD(m_UpdateLighting,					"UpdateLighting")
XT_END_TYPE


//=========================================================================
//					Component Creation Setup
//=========================================================================


void ::Actors::TrafficActorCreationData::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_GameActorCreate::DoSorceryCallback(pCallbackFn);
}




//=========================================================================
//					CutScene Proxy Actor Core Routines
//=========================================================================



namespace Actors
{


//=========================================================================

TrafficActor::TrafficActor()
{
	m_pEnvSampler = NULL;

	m_Random.Seed( 8453UL );
	m_FirstUpdate = false;
	m_Initialised = false;
}

//=========================================================================

TrafficActor::~TrafficActor()
{
	SAFE_DELETE_PTR(m_pEnvSampler);

	SceneGraph::C_SceneGraph* pSceneGraph = GetSceneGraph();

	for (Btl::Vector<TrafficItem>::Iterator it=m_TrafficItems.Begin(); it != m_TrafficItems.End(); ++it)
	{
		if (pSceneGraph && it->m_ModelInstance)
		{
			SceneGraph::Remove(pSceneGraph, it->m_ModelInstance);
		}

		SAFE_DELETE_PTR(it->m_ModelInstance);
	}
}

//=========================================================================

bool TrafficActor::InitialiseImpl()					// Called first
{
	if (!C_GameActor::InitialiseImpl())
		return(false);

	return(true);
}


//=========================================================================

bool TrafficActor::CreateImpl()						// Called second
{
	Actor::C_ActorHandle	handle;


	if (!C_GameActor::CreateImpl())
		return(false);

	const TrafficActorCreationData *pCreate = GetCreationData();

	if (!pCreate->m_Spline)								// Check that the spline exists
		return(false);

	handle = pCreate->m_Spline->GetActorHandle();
	if (!handle)
		return(false);

	m_Spline = ::Actor::ActorCast_Safe< ::Actors::C_SplineActor>(GetActorList()->GetActorPtr(handle));
	m_SplineLength = m_Spline->GetSplineWrapper()->GetCurveEval().GetArcLength();

	if (pCreate->m_Spacing == 0.0f)
		return(false);

	int numTrafficItems = (int)(m_SplineLength / pCreate->m_Spacing);			// Spline is all OK

	if (!pCreate->m_TrafficGroup)						// Check that the group exists
		return(false);
	if (pCreate->m_TrafficGroup->ObjDefList.count == 0)
		return(false);


	m_TrafficItems.Reserve(numTrafficItems, true);				// Create all the vehicles
	
	for(int a=0; a < numTrafficItems; a++)
	{
		const Game::DynamicObjectDef	*objdef = pCreate->m_TrafficGroup->ObjDefList[(rand()%pCreate->m_TrafficGroup->ObjDefList.count)];
		if (!objdef)
		{
			LOGERROR(Game::g_Log, "Dynamic object in group has a null object definition");
			return(false);
		}

		m_TrafficItems.PushBack();
		m_TrafficItems[a].m_SpacingOffset = (m_Random.FRand() - 0.5f) * pCreate->m_SpacingVariation * 2.0f;
		m_TrafficItems[a].m_ModelHandle.Load( objdef->m_ModelType->m_Filename.Text());			// Start the loader
	}

	// array needs to be created before load is called as it is iterated in the resource notify callback
	for (Btl::Vector<TrafficItem>::Iterator it=m_TrafficItems.Begin(); it != m_TrafficItems.End(); ++it)
	{
		((Resource::C_ResourceBase*)it->m_ModelHandle)->AddListener( this );			// Register resource listener
	}

	m_pEnvSampler = new WorldRender::C_EnvironmentSampler;
	m_pEnvSampler->Initialise( GetWorldRenderScene() );

	m_Initialised = true;
	return(true);
}

//=========================================================================

void TrafficActor::ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
{
	switch(result)
	{
		case Resource::E_Notify_DataReady:
		{
			bool initialise(true);

			for (Btl::Vector<TrafficItem>::Iterator it=m_TrafficItems.Begin(); it != m_TrafficItems.End(); ++it)
			{
				if (!it->m_Loaded)
				{
					if (it->m_ModelHandle == pResource)
					{
						it->m_pModelResource = it->m_ModelHandle;		// ??? all done in the casting ???
						it->m_Loaded = true;

						CreateModelInstance(*it);
					}
					else
					{
						initialise = false;
					}
				}
			}

			// This only should happen once all the objects have been initialised
			if (initialise)
			{
				m_offset = 0.0f;
				m_FirstUpdate = true;
				AddSubscriber(GameMessaging::E_GameMsg_Update);
			}
			break;
		}
		
		case Resource::E_Notify_DataError:
		case Resource::E_Notify_NoData:
			LOGERROR(Game::g_Log, DBSTR "Failed to load traffic model");
			break;
	}
}

//=========================================================================

void TrafficActor::CreateModelInstance(TrafficItem& titem)			// Create and load model into render scene
{
	S32 modelIndex = 0;													// Can only deal with one model element for now
	
	if (titem.m_pModelResource->GetModelData()->GetModelCount() > 1)
		modelIndex = 1;

	SceneGraph::C_SceneGraph* pSceneGraph = GetSceneGraph();
	
	Maths::C_Sphere boundingSphere = titem.m_pModelResource->GetModelData()->GetBounds().BoundingSphere();

	BIZ_ASSERT(titem.m_ModelInstance == NULL);
	
	MEMORY_TRACKER_PUSH("worldrender.modelinstance");
	titem.m_ModelInstance = new WorldRender::C_ModelInstance(titem.m_pModelResource->GetRenderModelData(), modelIndex, pSceneGraph);
	MEMORY_TRACKER_POP("worldrender.modelinstance");

	SceneGraph::Insert(pSceneGraph, titem.m_ModelInstance);
	
	titem.m_ModelInstance->SetWorldSphere(boundingSphere);

	if (titem.m_pModelResource->GetModelData()->GetModelCount() == 2)			// Disable any damage versions if present
	{
		titem.m_ModelInstance->SetElementVisibility(0, true);
		titem.m_ModelInstance->SetElementVisibility(1, false);
	}
}


//=========================================================================
//						MESSAGE HANDLERS
//=========================================================================


bool TrafficActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{

	switch(pMessage->Type())
	{
		case GameMessaging::E_GameMsg_Update:
		{
			const GameMessaging::C_UpdateMsg* pMsg = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage);

			m_offset += GetCreationData()->m_Speed * pMsg->GetValue();				// Move everything forward
			if (m_offset > m_SplineLength)
				m_offset -= m_SplineLength;

			int a(0);

			for (Btl::Vector<TrafficItem>::Iterator it=m_TrafficItems.Begin(); it != m_TrafficItems.End(); ++it, a++)
			{
				F32	itemoffset = m_offset + (a * GetCreationData()->m_Spacing) + it->m_SpacingOffset;			// Calc where we are on the spline

				if (itemoffset > m_SplineLength)
					itemoffset -= m_SplineLength;

				Maths::C_QuatTrans	newtransform;
				m_Spline->GetSplineWrapper()->GetForwardFacingSplinePos(newtransform, itemoffset);				// Move this item to it's new spline position
				
				it->m_WorldMatrix.Translation(newtransform.Translation());
				it->m_WorldMatrix.Rotation(newtransform.m_Quaternion);

				it->m_ModelInstance->SetElementMatrix(-1, it->m_WorldMatrix);								// Update render model
				it->m_ModelInstance->SetWorldSpherePosition(it->m_WorldMatrix.Translation());


				if (/*(GetCreationData()->m_UpdateLighting) ||*/ (m_FirstUpdate))		// Always update lighting on first frame
				{
					if (m_pEnvSampler)
					{
						m_pEnvSampler->SetPosition( it->m_WorldMatrix.Translation() );
						m_pEnvSampler->Update();
						m_pEnvSampler->ApplyToModelInstance(it->m_ModelInstance);
					}
				}
			}

			m_FirstUpdate = false;
			break;
		}
	}

	return(true);
}



} // namespace Actors
