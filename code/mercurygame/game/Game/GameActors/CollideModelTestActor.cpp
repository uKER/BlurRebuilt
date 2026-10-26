#include "stdafx.h"

#include <Game/GameActors/CollideModelTestActor.h>

#include <actor_system/ActorSys.h>

#include <shared_actors/XtActorBase.h>
// #include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
// #include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
// #include <shared_actors/Components/Messages/ResourceMessages.h>

#include <shared_actors/Components/Messages/PhysicsMessages.h>
#include <shared_actors/Components/Interfaces/I_PhysicsBodyComponents.h>
#include <shared_actors/Components/Creation/ModelResourceComponentCreate.h>

#include <shared_actors/Components/Creation/RenderComponentsCreate.h>
#include <shared_actors/Components/Interfaces/I_RenderComponents.h>
#include <shared_Actors/Components/Messages/RenderMessages.h>

#include <Game/ActorComponents/Creation/PhysicsBodyComponentCreate.h>

#include <resource/Resource.h>
#include <game_resource/ModelData.h>
#include <game_resource/ModelResource.h>
#include <world_render/ModelInstance.h>
#include <scenegraph/SceneGraph.h>

#include <gravity/sdk/BodyHelper.h>
#include <gravity/importer/Importer.h>

#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>

#include <Game/GameWorld/GameWorld.h>

#include <Game/GameDef/PhysicsConstants.h>
#include <world_render/Scene.h>

#include "GameApp.h"
#include "Game/GameWorld/GameWorldLayer.h"

XT_TYPE(Game::Actors::C_CollideModelTestActorCreate, "CollideModelTestActor")
	XT_BASE(::Actors::C_PosActorCreate)
	XT_FIELD(m_GameModel, "Model")
XT_END_TYPE


XT_TYPE(Game::Actors::C_PlatformActorCreate, "PlatformActor")
	XT_BASE(::Actors::C_PosActorCreate)
	XT_FIELD(m_PlatformName, "PlatformName")
XT_END_TYPE

namespace Game
{

namespace Actors
{

Debug::Bool g_DebugRenderCollideModelActor("ActorDebug.Actors.CollideModel.DebugRender", false);

// Actor Info


C_CollideModelTestActor::C_CollideModelTestActor()
{

}

C_CollideModelTestActor::~C_CollideModelTestActor()
{

}

bool C_CollideModelTestActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
	case E_PhysicsBodyNotification_BodyCreated:
		break;
	case E_PhysicsBodyNotification_TransformChanged:
		{
			const C_PhysicsBodyTransformChanged *pPhysicsTransMesg = Messaging::MessageCast<C_PhysicsBodyTransformChanged>(pMessage);

			m_WorldMatrix = pPhysicsTransMesg->GetValue();
			C_SetRenderMatrixMsg renderMsg(m_WorldMatrix);
			SendMessageToComponent(Components::E_IF_Render,&renderMsg );
		}
		break;
	
	default:
		return C_GameActor::OnMessage(pMessage);
	}

	return true;
}

bool C_CollideModelTestActor::InitialiseImpl()
{
	if (!C_GameActor::InitialiseImpl())
		return false;

	const C_CollideModelTestActorCreate * pCreate = GetCreationData();

	m_WorldMatrix = pCreate->GetCreationMatrix();

	// create Model file Resource
	{
		// use single resource item component
		Component::C_ComponentCreatorFactoryHelper<Components::C_FileModelResourceCreate> pCreator;

		pCreator->m_ModelHandle = pCreate->m_GameModel;
		if( InitialiseComponent( pCreator.Get() ) == NULL )
			return false;
	}
	// create model renderer
	{
		// use single resource item component
		Component::C_ComponentCreatorFactoryHelper<Components::C_ModelSingleElementRenderCreate> pCreator;

		//Store(m_WorldMatrix, pCreator->m_WorldMatrix);
		m_WorldMatrix = pCreator->m_WorldMatrix;
		pCreator->m_ModelIndex = 1;
		pCreator->m_ModelElementIndex = 0;
		Maths::Store(Maths::HVec3(0,0,0),&pCreator->m_RenderOffset);

		if( InitialiseComponent( pCreator.Get() ) == NULL )
			return false;
	}

	// create PBC
	{
		Component::C_ComponentCreatorFactoryHelper<Components::C_ModelPhysicsBodyCreateInfo> pCreator;

		pCreator->m_BodyType = ActorPhysics::E_BodyType_Dynamic;
		pCreator->m_Matrix = m_WorldMatrix;	
		pCreator->m_ModelIndex = 1;
		pCreator->m_ModelElementIndex = 0;
		pCreator->m_Mass = 100.0f;

		if( InitialiseComponent( pCreator.Get() ) == NULL )
			return false;
	}

	return true;
}

bool C_CollideModelTestActor::CreateImpl()
{
	if (!C_GameActor::CreateImpl())
		return false;

	//AddSubscriber( GameMessaging::E_GameMsg_Update );
	return true;
}

// Platform Actor
void C_PlatformActorCreate::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_PosActorCreate::DoSorceryCallback(pCallbackFn);

	if (m_pSorceryMovementComponentCreate != NULL)
		DoComponentSorceryCallback(pCallbackFn, *m_pSorceryMovementComponentCreate, &m_pMovementCreator);
}

C_PlatformActor::C_PlatformActor()
{

}

C_PlatformActor::~C_PlatformActor()
{

}

bool C_PlatformActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
	case E_PhysicsBodyNotification_BodyCreated:
		break;
	case E_PhysicsBodyNotification_TransformChanged:
		{
			const C_PhysicsBodyTransformChanged *pPhysicsTransMesg = Messaging::MessageCast<C_PhysicsBodyTransformChanged>(pMessage);

			m_WorldMatrix = pPhysicsTransMesg->GetValue();
			C_SetRenderMatrixMsg renderMsg(m_WorldMatrix);
			SendMessageToComponent(Components::E_IF_Render,&renderMsg );
		}
		break;

	default:
		return C_GameActor::OnMessage(pMessage);
	}

	return true;
}

bool C_PlatformActor::InitialiseImpl()
{
	if (!C_GameActor::InitialiseImpl())
		return false;

	const C_PlatformActorCreate * pCreate = GetCreationData();
	U32 platformIndex = 0;
	const GameResource::C_Model * pPlatformModel = C_GameApp::GetInstance()->GetGameWorldLayer()->GetLevelResource()->GetModelData()->GetModel(pCreate->m_PlatformName.Text());

	if(pPlatformModel)
		platformIndex = pPlatformModel->GetIndex();

	m_WorldMatrix = pCreate->GetCreationMatrix();

	// create Level Model Resource
	{
		// use single resource item component
		Component::C_ComponentCreatorFactoryHelper<Components::C_LevelModelResourceCreate> pCreator;

		if( InitialiseComponent( pCreator.Get() ) == NULL )
			return false;
	}
	// create model renderer
	{
		// use single resource item component
		Component::C_ComponentCreatorFactoryHelper<Components::C_ModelSingleElementRenderCreate> pCreator;

		//Store(m_WorldMatrix, pCreator->m_WorldMatrix);
		m_WorldMatrix = pCreator->m_WorldMatrix;
		pCreator->m_ModelIndex = platformIndex;
		pCreator->m_ModelElementIndex = 0;
		Maths::Store(Maths::HVec3(0,0,0),&pCreator->m_RenderOffset);

		if( InitialiseComponent( pCreator.Get() ) == NULL )
			return false;
	}

	// create PBC
	{
		Component::C_ComponentCreatorFactoryHelper<Components::C_ModelPhysicsBodyCreateInfo> pCreator;

		pCreator->m_BodyType = ActorPhysics::E_BodyType_Dynamic;
		pCreator->m_Matrix = m_WorldMatrix;	
		pCreator->m_ModelIndex = platformIndex;
		pCreator->m_ModelElementIndex = 0;
		pCreator->m_Mass = 100.0f;

		if( InitialiseComponent( pCreator.Get() ) == NULL )
			return false;
	}

	// create movement component
	if(pCreate->m_pSorceryMovementComponentCreate)
	{
		if( InitialiseComponent( pCreate->m_pMovementCreator ) == NULL )
			return false;
	}

	return true;
}

bool C_PlatformActor::CreateImpl()
{
	if (!C_GameActor::CreateImpl())
		return false;

	//AddSubscriber( GameMessaging::E_GameMsg_Update );
	return true;
}
} // namespace Actors

} // namespace Game

