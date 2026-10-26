/////////////////////////////////////////////////////////////
// VehicleActor
/////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <Game/GameActors/VehicleActor.h>

#include <game_resource/ModelData.h>

#include <Vehicle/VehicleModel.h>
#include <Vehicle/VehicleModelManager.h>
#include <Vehicle/Setup.h>

#include <RacingPhysics/RacingPhysics.h>

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>
#include <shared_actors/Utility/VisibilityDetection.h>
#include <misc_shared/messaging/GameMessaging.h>

#include <world_render/ModelInstance.h>
#include <SceneGraph/SceneGraph.h>

#include <Game/GameControls/GameControls.h>
#include <camera2/stock/ChaseCamera2.h>
#include <camera2/stock/DebugFeed.h>
#include <game/MGCam/CameraSystem.h>
#include <game/MGCam/VehicleCamera.h>
#include <GameApp.h>
#include <Game/GameDef/PhysicsConstants.h>
#include <framework/application/ApplicationBase.h>

Maths::HMat43 RPToMaths(const RacingPhysics::Axis* p_pAxis)
{
	Maths::HVec3 ax(p_pAxis->AxisX().X(), p_pAxis->AxisX().Y(), p_pAxis->AxisX().Z());
	Maths::HVec3 ay(p_pAxis->AxisY().X(), p_pAxis->AxisY().Y(), p_pAxis->AxisY().Z());
	Maths::HVec3 az(p_pAxis->AxisZ().X(), p_pAxis->AxisZ().Y(), p_pAxis->AxisZ().Z());
	Maths::HVec3 aw(p_pAxis->Position().X(), p_pAxis->Position().Y(),	p_pAxis->Position().Z());
	return Maths::HMat43(ax, ay, az, aw);	
}

Maths::HVec3 RPToMaths(const RacingPhysics::Vector3R* p_pVector)
{
	return Maths::HVec3(p_pVector->X(), p_pVector->Y(), p_pVector->Z());
}

Maths::HVec3 RPToMaths(const RacingPhysics::Point3R* p_pPoint)
{
	return Maths::HVec3(p_pPoint->X(), p_pPoint->Y(), p_pPoint->Z());
}

Maths::HVec3 RPToMaths(const RacingPhysics::Normal3R* p_pNormal)
{
	return Maths::HVec3(p_pNormal->X(), p_pNormal->Y(), p_pNormal->Z());
}

namespace Game
{

namespace Actors
{

/* 
===============================================================
					class C_VehicleActor

NOTES: 
===============================================================
*/

/*
===============
NAME: C_VehicleActor

NOTE: Constructor
===============
*/
C_VehicleActor::C_VehicleActor()
{
	m_pVehicleModel		= NULL;
	m_pVehiclePhysics	= NULL;
	m_pChaseCamera		= NULL;
	m_pCollisionBody	= NULL;
	m_pModelResource	= NULL;
	m_pVisDetect		= NULL;
}

/*
===============
NAME: ~C_VehicleActor

NOTE: Destructor
===============
*/
C_VehicleActor::~C_VehicleActor()
{
	Destroy();
}

/*
===============
NAME: Update

NOTE: Update
===============
*/
void C_VehicleActor::Update()
{
	//if (!MGCam::System().DebugFeed().UsesInput() && m_pCollisionBody && m_pVehiclePhysics && Input::GetGameInput(0)->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonX))
	//{
	//	RacingPhysics::Axis axis((Maths::HMat44)m_BirthPlace);
	//	axis.SetPosition(RacingPhysics::Point3R(axis.Position().X(), axis.Position().Y() + 5.0f, axis.Position().Z()));
	//	m_pVehiclePhysics->SetAxis(axis);
	//	Maths::C_Quaternion quat((Maths::HMat33)m_BirthPlace);
	//	Maths::HVec3 pos(axis.Position().X(), axis.Position().Y(), axis.Position().Z());
	//	m_pCollisionBody->Position(pos);
	//	m_pCollisionBody->Orientation(quat);
	//	m_pCollisionBody->LinearVelocity(Maths::HVec3(0.0f, 0.0f, 0.0f));
	//	m_pCollisionBody->AngularVelocity(Maths::HVec3(0.0f, 0.0f, 0.0f));
	//}

	//if (m_pCollisionBody)
	//	m_Matrix = m_pCollisionBody->ElementMatrix();

	// m_pVehiclePhysics->XtReload();

	MathsStorage::Matrix44 matrix44 = m_pVehiclePhysics->GetAxis().GetMatrix();

	Maths::HMat44 hMat44;
	hMat44 = Maths::Load<Maths::HMat44>(&matrix44);
	m_Matrix = (Maths::HMat43)hMat44;
	
	//Set everything for now, don't worry about wheels to start off with
	for (U32 iInstance = 0; iInstance < m_pModelInstances.Size(); iInstance++)
	{				
		WorldRender::C_ModelInstance* pModelInstance = m_pModelInstances[iInstance];

		pModelInstance->SetWorldSphere(Maths::C_Sphere(m_Matrix.Translation(), 2.0f));
		if(m_pVisDetect->IsVisible())
			pModelInstance->SetElementMatrix(-1, m_Matrix);
		//for (U32 iElement = 0; iElement < pModelInstance->GetNumElements(); iElement++)
		//{
		//	//Maths::HMat43 matrix = pModelInstance->GetElementMatrix(iElement);
		//	Vehicle::C_Component* pComponent = m_pVehicleModel->GetComponentList().GetComponent
		//	m_RestMatrix

		//	matrix = matrix * m_Matrix;
		//	
		//	pModelInstance->SetElementMatrix(iElement, matrix);
		//}
	}

	m_pVehicleModel->GetComponentList().UpdateMatrices(m_Matrix, m_pVisDetect);

	SynchData();

	//Update camera
	{
		Cam2::C_CamObj object;
		const RacingPhysics::Vehicle& vehicle = *m_pVehiclePhysics;
		
		RacingPhysics::Axis axis = vehicle.GetAxis();
		if( VEHICLE_FORWARD_Z_SIGN 1.0f < 0.0f )
		{
			axis.RotateLocalY180();
		}
			
		object.position = RacingPhysics::ConvertHVec(vehicle.GetCofG());
			
		RacingPhysics::Quaternion quat = RacingPhysics::Quaternion(axis);
		object.orient = Maths::C_Quaternion((float)quat.A().X(), (float)quat.A().Y(), (float)quat.A().Z(), quat.W());

		m_pChaseCamera->SetObject(object);
		m_pChaseCamera->Update(C_GameApp::GetInstance()->GetTimer().GetFrameTime());
	}
}

#ifndef CFG_GOLD
void C_VehicleActor::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if (!m_pVehiclePhysics)
		return;

	m_pVehiclePhysics->DisplayDebug();
}
#endif

Debug::Bool g_debugVehicleDisplayDebugModels("Vehicle.Render.Debug.DisplayModels", false);

/*
===============
NAME: MessageHandler

NOTE: Handles messages
===============
*/
bool C_VehicleActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
	case GameMessaging::E_GameMsg_Update:
		{
			Update();

			if (g_debugVehicleDisplayDebugModels)
			{
				//Maths::HMat43 mElement = m_pModelElement->GetMatrix();
				//Maths::HMat43 mModelDebug = mElement.InverseRotTrans() * m_Matrix;
				//m_pModelResource->GetModelData()->DebugRender(DbUtil::DefaultDebugRenderSys()->Deferred(), mModelDebug);
				m_pModelResource->GetModelData()->DebugRender(DbUtil::DefaultDebugRenderSys()->Deferred(), m_Matrix);
			}
			return true;
		}
		break;
	case ActorMsg::E_ResourceMsg_Loaded:
		{
			if (Messaging::MessageCast<ActorMsg::C_ResourceLoadedMsg>(pMessage)->GetValue() == 0 )
			{
				if (m_pModelResource != NULL)
					Destroy();

				m_pModelResource = QueryInterface<Components::I_ResourceLoader>()->GetResource<GameResource::C_ModelResource>( 0 );
				
				if (m_pModelResource != NULL)
					Create();
			}
			return true;
		}
		break;
	case ActorMsg::E_ResourceMsg_Unloaded:
		{
			if (Messaging::MessageCast<ActorMsg::C_ResourceUnloadedMsg>(pMessage)->GetValue() == 0 )
			{
				if (m_pModelResource != NULL)
					Destroy();

				m_pModelResource = NULL;
			}
			return true;
		}
		break;
	}

	return C_GameActor::OnMessage(pMessage);
}

/*
===============
NAME: SynchData

NOTE: Syncs racing physics data with model data - done in a thread-safe happy place
===============
*/
void C_VehicleActor::SynchData()
{
	//Need to accept these controls in actor-message form, not hard-code like this...
	RacingPhysics::CarControls& controls = ((RacingPhysics::Car*)m_pVehiclePhysics)->Controls();
	controls.Clear();

	if (MGCam::System().DebugFeed().PadUsed() != 0)
	{
		controls.throttle	= Input::GetGameInput(0)->GetButtonValue(Game::E_Button_Throttle);
		controls.steer		= Input::GetGameInput(0)->GetAxisSignedValue(Game::E_Axis_Steering);
		controls.brake		= Input::GetGameInput(0)->GetButtonValue(Game::E_Button_Brake);
		controls.handbrake	= Input::GetGameInput(0)->GetButtonValue(Game::E_Button_Handbrake);
		controls.gearUp		= Input::GetGameInput(0)->GetButtonPressedDebounced(Game::E_Button_GearUp);
		controls.gearDown	= Input::GetGameInput(0)->GetButtonPressedDebounced(Game::E_Button_GearDown);
	}

	m_pVehicleModel->GetModelInput()->m_mMatrix					= RPToMaths(&m_pVehiclePhysics->GetAxis());

	//m_pVehicleModel->GetModelInput()->m_vVelocity				= RPToMaths(&m_pVehiclePhysics->Velocity());
	//m_pVehicleModel->GetModelInput()->m_vAcceleration			= RPToMaths(&m_pVehiclePhysics->Acceleration());
	//m_pVehicleModel->GetModelInput()->m_vAngularVelocity		= RPToMaths(&m_pVehiclePhysics->AngularVelocity());
	//m_pVehicleModel->GetModelInput()->m_vAngularAcceleration	= RPToMaths(&m_pVehiclePhysics->AngularAcceleration());

	//m_pVehicleModel->GetModelInput()->m_iGear					= m_pVehiclePhysics->Gear();

	//m_pVehicleModel->GetModelInput()->m_fRevs					= m_pVehiclePhysics->Gear();
	//m_pVehicleModel->GetModelInput()->m_fThrottle				= controls.throttle;
	//m_pVehicleModel->GetModelInput()->m_fBrake					= controls.brake;
	//m_pVehicleModel->GetModelInput()->m_fSpeed					= m_pVehiclePhysics->Speed();

	//m_pVehicleModel->GetModelInput()->m_bHandbrake				= controls.handbrake;

	//m_pVehicleModel->GetModelInput()->m_bShouldUpdate			= m_pVehiclePhysics->ShouldUpdate();
	//m_pVehicleModel->GetModelInput()->m_bShouldRender			= m_pVehiclePhysics->ShouldRender();


	if (m_pVehiclePhysics->GetNumWheels() == m_pVehicleModel->GetModelInput()->m_Wheels.Size())
	{
		for (U32 iWheel = 0; iWheel < m_pVehiclePhysics->GetNumWheels(); iWheel++)
		{
			Vehicle::C_WheelInput* pWheelInput	= &m_pVehicleModel->GetModelInput()->m_Wheels[iWheel];

			Vehicle::E_PartCode ePartCode		= Vehicle::E_PartCode_Wheel;
			Vehicle::E_PartSubCode eLocCode	= Vehicle::E_PartSubCode_None;

			switch (iWheel)
			{
			case 0:
				eLocCode = Vehicle::E_PartSubCode_FrontLeft;
				break;
			case 1:
				eLocCode = Vehicle::E_PartSubCode_FrontRight;
				break;
			case 2:
				eLocCode = Vehicle::E_PartSubCode_BackLeft;
				break;
			case 3:
				eLocCode = Vehicle::E_PartSubCode_BackRight;
				break;
			}

			const RacingPhysics::CarWheel& wheel = ((RacingPhysics::Car*)m_pVehiclePhysics)->Wheel(iWheel);

			RacingPhysics::Axis axis;
			axis.SetRotation(wheel.GetRotationMatrix());
			axis.SetPosition(wheel.GetWheelPosition());
			
			pWheelInput->m_mMatrix				= RPToMaths(&axis);

			WorldRender::C_ModelInstance* pModelInstance = m_pVehicleModel->GetComponentList().GetModelInstance(ePartCode, eLocCode);
			
			if (pModelInstance)
			{
				pModelInstance->SetWorldSphere(Maths::C_Sphere(pWheelInput->m_mMatrix.Translation(), 2.0f));
				if(m_pVisDetect->IsVisible())
					pModelInstance->SetElementMatrix(-1, pWheelInput->m_mMatrix);
			}
			else
				bool breakHere = true;

			//for (U32 iElement = Vehicle::E_PartCode_Tyre; iElement < Vehicle::E_PartCode_BrakeCaliper; iElement++)
			//{
			//	ePartCode = (Vehicle::E_PartCode)iElement;

			//	Vehicle::C_Component* pComponent = &m_pVehicleModel->GetComponentList()[ePartCode];

			//	if (!pComponent)
			//		continue;

			//	WorldRender::C_ModelInstance* pModelInstance = m_pVehicleModel->GetComponentList().GetModelInstance(ePartCode, eLocCode);

			//	if (!pModelInstance || !pComponent->m_Nodes[eLocCode].m_pElement)
			//		continue;

			//	Maths::HMat43 componentMatrix = pWheelInput->m_mMatrix * pComponent->m_Nodes[eLocCode].m_RestMatrix;

			//	pModelInstance->SetWorldSphere(Maths::C_Sphere(pWheelInput->m_mMatrix.Translation(), 2.0f));
			//	pModelInstance->SetElementMatrix(pComponent->m_Nodes[eLocCode].m_pElement->GetIndex(), componentMatrix);
			//}
			/*WorldRender::C_ModelInstance* pModelInstance = m_pVehicleModel->GetComponentList().GetModelInstance(ePartCode, eLocCode);

			if (!pModelInstance)
				continue;

			RacingPhysics::CarWheel wheel = ((RacingPhysics::Car*)m_pVehiclePhysics)->Wheel(iWheel);

			RacingPhysics::Axis axis;
			axis.SetRotation(wheel.GetRotationMatrix());
			axis.SetPosition(wheel.GetWheelPosition());

			pWheelInput->m_mMatrix				= RPToMaths(&axis);

			pModelInstance->SetWorldSphere(Maths::C_Sphere(pWheelInput->m_mMatrix.Translation(), 2.0f));
			pModelInstance->SetElementMatrix(-1, pWheelInput->m_mMatrix);*/

			pWheelInput->m_bOnGround			= wheel.OnGround();

			if (pWheelInput->m_bOnGround)
			{
				pWheelInput->m_iSurfaceType			= (U32)wheel.SurfaceType();
				pWheelInput->m_iSurfaceModifier		= (U32)wheel.SurfaceModifier();

				pWheelInput->m_vGroundPosition		= RPToMaths(&wheel.GroundPosition());
			}
//			pWheelInput->m_vAngularVelocity		= RPToMaths(&wheel.AngularVelocity());
//			pWheelInput->m_vAngularAcceleration	= RPToMaths(&wheel.AngularAcceleration());

			

			
		}
	}
}

/*
===============
NAME: ResourceNotify

NOTE: ResourceNotify
===============
*/
//void C_VehicleActor::ResourceNotify(Resource::C_ResourceBase* p_pResource, const Resource::E_Notify p_Result, void* p_pData)
//{
//	//if ((U32)m_pVehicleModel == 0xcdcdcdcd)
//	//{
//	//	//SEVERE HACKAGE WTF!ONEONEONE
//	//	U32 nVehicles = Vehicle::C_VehicleModelManager::Instance().GetNumVehicles();
//	//	if (nVehicles == 0)
//	//		return;
//	//	m_pVehicleModel = Vehicle::C_VehicleModelManager::Instance().GetVehicleModel(nVehicles-1);
//	//}
//
//	switch (p_Result)
//	{
//		case Resource::E_Notify_DataReady:
//		{				
//			GameResource::C_ModelResource* pModelResource = m_ModelHandle;
//			if (p_pResource == pModelResource)
//			{
//				m_pModelResource = pModelResource;
//				//m_pVehicleModel->SetModelResource(m_pModelResource);
//				Create();
//			}
//		}
//		break;
//		case Resource::E_Notify_DataError:
//		case Resource::E_DataState_NoData:
//			if (p_pResource == m_pVehicleModel->GetModelResource())
//			{
//				Destroy();
//			}
//		break;
//	}
//	LOGINFO(Resource::g_Log, "Notify pResource=0x%08x, ['%s'] \"%s\" %d, 0x%08x", p_pResource, p_pResource->GetResourceTypeName().Chars(), p_pResource->GetResourceFilename().Chars(), p_Result, p_pData);
//}

//Debug::Bool g_debugSetChaseCamOnCarSpawn("Vehicle.SetChaseCamOnSpawn", false, Debug::Persist());

/*
===============
NAME: Create

NOTE: Create
===============
*/
void C_VehicleActor::Create()
{
	Vehicle::C_VehicleModelSetup setup;
	setup.m_bCreateDamageLayer	= true;
	setup.m_bCreateGameLayer	= true;
	setup.m_bCreateRenderLayer	= true;
	m_pVehicleModel = Vehicle::C_VehicleModelManager::Instance().AddVehicleModel(&setup);

	if (!m_pVehicleModel)
		return;

	// register this actor's visibility detection with the model instance
	if (!m_pVisDetect)
		m_pVisDetect = ::Actors::C_VisibilityDetector::CreateVisDetect(GetHandle());
	
	m_pVehicleModel->SetModelResource(m_pModelResource);

	m_pVehicleModel->GetComponentList().Clear();

	for (U32 iModel = 0; iModel < m_pModelResource->GetModelData()->GetModelCount(); iModel++)
	{
		const GameResource::C_Model* pModel = m_pModelResource->GetModelData()->GetModel(iModel);
		
		Util::C_String modelName = pModel->GetName();
		modelName.ConvertToLower();

		if (modelName == "m_collisionmesh")
		{
			CreateCollisionMesh(pModel);
			continue;
		}
		SceneGraph::C_SceneGraph* pSceneGraph = GetSceneGraph();
		WorldRender::C_ModelInstance* pModelInstance = new WorldRender::C_ModelInstance(m_pModelResource->GetRenderModelData(), pModel->GetIndex(), pSceneGraph);
		m_pModelInstances.PushBack(pModelInstance);

		SceneGraph::Insert(pSceneGraph, pModelInstance);

		pModelInstance->SetVisDetect(m_pVisDetect);

		for (U32 iElement = 0; iElement < pModel->GetElementCount(); iElement++)
		{
			const GameResource::C_ModelElement* pModelElement = pModel->GetElement(iElement);
			pModelInstance->SetElementMatrix(iElement, pModelElement->GetMatrix() * m_Matrix );

			const Gravity::C_ImportShape* pImportShape = pModelElement->GetPhysicsImportShape();

			Gravity::C_Body* pPhysicsBody = NULL;

			Collision::C_CollisionFilter* pFilter = &PhysicsConstants::kVehicleBodyCollisionFilter;
	
			if (pImportShape)
			{
				Gravity::C_ShapeDesc* pDesc = pImportShape->GetShapeDesc();
				pDesc->RemapShape();
				pDesc->SetRemapped();

				SharedPhysics::C_Deactivator deactivator;
				deactivator.m_ActivateThresholdSq = 0.005f;
				deactivator.m_SleepThresholdSq = 0.001f;

				pPhysicsBody = GetPhysicsScene()->CreateBody();
				
				pPhysicsBody->SetExternalConstants( GetPhysicsExternalConstants() );
				pPhysicsBody->Baumgarte( PhysicsConstants::kDefaultBaumgarte );
				pPhysicsBody->Slop( PhysicsConstants::kDefaultVehicleSlop );

				Gravity::C_BodyHelper bodyHelper( pDesc );

				Maths::HMat43 bodyMat = m_Matrix;

				bodyHelper.CreateBody( pImportShape->GetShapeDesc()->GetMass(), bodyMat.AW(), (Maths::HMat33)bodyMat, pImportShape->GetImportIneriaTensor(), pPhysicsBody, pFilter);
				pPhysicsBody->SetMass(1500.f);

				pPhysicsBody->SetDeactivator(deactivator);

				GetPhysicsScene()->AddBody(pPhysicsBody);
				
				pPhysicsBody->Activate();

				pPhysicsBody->ElementMatrix(pModelElement->GetMatrix() * m_Matrix);
			}	

			m_pVehicleModel->SetComponent(pModel, pModelElement, m_pModelResource, pModelInstance, pPhysicsBody, NULL);
		}
	}

	m_pVehicleModel->GetModelInput()->m_Wheels.Resize(m_pVehiclePhysics->GetNumWheels());

	m_pVehicleModel->GetComponentList().Initialise();

	m_pChaseCamera = new MGCam::C_VehicleCamera(GetPhysicsScene(), 0);
	
	if (m_pChaseCamera)
	{
		MGCam::System().GameFeedShow(m_pChaseCamera);
		//MGCam::System().DebugFeedShow(MGCam::E_DebugCam_GameFeed);
	}

	AddSubscriber( GameMessaging::E_GameMsg_Update );
}

/*
===============
NAME: CreateCollisionMesh

NOTE: Create vehicle physics body
===============
*/
void C_VehicleActor::CreateCollisionMesh(const GameResource::C_Model* p_pPhysicsModel)
{
	//RacingPhysics::Car& car = RacingPhysics::VehicleManager::Instance().AddVehicle<RacingPhysics::Car>();

	//m_pVehiclePhysics = (RacingPhysics::Vehicle*)&car;
	m_pVehiclePhysics = new RacingPhysics::Car();

	RacingPhysics::VehicleInitialiseInfo info;
	info.filename = "GT500";
	info.autoGears = true;

	//So can you only have one element then?
	for (U32 iElement = 0; iElement < p_pPhysicsModel->GetElementCount(); iElement++)
	//U32 iElement = 0;
	//if (iElement < p_pPhysicsModel->GetElementCount())
	{
		const GameResource::C_ModelElement* pElement = p_pPhysicsModel->GetElement(iElement);

		Gravity::C_ImportShape* pImportShape = const_cast<Gravity::C_ImportShape*>( pElement->GetPhysicsImportShape() );

		if (pImportShape && !m_pCollisionBody)
		{
			Collision::C_CollisionFilter* pFilter = &PhysicsConstants::kVehicleBodyCollisionFilter;
			Maths::HMat33 orientation;
			orientation.SetIdentity();

			Gravity::C_ShapeDesc* pDesc = pImportShape->GetShapeDesc();
			pDesc->RemapShape();
			
			m_pCollisionBody = GetPhysicsScene()->CreateBody();

			m_pCollisionBody->SetExternalConstants(GetPhysicsExternalConstants());
			m_pCollisionBody->Baumgarte(PhysicsConstants::kDefaultBaumgarte);
			m_pCollisionBody->Slop(PhysicsConstants::kDefaultVehicleSlop);

			const Maths::HMat33 inertiaTensor(pImportShape->GetImportIneriaTensor());
			
			Gravity::C_BodyHelper helper(pDesc);
			helper.CreateBody(pImportShape->GetShapeDesc()->GetMass(), Maths::HVec3(0.f, 0.f, 0.f), orientation, inertiaTensor, m_pCollisionBody, pFilter);
			m_pCollisionBody->SetMass(1500.0f);
	
			SharedPhysics::C_Deactivator deactivator;
			deactivator.m_ActivateThresholdSq = -1.f;
			deactivator.m_SleepThresholdSq = -1.f;
			deactivator.m_sSleepCountdown = 15;

			m_pCollisionBody->SetDeactivator(deactivator);
			GetPhysicsScene()->AddBody(m_pCollisionBody);
			m_pVehiclePhysics->SetGravityBody(m_pCollisionBody, Maths::HVec3(0.f, 0.42f, 0.44f));
		}
	}

	m_pVehiclePhysics->Initialise(info);
	RacingPhysics::Axis axis( (Maths::HMat44)m_Matrix );
	m_pVehiclePhysics->SetAxis( axis );
	m_pVehiclePhysics->CopyToGravity();
	m_pVehiclePhysics->SetGravityScene( GetPhysicsScene() );
}

/*
===============
NAME: Destroy

NOTE: Destroy
===============
*/
void C_VehicleActor::Destroy()
{
	for (U32 iModel = 0; iModel < m_pModelInstances.Size(); iModel++)
	{
		if (m_pModelInstances[iModel])
		{
			SceneGraph::Remove(GetSceneGraph(), m_pModelInstances[iModel]);
			delete m_pModelInstances[iModel];
		}
	}

	SAFE_DELETE_PTR(m_pVehiclePhysics);

	::Actors::C_VisibilityDetector::ReleaseVisDetect(m_pVisDetect);
	m_pVisDetect = NULL;

	m_pModelInstances.Clear();

	m_pVehicleModel->Destroy();

	MGCam::System().GameFeedRemove(m_pChaseCamera); // Make sure not being shown
	SAFE_DELETE_PTR(m_pChaseCamera);
}

/*
===============
NAME: CreateImpl

NOTE: CreateImpl
===============
*/
bool C_VehicleActor::CreateImpl()
{
	if (!C_GameActor::CreateImpl())
		return false;

	const C_VehicleActorCreate * pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());

	// load resources
	SendMessageToComponent( Components::E_ResourceIF_Loader, &ActorMsg::C_ResourceLoadMsg( 0 ) );

	//m_pGameWorld = pVehicleCreate->m_pGameWorld;

	//m_Matrix = pVehicleCreate->GetCreationMatrix();
	//m_ModelHandle.Load(pVehicleCreate->m_ModelFilename, this);

	////m_pVehicleModel = pVehicleCreate->m_pVehicleModel;
	////m_pVehiclePhysics = pVehicleCreate->m_pVehiclePhysics;

	return true;
}

/*
===============
NAME: InitialiseImpl

NOTE: InitialiseImpl
===============
*/
bool C_VehicleActor::InitialiseImpl()
{
	if (!C_GameActor::InitialiseImpl())
		return false;

	const C_VehicleActorCreate * pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());

	//m_pGameWorld = pVehicleCreate->m_pGameWorld;

	m_Matrix = pVehicleCreate->GetCreationMatrix();
	m_BirthPlace = m_Matrix;

	// create resource loader
	{
		// use single resource item component
		Component::C_ComponentCreatorFactoryHelper<Components::C_ResourceItemCreate> pCreator;
		pCreator->m_Desc.m_Name = pVehicleCreate->m_ModelFilename;
		pCreator->m_Desc.m_Type = GameResource::C_ModelResource::StaticGetResourceTypeId();

		// use resource list component
		//Component::C_ComponentCreatorFactoryHelper<Components::C_ResourceListCreate> pCreator;
		//pCreator->m_List.Reserve( 1 );
		//pCreator->AddItem( pCreate->m_ModelFilename, GameResource::C_ModelResource::StaticGetResourceTypeId() );

		if( InitialiseComponent( pCreator.Get() ) == NULL )
			return false;
	}

	//m_ModelHandle.Load(pVehicleCreate->m_ModelFilename, this);

	//m_pVehicleModel = pVehicleCreate->m_pVehicleModel;
	//m_pVehiclePhysics = pVehicleCreate->m_pVehiclePhysics;

	return true;
}

} //namespace Actors

} //namespace Game
