#include "stdafx.h"

#include <Game/GameActors/TestActors.h>

#include <actor_system/ActorSys.h>
#include <misc_shared/messaging/GameMessaging.h>

#include <shared_actors/XtActorBase.h>
#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/PhysicsMessages.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>
#include <shared_actors/Utility/PhysicsBodyUserData.h>
#include <shared_actors/Utility/VisibilityDetection.h>

#include <resource/Resource.h>
#include <resource_utils/GameModel.h>
#include <game_resource/ModelData.h>
#include <game_resource/ModelResource.h>
#include <world_render/ModelInstance.h>
#include <world_render/RenderModelData.h>
#include <world_render/MaterialParameter.h>
#include <world_render/EnvironmentSampler.h>
#include <scenegraph/SceneGraph.h>

#include <gravity/sdk/BodyHelper.h>
#include <gravity/importer/Importer.h>

#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>

#include <Game/GameWorld/GameWorld.h>

#include <Game/GameDef/PhysicsConstants.h>
#include <world_render/Scene.h>

XT_TYPE(Game::Actors::C_ModelTestActorCreateXt, "ModelTestActor")
	XT_BASE(::Actors::C_PosActorCreate)
	XT_FIELD(m_GameModel, "Model")
XT_END_TYPE

Debug::Logger &g_TmpLog = Debug::GetLogger("gamesys.actors.visdetect");

Debug::Float g_dbActorMass("Game.FrontEnd.ActorMass", 25.f);

namespace Game
{

namespace Actors
{

Debug::Bool g_DisplayTestModels("ActorDebug.Actors.TestModel.ShowInfo", false);

// Actor Info

Util::C_String C_ModelTestActorCreateProcedural::GetModelFilename() const
{
	return m_ModelFilename;
}

void C_ModelTestActorCreateProcedural::Serialise(Sys::C_ContextualStream& stream) const
{
	C_ModelTestActorCreate::Serialise(stream);
	stream << m_ModelFilename;
}

void C_ModelTestActorCreateProcedural::Deserialise(Sys::C_ContextualStream& stream)
{
	C_ModelTestActorCreate::Deserialise(stream);
	stream >> m_ModelFilename;
}

Util::C_String C_ModelTestActorCreateXt::GetModelFilename() const
{
	if (m_GameModel)
		return Util::C_String(m_GameModel->m_Filename);

	return Util::C_String();
}

C_ModelTestActor::C_ModelTestActor()
  : m_pModelResource(NULL)
  , m_pModelInstance(NULL)
  , m_pBody(NULL)
  , m_pPhysicsBodyUserData(NULL)
  , m_pEnvSampler(NULL)
  , m_pVisDetect(NULL)
{
}

C_ModelTestActor::~C_ModelTestActor()
{
	if( m_pModelResource != NULL )
		DestroyModel();

	::Actors::C_VisibilityDetector::ReleaseVisDetect(m_pVisDetect);
}

bool C_ModelTestActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_ResourceMsg_Loaded:
		if (Messaging::MessageCast<ActorMsg::C_ResourceLoadedMsg>(pMessage)->GetValue() == 0 )
		{
			if (m_pModelResource != NULL)
				DestroyModel();

			m_pModelResource = QueryInterface<Components::I_ResourceLoader>()->GetResource<GameResource::C_ModelResource>( 0 );
			
			if (m_pModelResource != NULL)
				CreateModel();
		}
		return true;

	case ActorMsg::E_ResourceMsg_Unloaded:
		if (Messaging::MessageCast<ActorMsg::C_ResourceUnloadedMsg>(pMessage)->GetValue() == 0 )
		{
			if (m_pModelResource != NULL)
				DestroyModel();

			m_pModelResource = NULL;
		}
		return true;

	case ::Actors::E_VisibilityMsg_OnVisible:
		LOGDEBUG( g_TmpLog, "TestActor has become visible\n");
		return true;

	case ::Actors::E_VisibilityMsg_OnHidden:
		LOGDEBUG( g_TmpLog, "TestActor has become invisible\n");
		return true;

	case GameMessaging::E_GameMsg_Update:
		if(m_pModelInstance != NULL)
		{
			//m_WorldMatrix = Maths::HMat43(m_pBody->Orientation(), m_pBody->Position());
			if (m_pBody)
				m_WorldMatrix = m_pBody->ElementMatrix();//m_pModelElement->GetMatrix() * m_pBody->ElementMatrix();
			m_pModelInstance->SetWorldSphere( m_LocalBoundingSphere * m_WorldMatrix );
			m_pModelInstance->SetElementMatrix(-1, m_WorldMatrix);

			m_pEnvSampler->SetPosition( m_WorldMatrix.AW() );
			m_pEnvSampler->Update();
			m_pEnvSampler->ApplyToModelInstance(m_pModelInstance);
		}
		
		return true;

		case Actor::E_ActorMsg_PhysicsBodyEvent:
			return true;
		break;

	}

	return C_GameActor::OnMessage(pMessage);
}

#ifndef CFG_GOLD
void C_ModelTestActor::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if( g_DisplayTestModels )
	{
		Maths::HMat43 mElement = m_pModelElement->GetMatrix();
		Maths::HMat43 mModelDebug = mElement.InverseRotTrans() * m_WorldMatrix;
		m_pModelResource->GetModelData()->DebugRender(debugRenderPtr, mModelDebug);
	}	
}
#endif

void C_ModelTestActor::CreateModel()
{
	m_pEnvSampler = new WorldRender::C_EnvironmentSampler;
	m_pEnvSampler->Initialise( GetWorldRenderScene() );

	//const GameResource::C_Model* pModel = m_pModelResource->GetModelData()->GetModel("M_woodBox_UNDAMAGED");
	S32 modelIndex = 0;
	if(m_pModelResource->GetModelData()->GetModelCount() > 1)
		modelIndex = 1;

	const GameResource::C_Model* pModel = m_pModelResource->GetModelData()->GetModel(modelIndex);

	m_pModelElement = pModel->GetElement(0);

	if(m_pModelInstance == NULL)
	{
		SceneGraph::C_SceneGraph* pSceneGraph = GetSceneGraph();
		m_pModelInstance = new WorldRender::C_ModelInstance( m_pModelResource->GetRenderModelData(), pModel->GetIndex(), pSceneGraph );
		m_pModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, E_RenderPass_CustomSolid);
		
		// this sets all elements to same matrix
		SceneGraph::Insert(pSceneGraph, m_pModelInstance);
		m_pModelInstance->SetElementMatrix(-1, m_pModelElement->GetMatrix() * m_WorldMatrix );
		
		m_LocalBoundingSphere = m_pModelElement->GetLocalBounds().BoundingSphere();
		m_pModelInstance->SetWorldSphere( m_LocalBoundingSphere * m_pModelElement->GetMatrix() * m_WorldMatrix);
		
		m_pModelInstance->SetNearFarDistance(0.0f, 100.0f);
	}

	// create phsyics body
	{
		//m_pModelResource->GetShapeDescriptor();

		const GameResource::C_ModelElement* pElement = pModel->GetElement(0);

		const Gravity::C_ImportShape* pImportShape = pElement->GetPhysicsImportShape();

		if (pImportShape)
		{
			Gravity::C_ShapeDesc* pDesc = pImportShape->GetShapeDesc();
			/*Gravity::C_ContinuousShapeDesc contDesc;
			contDesc.m_pDiscreteShape = _pDesc;
			contDesc.m_pContinuousShape = ((Gravity::C_CompositeDesc*)_pDesc)->shapes[0].m_pShapeDesc;

			Gravity::C_ShapeDesc* pDesc = &contDesc;*/

			pDesc->RemapShape();
			pDesc->SetRemapped();
			
			const Maths::HF32 mass((F32)g_dbActorMass);

			SharedPhysics::C_Deactivator deactivator;
			deactivator.m_ActivateThresholdSq = mass * 0.05f;
			deactivator.m_SleepThresholdSq = mass * 0.01f;

			m_pBody = GetPhysicsScene()->CreateBody();

			m_pBody->SetExternalConstants(GetPhysicsExternalConstants());

			Gravity::C_BodyHelper bodyHelper( pDesc );

			Maths::HMat43 bodyMat = m_WorldMatrix;

			bodyHelper.CreateBody( mass, bodyMat.AW(), (Maths::HMat33)bodyMat, pDesc->InertiaTensor(mass), m_pBody, &PhysicsConstants::kDynamicObjectCollisionFilter);
			//m_pBody->SetMass(100.f);

			m_pBody->SetDeactivator(deactivator);
			GetPhysicsScene()->AddBody(m_pBody);
			m_pBody->Slop( PhysicsConstants::kDefaultSlop * 0.01f );
			m_pBody->Baumgarte( PhysicsConstants::kDefaultBaumgarte );
			m_pBody->Activate();
			m_pBody->SetAngularDampingCoefficient(0.99f);
			m_pBody->SetLinearDampingCoefficient(0.995f);
			m_pBody->SetMaxAngularVelocity(10.f);
			m_pBody->SetContinuousSpeedThreshold(2.5f);

			m_pBody->ElementMatrix( m_WorldMatrix );

			m_pPhysicsBodyUserData = ::Actors::C_SimplePhysicsBodyUserData::Create(this);
			m_pBody->SetUserData( m_pPhysicsBodyUserData );

			//bodyHelper.MakeFloat(300.f, 30.f, 4.f, m_pBody, 1.f, 0.5f);

		}
	}

	// register this actor's visibility detection with the model instance
	if (!m_pVisDetect)
		m_pVisDetect = ::Actors::C_VisibilityDetector::CreateVisDetect(GetHandle());
	m_pModelInstance->SetVisDetect(m_pVisDetect);

	AddSubscriber( GameMessaging::E_GameMsg_Update );
}

void C_ModelTestActor::DestroyModel()
{
	SAFE_DELETE_PTR( m_pEnvSampler );

	if( m_pModelInstance )
	{
		SceneGraph::Remove(GetSceneGraph(), m_pModelInstance);
		delete m_pModelInstance;
		m_pModelInstance = NULL;
	}

	if ( m_pBody != NULL )
	{
		if ( m_pPhysicsBodyUserData != NULL )
		{
			m_pBody->SetUserData( NULL );
			::Actors::C_SimplePhysicsBodyUserData::Destroy( m_pPhysicsBodyUserData );
			m_pPhysicsBodyUserData = NULL;
		}

		GetPhysicsScene()->RemoveBody( m_pBody );
		m_pBody = NULL;
	}
}

bool C_ModelTestActor::InitialiseImpl()
{
	if (!C_GameActor::InitialiseImpl())
		return false;

	const C_ModelTestActorCreate * pCreate = GetCreationData();

	m_WorldMatrix = pCreate->GetCreationMatrix();

	// create resource loader
	{
		// use single resource item component
		Component::C_ComponentCreatorFactoryHelper<Components::C_ResourceItemCreate> pCreator;
		pCreator->m_Desc.m_Name = pCreate->GetModelFilename();
		pCreator->m_Desc.m_Type = GameResource::C_ModelResource::StaticGetResourceTypeId();
		pCreator->m_Desc.SetPrime( true );

		// use resource list component
		//Component::C_ComponentCreatorFactoryHelper<Components::C_ResourceListCreate> pCreator;
		//pCreator->m_List.Reserve( 1 );
		//pCreator->AddItem( pModelTestCreate->GetModelFilename(), GameResource::C_ModelResource::StaticGetResourceTypeId(), Components::E_ResourceFlag_Prime );

		if( InitialiseComponent( pCreator.Get() ) == NULL )
			return false;
	}

	return true;
}

bool C_ModelTestActor::CreateImpl()
{
	if (!C_GameActor::CreateImpl())
		return false;

	//const C_ModelTestActorCreate * pCreate = GetCreationData();

	m_pModelResource = QueryInterface<Components::I_ResourceLoader>()->GetResource<GameResource::C_ModelResource>( 0 );
	
	if (m_pModelResource != NULL)
		CreateModel();

	return true;
}


} // namespace Actors

} // namespace Game

