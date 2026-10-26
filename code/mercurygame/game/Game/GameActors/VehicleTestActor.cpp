#include "stdafx.h"

#include <Game/GameActors/VehicleTestActor.h>

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <misc_shared/messaging/GameMessaging.h>

#include <Resource/Resource.h>
#include <game_resource/ModelResource.h>
#include <game_resource/ModelData.h>

#include <world_render/RenderModelData.h>
#include <world_render/ModelInstance.h>
#include <world_render/MaterialParameter.h>
#include <world_render/Scene.h>
#include <world_render/LightSamplePointScene.h>

#include <SceneGraph/SceneGraph.h>

#include <Game/GameWorld/GameWorld.h>

#include <gravity/sdk/BodyHelper.h>
#include <gravity/importer/Importer.h>

#include "racingphysics/racingphysics.h"
#include "racingphysics/debug/draw.h"
#include "racingphysics/debug/colour.h"

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>

#include <Game/GameControls/GameControls.h>
#include <game/MGCam/CameraSystem.h>
#include <game/MGCam/CameraSwitcher.h>
#include <game/MGCam/VehicleCamera.h>

#include <framework/application/ApplicationBase.h>

#include <camera2/stock/DebugFeed.h>

#include <GameApp.h>
#include <Game/GameDef/PhysicsConstants.h>

namespace Game
{

namespace Actors
{

Debug::Bool g_DisplayVehicleModels("ActorDebug.Actors.VehicleTestActor.DisplayModelInfo", false);

C_VehicleTestActor::C_VehicleTestActor()
  : m_pModelResource(NULL)
  , m_pModelInstance(NULL)
  , m_pBody(NULL)
  , m_pCar(NULL)
  , m_pLightSampler(NULL)
  , m_pInteriorModelInstance(NULL)
{}

C_VehicleTestActor::~C_VehicleTestActor()
{
	if( m_pModelResource != NULL )
		DestroyModel();
	ExitCamera();
}

bool C_VehicleTestActor::CreateImpl()
{
	if (!C_GameActor::CreateImpl())
		return false;

	//const C_VehicleTestActorCreate * pCreate = (const C_VehicleTestActorCreate *)(GetCreationData().GetData());

	m_pModelResource = QueryInterface<Components::I_ResourceLoader>()->GetResource<GameResource::C_ModelResource>( 0 );
	
	if (m_pModelResource != NULL)
		CreateModel();

	return true;
}

bool C_VehicleTestActor::InitialiseImpl()
{
	if (!C_GameActor::InitialiseImpl())
		return false;

	const C_VehicleTestActorCreate * pCreate = (const C_VehicleTestActorCreate *)(GetCreationData().GetData());
	m_WorldMatrix = pCreate->GetCreationMatrix();

	// create resource loader
	{
		// use single resource item component
		Component::C_ComponentCreatorFactoryHelper<Components::C_ResourceItemCreate> pCreator;
		pCreator->m_Desc.m_Name = pCreate->m_ModelFilename;
		pCreator->m_Desc.m_Type = GameResource::C_ModelResource::StaticGetResourceTypeId();
		pCreator->m_Desc.SetPrime( true );

		// use resource list component
		//Component::C_ComponentCreatorFactoryHelper<Components::C_ResourceListCreate> pCreator;
		//pCreator->m_List.Reserve( 1 );
		//pCreator->AddItem( pCreate->m_ModelFilename, GameResource::C_ModelResource::StaticGetResourceTypeId(), Components::E_ResourceFlag_Prime );

		if( InitialiseComponent( pCreator.Get() ) == NULL )
			return false;
	}

	return true;
}

void C_VehicleTestActor::CreateGravityCar(RacingPhysics::Car& car, Gravity::C_ImportShape* pImportShape, const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF cOfMOffset)
{
	Gravity::C_Scene* pScene = GetPhysicsScene();
	m_pBody = pScene->CreateBody();

	car.SetGravityScene( pScene );
	car.SetWheelGroundFilter( &PhysicsConstants::kVehicleWheelCollisionFilter );
	car.SetWheelOverlayFilter( &PhysicsConstants::kVehicleOverlayCollisionFilter );

	Collision::C_CollisionFilter filter = PhysicsConstants::kVehicleBodyCollisionFilter;
	static U32 groupId = 1;
	filter.m_GroupId = groupId++;
	//Kier - hack because we don't have collision flags yet, only mask!
	filter.m_GroupFlags = filter.m_CollisionMask;
	filter.m_GroupMask = filter.m_CollisionMask;
	Maths::HMat33 orientation;
	orientation.SetIdentity();

	Gravity::C_ShapeDesc* pDesc=pImportShape->GetShapeDesc();
	
	/*Maths::RotY rotY(Maths::Constants<Maths::HF32>::Pi());
	Maths::HMat33 staticRotation(rotY);*/
	//pDesc->ApplyStaticRotation(staticRotation);
	pDesc->RemapShape();
	pDesc->SetRemapped();
	
	m_pBody->SetExternalConstants( GetPhysicsExternalConstants() );
	m_pBody->Baumgarte( PhysicsConstants::kDefaultBaumgarte );
	m_pBody->Slop( PhysicsConstants::kDefaultVehicleSlop );
	
	Maths::HMat33 identity;
	identity.SetIdentity();

	Gravity::C_BodyHelper helper(pDesc);

//	const Maths::HMat33 transRot(staticRotation.Transposed());
//	const Maths::HMat33 inertiaTensor( transRot * pImportShape->GetImportIneriaTensor() * staticRotation );

	helper.CreateBody(pImportShape->GetShapeDesc()->GetMass(), Maths::HVec3(0.f, 0.f, 0.f), orientation, pImportShape->GetImportIneriaTensor(), m_pBody, &filter);
	m_pBody->SetMass(mass);
	
	SharedPhysics::C_Deactivator deactivator;
	deactivator.m_ActivateThresholdSq = -1.f;
	deactivator.m_SleepThresholdSq = -1.f;
	deactivator.m_sSleepCountdown = 15;

	m_pBody->SetDeactivator(deactivator);
	pScene->AddBody(m_pBody);
	car.SetGravityBody(m_pBody, Maths::HVec3(0.f, -car.Wheel(0).Radius(), 0.f));
}

void C_VehicleTestActor::CreateModel()
{
	m_pLightSampler = new WorldRender::C_LightSampler;
	m_pLightSampler->Initialise( &GetWorldRenderScene()->GetLightSamplePointScene() );

	const GameResource::C_Model* pInteriorModel = m_pModelResource->GetModelData()->GetModel("m_Interior");

	//const GameResource::C_Model* pModel = m_pModelResource->GetModelData()->GetModel("M_woodBox_UNDAMAGED");
	S32 modelIndex = 0;
	if (!pInteriorModel)
	{
		// not an amax-format car
		if(m_pModelResource->GetModelData()->GetModelCount() > 1)
			modelIndex = 1;
	}

	const GameResource::C_Model* pModel = m_pModelResource->GetModelData()->GetModel(modelIndex);

	m_pModelElement = pModel->GetElement(0);

	if(m_pModelInstance == NULL)
	{
		m_pModelInstance = new WorldRender::C_ModelInstance( m_pModelResource->GetRenderModelData(), pModel->GetIndex(), GetSceneGraph() );
		
		// this sets all elements to same matrix
		m_pModelInstance->SetElementMatrix(-1, m_pModelElement->GetMatrix() * m_WorldMatrix );
		m_pModelInstance->SetWorldSphere(Maths::C_Sphere(m_WorldMatrix.Translation(), 10.0f));
		
		SceneGraph::Insert(GetSceneGraph(), m_pModelInstance);

		m_pModelInstance->SetElementMatrix(-1, m_pModelElement->GetMatrix() * m_WorldMatrix );
		m_pModelInstance->SetWorldSphere(Maths::C_Sphere(m_WorldMatrix.Translation(), 10.0f));
	}

	if(pInteriorModel)
	{
		m_pInteriorModelInstance = new WorldRender::C_ModelInstance( m_pModelResource->GetRenderModelData(), pInteriorModel->GetIndex(), GetSceneGraph() );

		// this sets all elements to same matrix
		m_pInteriorModelInstance->SetElementMatrix(-1, m_pModelElement->GetMatrix() * m_WorldMatrix );
		m_pInteriorModelInstance->SetWorldSphere(Maths::C_Sphere(m_WorldMatrix.Translation(), 10.0f));
		
		SceneGraph::Insert(GetSceneGraph(), m_pInteriorModelInstance);

		m_pInteriorModelInstance->SetElementMatrix(-1, m_pModelElement->GetMatrix() * m_WorldMatrix );
		m_pInteriorModelInstance->SetWorldSphere(Maths::C_Sphere(m_WorldMatrix.Translation(), 10.0f));
	}

	const S32 wheelCount = 4;
	m_pWheelInstanceList.Resize(wheelCount);
	for(S32 wheelNo=0; wheelNo<wheelCount;wheelNo++)
	{
		const GameResource::C_Model* pWheelModel = NULL;
		
		switch(wheelNo)
		{
			case 0:
				pWheelModel = m_pModelResource->GetModelData()->GetModel("M_Wheel_FL");
				if (!pWheelModel)
					pWheelModel = m_pModelResource->GetModelData()->GetModel("m_Wheel_FrontLeft");
				break;
			case 1:
				pWheelModel = m_pModelResource->GetModelData()->GetModel("M_Wheel_FR");
				if (!pWheelModel)
					pWheelModel = m_pModelResource->GetModelData()->GetModel("m_Wheel_FrontRight");
				break;	
			case 2:
				pWheelModel = m_pModelResource->GetModelData()->GetModel("M_Wheel_RL");
				if (!pWheelModel)
					pWheelModel = m_pModelResource->GetModelData()->GetModel("m_Wheel_BackLeft");
				break;
			case 3:
				pWheelModel = m_pModelResource->GetModelData()->GetModel("M_Wheel_RR");
				if (!pWheelModel)
					pWheelModel = m_pModelResource->GetModelData()->GetModel("m_Wheel_BackRight");
				break;
		}

		if( pWheelModel )
		{
			const GameResource::C_ModelElement* pWheelElement = pWheelModel->GetElement(0);
			m_pWheelInstanceList[wheelNo] = new WorldRender::C_ModelInstance( m_pModelResource->GetRenderModelData(), pWheelModel->GetIndex(), GetSceneGraph());
			
			// this sets all elements to same matrix
			m_pWheelInstanceList[wheelNo]->SetElementMatrix(-1, pWheelElement->GetMatrix() * m_WorldMatrix );
			m_pWheelInstanceList[wheelNo]->SetWorldSphere(Maths::C_Sphere(m_WorldMatrix.Translation(), 10.0f));
		
			SceneGraph::Insert(GetSceneGraph(), m_pWheelInstanceList[wheelNo]);

		}
	}


	{

		const GameResource::C_ModelElement* pElement = pModel->GetElement(0);
		Gravity::C_ImportShape* pImportShape = const_cast<Gravity::C_ImportShape*>( pElement->GetPhysicsImportShape() );

		//RacingPhysics::Car& car = RacingPhysics::VehicleManager::Instance().AddVehicle<RacingPhysics::Car>();
		m_pCar = new RacingPhysics::Car();
		RacingPhysics::VehicleInitialiseInfo info;
		info.filename = "DefaultCar";
		info.autoGears = true;

		m_pCar->Initialise(info);
		CreateGravityCar(*m_pCar, pImportShape, 1200.f, Maths::HVec3(0.f, 0.45f, 0.1f));		
		RacingPhysics::Axis axis( (Maths::HMat44)m_WorldMatrix );
		m_pCar->SetAxis( axis );
		m_pCar->CopyToGravity();	
	}

	InitCamera();

	AddSubscriber( GameMessaging::E_GameMsg_Update );
}

void C_VehicleTestActor::DestroyModel()
{
	SAFE_DELETE_PTR( m_pLightSampler );

	SAFE_DELETE_PTR( m_pCar );
	if( m_pModelInstance )
	{
		SceneGraph::Remove(GetSceneGraph(), m_pModelInstance);
		delete m_pModelInstance;
	}
	if ( m_pInteriorModelInstance )
	{
		SceneGraph::Remove(GetSceneGraph(), m_pInteriorModelInstance);
		delete m_pInteriorModelInstance;
	}
}

void C_VehicleTestActor::InitCamera()
{
	// Camera2 stuff:
	Gravity::C_Scene* scene = GetPhysicsScene();
	m_pChaseCamera = new MGCam::C_VehicleCamera(scene, m_pBody->GetShape()->m_Filter.m_GroupId);
	if (m_pChaseCamera == 0)
		return;
	
	MGCam::System().GameFeedShow(m_pChaseCamera);
	MGCam::Switcher().SetType(MGCam::E_DebugCam_GameFeed);
}

void C_VehicleTestActor::ExitCamera()
{
	MGCam::System().GameFeedRemove(m_pChaseCamera); // Make sure not being shown
	SAFE_DELETE_PTR(m_pChaseCamera);
}

#ifndef CFG_GOLD
void C_VehicleTestActor::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if (!m_pCar)
		return;

	m_pCar->DisplayDebug();
}
#endif

bool C_VehicleTestActor::OnMessage(const Messaging::C_MessageBase *pMessage)
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
				if (m_pBody)
					m_WorldMatrix = m_pBody->ElementMatrix();

				{
					RacingPhysics::CarControls& controls = m_pCar->Controls();

					controls.Clear();

					if (MGCam::System().DebugFeed().PadUsed() != 0)
					{
						controls.throttle = Input::GetGameInput(0)->GetButtonValue( Game::E_Button_Throttle );
						controls.steer = Input::GetGameInput(0)->GetAxisSignedValue( Game::E_Axis_Steering );
						controls.brake = Input::GetGameInput(0)->GetButtonValue( Game::E_Button_Brake );
						controls.handbrake = Input::GetGameInput(0)->GetButtonValue( Game::E_Button_Handbrake );
						controls.gearUp = Input::GetGameInput(0)->GetButtonPressedDebounced( Game::E_Button_GearUp );
						controls.gearDown = Input::GetGameInput(0)->GetButtonPressedDebounced( Game::E_Button_GearDown );
					}					
				}

				m_pModelInstance->SetWorldSphere(Maths::C_Sphere(m_WorldMatrix.Translation(), 2.0f));
				m_pModelInstance->SetElementMatrix(-1, m_WorldMatrix);

				if (m_pInteriorModelInstance)
				{
					m_pInteriorModelInstance->SetWorldSphere(Maths::C_Sphere(m_WorldMatrix.Translation(), 2.0f));
					m_pInteriorModelInstance->SetElementMatrix(-1, m_WorldMatrix);
				}

				// update cam
				{
					// Chase camera:
					Cam2::C_CamObj object;
					const RacingPhysics::Vehicle& vehicle = *m_pCar;//RacingPhysics::VehicleManager::Instance().GetVehicle(0);
					
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

				// set rendered wheel positions
				{
					const S32 wheelCount = m_pWheelInstanceList.Size();
					for(S32 wheelNo=0; wheelNo<wheelCount; wheelNo++)
					{
						const Maths::HMat43 mWheel = GetWheelMatrix( wheelNo );

						if (m_pWheelInstanceList[wheelNo])
						{
							m_pWheelInstanceList[wheelNo]->SetElementMatrix(-1, mWheel );
							m_pWheelInstanceList[wheelNo]->SetWorldSphere(Maths::C_Sphere(mWheel.Translation(), 2.0f));
						}
					}

				}

				if( g_DisplayVehicleModels )
				{
					Maths::HMat43 mElement = m_pModelElement->GetMatrix();
					Maths::HMat43 mModelDebug = mElement.InverseRotTrans() * m_WorldMatrix;
					m_pModelResource->GetModelData()->DebugRender(DbUtil::DefaultDebugRenderSys()->Deferred(), mModelDebug);
				}

				if(m_pLightSampler)
				{
					m_pLightSampler->SetPosition( m_WorldMatrix.AW() );
					m_pLightSampler->Update();

					WorldRender::MaterialParameter handle = m_pModelInstance->GetRenderModel()->GetParameter("g_SHNode");
					if (handle.index != -1)
					{
						m_pModelInstance->SetParameter(handle, &m_pLightSampler->GetLightSampleData().m_SampleData[0]);	
					}

					const S32 wheelCount = m_pWheelInstanceList.Size();
					for(S32 wheelNo=0; wheelNo<wheelCount; wheelNo++)
					{
						if (m_pWheelInstanceList[wheelNo])
						{
							WorldRender::MaterialParameter handle = m_pWheelInstanceList[wheelNo]->GetRenderModel()->GetParameter("g_SHNode");
							if (handle.index != -1)
							{
								m_pWheelInstanceList[wheelNo]->SetParameter(handle, &m_pLightSampler->GetLightSampleData().m_SampleData[0]);	
							}
						}
					}

				}

				/*
				for (int i = 0; i < RacingPhysics::VehicleManager::Instance().GetNumVehicles(); i++)
				{
					if (i % 2 == 1) continue;

					if (RacingPhysics::Motorbike* p = RacingPhysics::Motorbike::GetMotorbikePointer(&RacingPhysics::VehicleManager::Instance().GetVehicle(i)))
					{
						RacingPhysics::MotorbikeControls& controls = p->Controls();
						controls.Clear();
						controls.throttle = m_controller.GetAnalogue(GamePad::Button_RightTrigger);
						controls.fullBrakes = m_controller.GetAnalogue(GamePad::Button_LeftTrigger);
						controls.steer = m_controller.GetAxis(GamePad::Axis_LX);
						controls.lean = m_controller.GetAxis(GamePad::Axis_LY);
						controls.rearBrake = m_controller.GetDigitalHeld(GamePad::Button_A);
						
						if (m_debugUsesInput)
							controls.Clear();
					}

					if (RacingPhysics::Car* p = RacingPhysics::Car::GetCarPointer(&RacingPhysics::VehicleManager::Instance().GetVehicle(i)))
					{
						RacingPhysics::CarControls& controls = p->Controls();
						controls.Clear();
						controls.throttle = m_controller.GetAnalogue(GamePad::Button_RightTrigger);
						controls.brake = m_controller.GetAnalogue(GamePad::Button_LeftTrigger);
						controls.steer = m_controller.GetAxis(GamePad::Axis_LX);
						controls.handbrake = m_controller.GetDigitalHeld(GamePad::Button_A);
						controls.gearUp = m_controller.GetDigitalPressed(GamePad::Button_B);
						controls.gearDown = m_controller.GetDigitalPressed(GamePad::Button_X);

						if (m_controller.GetDigitalPressed(GamePad::Button_RightShoulder))
							p->SetVelocity(p->GetVelocity() + Maths::Constants<RacingPhysics::Normal3R>::Y() * 10.f);

						if (m_controller.GetDigitalHeld(GamePad::Button_Back)
							&& m_controller.GetDigitalPressed(GamePad::Button_A))
						{
							Maths::HVec3 c = m_DebugFeed.Position();
							p->SetPosition(RacingPhysics::Point3R(c.X(), c.Y(), c.Z()));
							RacingPhysics::Axis axis;
							axis.SetPosition(RacingPhysics::Point3R(c.X(), c.Y(), c.Z()));
							p->SetAxis(axis);
							p->SetVelocity(Maths::Constants<RacingPhysics::Vector3R>::Null());
							p->SetAngularVelocity(Maths::Constants<RacingPhysics::Vector3R>::Null());
							p->CopyToGravity();
						}

						p->PerformanceModifiers().peakTorqueScale = g_peakTorqueScale;
						p->PerformanceModifiers().forwardGripScale = g_forwardGripScale;
						p->PerformanceModifiers().backwardGripScale = g_backwardGripScale;
						p->PerformanceModifiers().rpmScale = g_rpmScale;
						p->PerformanceModifiers().transverseGripScale = g_transverseGripScale;

						if (m_debugUsesInput)
							controls.Clear();

						//p->CopyToGravity();
					}
				}
				*/
			}
				
			return true;
		break;
	}

	return C_GameActor::OnMessage(pMessage);
}

const Maths::HMat43 C_VehicleTestActor::GetWheelMatrix( const S32 wheelNo ) const
{
	const RacingPhysics::CarWheel& wheel = m_pCar->Wheel( wheelNo );
	const RacingPhysics::Point3R vWheel = wheel.GetWheelPosition();
	const RacingPhysics::RotationMatrix mWheelRotation = wheel.GetRotor().GetRotationMatrix();

	const Maths::HMat43 mWheel(mWheelRotation.Matrix3(), vWheel);

	return mWheel;
}


class C_ModelTypeCar : public GameResource::I_ModelType
{
public:
	C_ModelTypeCar(){}

	void OnLoad(GameResource::C_ModelResource* p_pModelResource)
	{
		if (!p_pModelResource)
			return;

		GameResource::C_ModelData *pModelDesc = p_pModelResource->GetModelData();
		WorldRender::C_RenderModelData *pGRModel = p_pModelResource->GetRenderModelData();

		// for each model inside the file
		for (int iModel = 0; iModel < pModelDesc->GetModelCount(); iModel++)
		{
			GameResource::C_Model *pModel = pModelDesc->GetModel(iModel);
			WorldRender::C_Model *pWRModel = &p_pModelResource->GetRenderModelData()->GetModel(iModel);

			Util::C_String modelName = pModel->GetName();
			modelName.ConvertToLower();

			if (modelName == "m_shadowcaster")
			{
				pWRModel->ExposeParameter("g_WorldMatrix", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 16);
				pWRModel->ExposeParameter("g_ShadowPlaneSpaceMatrix", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 16);
				pWRModel->ExposeParameter("g_ShadowPlane", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
				pWRModel->ExposeParameter("g_LightPosition", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
				pWRModel->ExposeParameter("g_ShadowAlpha", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
			}
			else
			{
				/*pWRModel->ExposeParameter("g_sPaintMaterialTexture", RenderingData::MATERIAL_ELEMENT_TYPE_TEXTURE_PTR, 1);
				pWRModel->ExposeParameter("g_sVehicleEnvironmentTexture", RenderingData::MATERIAL_ELEMENT_TYPE_TEXTURE_PTR, 1);
				pWRModel->ExposeParameter("g_sVehicleDamageVolumeTexture", RenderingData::MATERIAL_ELEMENT_TYPE_TEXTURE_PTR, 1);*/
				pWRModel->ExposeParameter("g_sPaintMaterialTexture", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 1);
				pWRModel->ExposeParameter("g_sVehicleDamageVolumeTexture", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 1);
				pWRModel->ExposeParameter("g_sVehicleDamageInfoTexture", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 1);
				pWRModel->ExposeParameter("g_sVehicleDirtLayerTexture", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 1);
				
				pWRModel->ExposeParameter("g_VehicleBoundsInfo", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
				pWRModel->ExposeParameter("g_mVehicleMatrix", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 16);
				pWRModel->ExposeParameter("g_mVehicleInverseMatrix", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 16);
				pWRModel->ExposeParameter("g_SHNode", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 16);
				pWRModel->ExposeParameter("g_vDebugDamageSettings", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
				bool exposeControlPoints = false;
				if (modelName.ISearch("m_bonnet") != -1 ||
					modelName.ISearch("m_boot") != -1 ||
					modelName.ISearch("m_bumper") != -1 ||
					modelName.ISearch("m_wing") != -1)
				{
					pWRModel->ExposeParameter("g_vControlPoints", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 12);
					exposeControlPoints = true;
				}
				pWRModel->ExposeParameter("g_vDirtBaseLayerSettings", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
				pWRModel->ExposeParameter("g_vDirtDetailLayerSettings", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);

				// for each element inside this model
				for (int iElement=0; iElement<pModel->GetElementCount(); iElement++)
				{
					GameResource::C_ModelElement *pElement = pModel->GetElement(iElement);

					Util::C_String elementName = pElement->GetName();
					elementName.ConvertToLower();
					
					if (exposeControlPoints ||
						elementName.ISearch("e_bonnet") != -1 ||
						elementName.ISearch("e_boot") != -1 ||
						elementName.ISearch("e_bumper") != -1 ||
						elementName.ISearch("e_wing") != -1)
					{
						pWRModel->ExposeParameter(pElement->GetIndex(), "g_vControlPoints", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 12);
					}
					//Tyre
					else if (elementName.Search("e_tyre") != -1)
					{
						pWRModel->ExposeParameter(pElement->GetIndex(), "g_vRenderSettings", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
					}
					//Glass
					else if (elementName.Search("e_window") != -1 || elementName.Search("e_headlightcover") != -1)
					{
						pWRModel->ExposeParameter(pElement->GetIndex(), "g_vRenderSettings", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
						pWRModel->ExposeParameter(pElement->GetIndex(), "g_vRenderColour", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
						pWRModel->ExposeParameter(pElement->GetIndex(), "g_vDamageSettings", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
						//if (elementName.Search("e_window") != -1)
						//{
						//	pWRModel->ExposeParameter(pElement->GetIndex(), "g_vScreenSettingsVS", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
						//	pWRModel->ExposeParameter(pElement->GetIndex(), "g_vScreenSettingsPS", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
						//	pWRModel->ExposeParameter(pElement->GetIndex(), "g_tRimLight", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 1);						
						//}
					}
					//Alloy
					else if (elementName.Search("e_alloyrim") != -1)
					{
						pWRModel->ExposeParameter(pElement->GetIndex(), "g_vRenderSettings", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
						pWRModel->ExposeParameter(pElement->GetIndex(), "g_vRenderColour", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
					}
					//Light
					else if (elementName.Search("e_headlight") != -1 || elementName.Search("e_rearlight") != -1 ||
							 elementName.Search("e_foglight") != -1 || elementName.Search("e_brakelight") != -1 ||
							 elementName.Search("e_reverselight") != -1 || elementName.Search("e_runninglight") != -1 ||
							 elementName.Search("e_signallight") != -1 || elementName.Search("e_rearandbrakelight") != -1)
					{
						pWRModel->ExposeParameter(pElement->GetIndex(), "g_vRenderColour", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
						pWRModel->ExposeParameter(pElement->GetIndex(), "g_vDamageSettings", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
					}
					//pWRModel->ExposeParameter(pElement->GetIndex(), "g_vRenderSettings", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
					//pWRModel->ExposeParameter(pElement->GetIndex(), "g_vRenderColour", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
					//pWRModel->ExposeParameter(pElement->GetIndex(), "g_vDamageSettings", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
				}
			}
		}
	}
};

void RegisterCarModelCallback()
{
	GameResource::g_ModelTypeManager.RegisterModelType("car", new C_ModelTypeCar());
}

} // namespace Actors


} // namespace Game
