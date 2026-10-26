#include "stdafx.h"

#include <Game/GameActors/CharacterTestActor.h>

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>
#include <misc_shared/messaging/GameMessaging.h>

#include <Resource/Resource.h>
#include <game_resource/ModelResource.h>
#include <world_render/ModelInstance.h>
#include <SceneGraph/SceneGraph.h>
#include <game_resource/ModelData.h>

#include <Game/GameWorld/GameWorld.h>

#include <gravity/sdk/BodyHelper.h>
#include <gravity/importer/Importer.h>
#include <Game/GameDef/PhysicsConstants.h>

#include <Game/GameControls/GameControls.h>
#include "gravity/sdk/charactercontroller.h"
#include <Game/GameDef/Materials.h>
#include <framework/application/ApplicationBase.h>
#include <camera2.h>
#include <camera2/stock/CharacterCamera.h>
#include "game/MGCam/CameraSystem.h"
#include "game/MGCam/CameraSwitcher.h"
#include "game/MGCam/PhysicsChecks.h"

#include <framework/application/ApplicationBase.h>
#include <GameApp.h>
#include "gravity/sdk/ShapeDescriptors.h"

namespace Game
{

namespace Actors
{

// Actor Info

C_CharacterTestActor::C_CharacterTestActor()
  : m_pModelResource(NULL)
  , m_pModelInstance(NULL)
  , m_pBody(NULL)
{
	m_pModelResource = 0;
	m_pModelInstance = 0;
	m_pModelElement = 0;
	m_pBody = 0;

	m_pCamera = 0;
	m_pPhysicsChecks = 0;
}

C_CharacterTestActor::~C_CharacterTestActor()
{
	if( m_pModelResource != NULL )
		DestroyModel();
	ExitCamera();
}

bool C_CharacterTestActor::OnMessage(const Messaging::C_MessageBase *pMessage)
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

	case GameMessaging::E_GameMsg_Update:
		if(m_pModelInstance != NULL)
		{
			//m_WorldMatrix = Maths::HMat43(m_pBody->Orientation(), m_pBody->Position());
			if (m_pBody)
				m_WorldMatrix = m_pBody->ElementMatrix();//m_pModelElement->GetMatrix() * m_pBody->ElementMatrix();
			m_pModelInstance->SetWorldSphere(Maths::C_Sphere(m_WorldMatrix.Translation(), 2.0f));
			m_pModelInstance->SetElementMatrix(-1, m_WorldMatrix);

			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix( m_WorldMatrix );		


			// Update character controller
			{
				bool inputActive = MGCam::System().DebugFeed().PadUsed() != 0;

				const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
				Maths::HMat43 cameraMatrix = view.DebugFeed().Mat43();

				Maths::HVec3 xDir = cameraMatrix.AX();
				Maths::HVec3 zDir = cameraMatrix.AZ();
				xDir.Y( 0.0f );
				zDir.Y( 0.0f );
				xDir.Normalise();
				zDir.Normalise();

				F32 padX = 0.f, padY = 0.f;
				if (inputActive)
				{
					padX = Input::GetGameInput(0)->GetAxisSignedValue( Game::E_Axis_MoveX );
					padY = Input::GetGameInput(0)->GetAxisSignedValue( Game::E_Axis_MoveY );
				}
				
				const Maths::HF32 targetSpeed(3.f);
				Maths::HVec3 direction = xDir * padX + zDir * padY;
				Maths::HVec3 targetVelocity = direction * targetSpeed;

				m_CharacterController.SetTargetVelocity( targetVelocity, Maths::Constants<Maths::HVec3>::Zero() );

				if( inputActive && Input::GetGameInput(0)->GetButtonPressedDebounced( E_Button_Jump ) )
				{
					m_CharacterController.Jump(Maths::HF32(5.f));
				}
			}

			// update cam
			{
				// Chase camera:
				Cam2::C_CamObj object;
				object.position = m_WorldMatrix.AW();
				object.orient = Maths::C_Quaternion( Maths::HMat33(m_WorldMatrix) );

				m_pCamera->SetObject(object);

				F32 sx = 0.f, sy = 0.f;
				if (MGCam::System().DebugFeed().PadUsed() != 0)
				{
					sx = Input::GetGameInput(0)->GetAxisSignedValue( E_Axis_CameraLookX );
					sy = Input::GetGameInput(0)->GetAxisSignedValue( E_Axis_CameraLookY );
				}
				m_pCamera->SetLookAround(sx * 2.f, sy * 2.f);

				m_pCamera->Update(C_GameApp::GetInstance()->GetTimer().GetFrameTime());
			}
		}
		
		return true;
	}

	return C_GameActor::OnMessage(pMessage);
}

void C_CharacterTestActor::CreateModel()
{
	//const GameResource::C_Model* pModel = m_pModelResource->GetModelData()->GetModel("M_woodBox_UNDAMAGED");
	S32 modelIndex = 0;
	if(m_pModelResource->GetModelData()->GetModelCount() > 1)
		modelIndex = 1;

	const GameResource::C_Model* pModel = m_pModelResource->GetModelData()->GetModel(modelIndex);

	m_pModelElement = pModel->GetElement(0);

	if(m_pModelInstance == NULL)
	{
		m_pModelInstance = new WorldRender::C_ModelInstance( m_pModelResource->GetRenderModelData(), pModel->GetIndex(), GetSceneGraph() );
		
		// this sets all elements to same matrix
		m_pModelInstance->SetElementMatrix(-1, m_pModelElement->GetMatrix() * m_WorldMatrix );
		m_pModelInstance->SetWorldSphere(Maths::C_Sphere(m_WorldMatrix.Translation(), 10.0f));
		
		SceneGraph::Insert(GetSceneGraph(), m_pModelInstance);
	}

	// create phsyics body
	{
		/*
		const GameResource::C_ModelElement* pElement = pModel->GetElement(0);

		const Gravity::C_ImportShape* pImportShape = pElement->GetPhysicsImportShape();

		if (pImportShape)
		{
			Gravity::C_ShapeDesc* pDesc = pImportShape->GetShapeDesc();
			pDesc->RemapShape();

			SharedPhysics::C_Deactivator deactivator;
			deactivator.m_ActivateThresholdSq = 0.005f;
			deactivator.m_SleepThresholdSq = 0.001f;

			m_pBody = GetPhysicsScene()->CreateBody();

			m_pBody->SetExternalConstants(GetPhysicsExternalConstants());

			Gravity::C_BodyHelper bodyHelper( pDesc );

			Maths::HMat43 bodyMat = m_WorldMatrix;

			bodyHelper.CreateBody( bodyMat.AW(), (Maths::HMat33)bodyMat, pImportShape->GetImportIneriaTensor(), m_pBody, &PhysicsConstants::kDynamicObjectCollisionFilter);
			m_pBody->SetMass(100.f);

			m_pBody->SetDeactivator(deactivator);
			GetPhysicsScene()->AddBody(m_pBody);
			m_pBody->Slop( PhysicsConstants::kDefaultSlop );
			m_pBody->Baumgarte( PhysicsConstants::kDefaultBaumgarte );
			m_pBody->Activate();

			m_pBody->ElementMatrix( m_WorldMatrix );
		}
		*/
		
		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = 0.0f;//0.005f;
		deactivator.m_SleepThresholdSq = 0.0f;//0.001f;
		Maths::HF32 height(0.5f);
		Maths::HF32 radius(0.4f);
		Maths::HF32 rayHeight(0.4f);
		Maths::HF32 rayRadius(0.3f);
		Maths::HF32 zero(0.0f);
		Maths::HF32 mass(40.0f);
		m_pBody = GetPhysicsScene()->CreateBody();
		m_pBody->SetExternalConstants(GetPhysicsExternalConstants());
		Gravity::C_CharacterControllerDesc controllerDesc;
		controllerDesc.m_CapsuleHeight = height;
		controllerDesc.m_CapsuleRadius = radius;
		controllerDesc.m_RayHeight = Maths::Max(rayHeight - rayRadius, zero);
		controllerDesc.m_RayRadius = rayRadius;
		controllerDesc.m_MaxCosAngle = Maths::HF32(0.5f); // cos 60
		controllerDesc.m_MinPen = rayHeight;
		controllerDesc.m_FeetMaterial = 0;
		Gravity::C_BodyHelper bodyHelper(&controllerDesc);
		
		Maths::HMat33 orient( Maths::AxisY( Maths::C_Normal( 0.0f, 1.0f, 0.0f) ), Maths::AxisZ( Maths::C_Normal( m_WorldMatrix.AZ() ) ));

		bodyHelper.CreateBody(mass, m_WorldMatrix.AW(), orient, m_pBody, &PhysicsConstants::kCharacterCollisionFilter, E_PhysicsMaterial_CharacterController, E_EffectMaterial_Default);
		m_pBody->SetDeactivator(deactivator);
		m_pBody->Slop( 0.00000001f ); //PhysicsConstants::kDefaultSlop );
		m_pBody->Baumgarte( 2.0f );//PhysicsConstants::kDefaultBaumgarte ); // 2.0f
		GetPhysicsScene()->AddBody( m_pBody );

		m_CharacterController = Gravity::C_CharacterController(m_pBody, Maths::HF32(10.f), Maths::HF32(500.f));
		GetPhysicsScene()->AddCharacterController(&m_CharacterController);
	}

	InitCamera();

	AddSubscriber( GameMessaging::E_GameMsg_Update );
}

void C_CharacterTestActor::DestroyModel()
{
	if( m_pModelInstance )
	{
		SceneGraph::Remove(GetSceneGraph(), m_pModelInstance);
		delete m_pModelInstance;
		m_pModelInstance = NULL;
	}
}

bool C_CharacterTestActor::InitialiseImpl()
{
	if (!C_GameActor::InitialiseImpl())
		return false;

	const C_CharacterTestActorCreate * pCreate = (const C_CharacterTestActorCreate *)(GetCreationData().GetData());

	m_WorldMatrix = pCreate->GetCreationMatrix();

	// create resource loader
	{
		// use single resource item component
		Component::C_ComponentCreatorFactoryHelper<Components::C_ResourceItemCreate> pCreator;
		pCreator->m_Desc.m_Name = pCreate->m_ModelFilename;
		pCreator->m_Desc.m_Type = GameResource::C_ModelResource::StaticGetResourceTypeId();

		// use resource list component
		//Component::C_ComponentCreatorFactoryHelper<Components::C_ResourceListCreate> pCreator;
		//pCreator->m_List.Reserve( 1 );
		//pCreator->AddItem( pModelTestCreate->m_ModelFilename, GameResource::C_ModelResource::StaticGetResourceTypeId() );

		if( InitialiseComponent( pCreator.Get() ) == NULL )
			return false;
	}

	return true;
}

bool C_CharacterTestActor::CreateImpl()
{
	if (!C_GameActor::CreateImpl())
		return false;

	const C_CharacterTestActorCreate * pCreate = (const C_CharacterTestActorCreate *)(GetCreationData().GetData());

	// load resources
	SendMessageToComponent( Components::E_ResourceIF_Loader, &ActorMsg::C_ResourceLoadMsg( 0 ) );

	return true;
}

void C_CharacterTestActor::InitCamera()
{
	m_pCamera = new Cam2::C_CharacterCamera();
	if (m_pCamera == 0)
		return;
	
	m_pPhysicsChecks = new MGCam::C_PhysicsChecks(GetPhysicsScene(), 0);
	m_pCamera->SetPhysics(m_pPhysicsChecks);
	
	MGCam::System().GameFeedShow(m_pCamera);
	MGCam::Switcher().SetType(MGCam::E_DebugCam_GameFeed);
}

void C_CharacterTestActor::ExitCamera()
{
	MGCam::System().GameFeedRemove(m_pCamera); // Make sure not being shown
	SAFE_DELETE_PTR(m_pCamera)
	SAFE_DELETE_PTR(m_pPhysicsChecks)
}

} // namespace Actors

} // namespace Game

