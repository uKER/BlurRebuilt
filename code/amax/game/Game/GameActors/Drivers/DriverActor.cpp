#include "stdafx.h"
#include <Game/GameActors/Drivers/DriverActor.h>
#include <Game/GameActors/Drivers/XtAnimationTree.h>
#include <Game/GameWorld/ViewType.h>
#include <Game/GameWorld/RenderPass.h>
#include <Game/GameUtil/ResourceBag.h>
#include <Game/Game.h>
#include <Game/Messaging/GameMessages.h>
#include <GameLogic/ActorStudioActors/CutSceneMessages.h>
#include <GameLogic/GameData/GameData.h>
#include <GameLogic/Messaging/GameLogicMessages.h>
#include <GameApp.h>
#include <game_resource/ModelResource.h>
#include <game_resource/ModelData.h>
#include <resource_utils/GameModel.h>
#include <resource_utils/ResourceBagUtils.h>
#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>
#include <shared_actors/Utility/VisibilityDetection.h>
#include <world_render/ModelInstance.h>
#include <world_render/Scene.h>
#include <animated_model/AnimatedModelInstance.h>
#include <animated_model/TaskManager.h>
#include <animation/common/StringId.h>
#include <animation/common/JointTransform.inl>
#include <animation/deferred/CommandList.h>
#include <animation/deferred/CommandProcessor.h>
#include <animation/pose/PoseFormat.h>
#include <animation/pose/Pose.h>
#include <animation/resource/CycleResource.h>
#include <animation/resource/SkeletonResource.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/task/EvaluationState.h>
#include <animation/util/DebugRender.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Vehicle/Input.h>

namespace Game
{
DECLARE_DEBUG_BOOL(gDebugDriverForceEnableSteeringIk, "ActorDebug.Actors.Driver.SteeringIK.ForceEnable", true);
DECLARE_DEBUG_VECTOR3(gReferenceSteeringPos, "ActorDebug.Actors.Driver.SteeringIK.ReferencePos", -0.407f, 0.507f, 0.247f)
DECLARE_DEBUG_FLOAT(gReferenceSteeringTilt, "ActorDebug.Actors.Driver.SteeringIK.ReferenceTilt", 19.8f, Debug::Range(-90.f, 90.f))
DECLARE_DEBUG_FLOAT(gReferenceSteeringRadius, "ActorDebug.Actors.Driver.SteeringIK.ReferenceRadius", 0.180f, Debug::Range(0.f, 0.4f))
DECLARE_DEBUG_FLOAT(gDriverIkWheelToHandsDist, "ActorDebug.Actors.Driver.SteeringIK.WheelToHandsDistance", 0.05f, Debug::Range(0.f, 0.2f));
DECLARE_DEBUG_FLOAT(gDriverIkFalloffDist, "ActorDebug.Actors.Driver.SteeringIK.FalloffDistance", 0.15f, Debug::Range(0.f, 0.5f));
DECLARE_DEBUG_BOOL(gDebugUseTestAnimationTree, "ActorDebug.Actors.Driver.UseTestAnimationTree", false, Debug::Persist());


//
// C_DriverActorCreate
//
C_DriverActorCreate::C_DriverActorCreate()
{
}

const CHAR8* C_DriverActorCreate::GetDriverModelFilename() const
{
	if (characterDef)
	{
		return characterDef->model.Text();
	}
	else
	{
		return "models/characters/firstplay_drivers/driver_m_01.model";
	}
}

const CHAR8* C_DriverActorCreate::GetSkeletonFilename() const
{
	if (characterDef)
	{
		return characterDef->skeleton.Text();
	}
	else
	{
		return "animations/CrowdMaleTest.skeleton";
	}
}

bool C_DriverActorCreate::HasSeatbeltModel() const
{
	if (!characterDef)
	{
		return false;
	}
	if (!characterDef->seatbeltModel)
	{
		return false;
	}
	return true;
}

Util::C_String C_DriverActorCreate::GetSeatbeltModelFilename() const
{
	Util::C_String fn;
	if (characterDef)
	{
		if (characterDef->seatbeltModel)
		{
			fn = characterDef->seatbeltModel->GetFileName();
		}
	}
	return fn;
}

bool C_DriverActorCreate::IsRightHandDrive() const
{
	if (vehicleDef)
	{
		return vehicleDef->rightHandDrive;
	}
	return false;
}

Maths::HVec3 C_DriverActorCreate::GetOffset() const
{
	return vehicleDef ? Load(vehicleDef->driverOffset) : Maths::HVec3();
}

F32 C_DriverActorCreate::GetCabinFloorOffset() const
{
	return vehicleDef ? vehicleDef->cabinFloorOffset : 0.f;
}

F32 C_DriverActorCreate::GetCabinFootwellOffset() const
{
	return vehicleDef ? vehicleDef->cabinFootwellOffset : 0.f;
}

bool C_DriverActorCreate::IsSteeringWheelIkEnabled() const
{
	return vehicleDef ? vehicleDef->enableSteeringWheelIk : false;
}

Maths::Point3 C_DriverActorCreate::GetSteeringWheelPos() const
{
	if (vehicleDef)
	{
		if (IsSteeringWheelIkEnabled())
		{
			return Load(vehicleDef->steeringWheelPosition);
		}
	}
	Maths::Point3 pos = Maths::MakePoint3(gReferenceSteeringPos->vx, gReferenceSteeringPos->vy, gReferenceSteeringPos->vz);
	
	if (IsRightHandDrive())
	{
		pos = Maths::MakePoint3(-Maths::X(pos) + Maths::X(Maths::Vector3(GetOffset())), Maths::Y(pos), Maths::Z(pos));
	}
	return pos;
}

F32 C_DriverActorCreate::GetSteeringWheelTilt() const
{
	if (vehicleDef)
	{
		if (IsSteeringWheelIkEnabled())
		{
			return vehicleDef->steeringWheelTilt;
		}
	}
	return gReferenceSteeringTilt;
}

F32 C_DriverActorCreate::GetSteeringWheelRadius() const
{
	if (vehicleDef)
	{
		if (IsSteeringWheelIkEnabled())
		{
			return vehicleDef->steeringWheelRadius;
		}
	}
	return gReferenceSteeringRadius;
}

U32 C_DriverActorCreate::GetDrivingStyle() const
{
	if (vehicleDef)
	{
		if (vehicleDef->forceNeutralDrivingStyle)
		{
			return E_AmaxCharacterDrivingStyle_Neutral;
		}
	}

	if (characterDef)
	{
		return characterDef->drivingStyle;
	}
	return E_AmaxCharacterDrivingStyle_Neutral;
}

bool C_DriverActorCreate::CannotRestArmOnDoor() const
{
	return vehicleDef ? vehicleDef->cannotRestArmOnDoor : false;
}

bool C_DriverActorCreate::CannotPutArmThroughWindow() const
{
	return vehicleDef ? vehicleDef->cannotPutArmThroughWindow : false;
}


DECLARE_DEBUG_FLOAT(gDriverOvertakeRequestLifetime, "ActorDebug.Actors.Driver.OvertakeRequestLifetime", 0.5f)
DECLARE_DEBUG_FLOAT(gDriverFrustrationRequestLifetime, "ActorDebug.Actors.Driver.FrustrationRequestLifetime", 2.f)
DECLARE_DEBUG_FLOAT(gDriverChangeGearRequestLifetime, "ActorDebug.Actors.Driver.ChangeGearRequestLifetime", 1.5f)
DECLARE_DEBUG_FLOAT(gDriverHandbrakeRequestLifetime, "ActorDebug.Actors.Driver.HandbrakeRequestLifetime", 4.f)
DECLARE_DEBUG_FLOAT(gDriverCutsceneAnimRequestLifetime, "ActorDebug.Actors.Driver.CutsceneAnimRequestLifetime", 1.f)
DECLARE_DEBUG_FLOAT(gDriverCollisionRequestLifetime, "ActorDebug.Actors.Driver.CollisionRequestLifetime", 1.5f)
DECLARE_DEBUG_FLOAT(gDebugDriverFarCullDistance, "ActorDebug.Actors.Driver.FarCullDistance", 29.f, Debug::Range(0.f, 100.f));

//
// C_DriverActor
//

C_DriverActor::C_DriverActor():
	m_skeletonResource(NULL),
	m_pDriverModel(NULL),
	m_visibilityDetectionFlags(0xffffffff),
	m_pVehicleLightSampler(NULL),
	m_pVehicleEnvMapTexture(NULL),
	m_pVehicleModelInput(NULL),
	m_headJointIndex(-1),
	m_currentGear(-1),
	m_isVisible(true),
	m_visibilityChanged(true),
	m_onScreen(true),
	m_created(false)
{
	m_evaluationState = Animation::C_EvaluationState::New(64);
	
	Reset();
}

C_DriverActor::~C_DriverActor()
{
	Destroy();
}

Maths::Mat43 MakeSteeringWheelMatrix(const Maths::Point3& pos, F32 tiltDegrees)
{
	Maths::Mat33 rotationMatrix = Maths::MakeMat33_RotY(Maths::MakeVF32(Maths::DegToRad(180.f)));
	rotationMatrix = rotationMatrix * Maths::MakeMat33_RotX(Maths::MakeVF32(Maths::DegToRad(tiltDegrees)));
	return Maths::MakeMat43(rotationMatrix, pos);
}

// Begins loading of driver resources (animations, models etc). Once all resources are loaded, Create() is called
// to complete the creation process.
bool C_DriverActor::Prime(const C_DriverActorCreate& creationParams, SceneGraph::C_SceneGraph* pSceneGraph)
{
	m_creationParams = creationParams;
	m_pSceneGraph = pSceneGraph;
	m_created = false;

	m_vehicleIndex = m_creationParams.vehicleIndex;
	m_vehicleIsRightHandDrive = m_creationParams.IsRightHandDrive();

	m_driverOffset = m_creationParams.GetOffset();

	m_matrix.SetIdentity();
	m_matrix.Translation(m_driverOffset);

	const C_DriverActorCreate* createParams = GetCreationParams();

	// Create resource bag.
	m_resourceBagHandle = C_ResourceBag::Create();
	m_resourceBagHandle->AddListener(this);

	m_resourceBagHandle->LockListeners();

	// Request loading of driver model, seatbelt and skeleton resource.
	m_modelParts[E_ModelPart_Driver].AddToResourceBag(*m_resourceBagHandle, createParams->GetDriverModelFilename());
	if (createParams->HasSeatbeltModel())
	{
		m_modelParts[E_ModelPart_Seatbelt].AddToResourceBag(*m_resourceBagHandle, createParams->GetSeatbeltModelFilename());
	}

	m_skeletonTag = MakeResourceBagTag(createParams->GetSkeletonFilename());
	m_resourceBagHandle->AddResource(m_skeletonTag, createParams->GetSkeletonFilename(), Animation::C_SkeletonResource::StaticGetResourceTypeId());

	// Initialise randomisation.
	// Mix in pickup position seed as it's time-based and should be consistent across network games and replays.
	U32 levelSeed = C_Game::GetInstance()->GetLevelState().m_RandomSeed;
	if (levelSeed == 0)
	{
		levelSeed = static_cast<U32>(Time::GetPerformanceCounter());
	}
	U32 hashId = Btl::Hash(GetCreationParams()->GetDriverModelFilename());
	U32 vehicleSeed = levelSeed ^ hashId;
	if (!vehicleSeed) vehicleSeed = 0x91237123;
	m_animTree.SetRandomSeed(vehicleSeed);

	// Create animation tree and add its animation resources to the resource bag.
	CreateAnimTree(*m_resourceBagHandle);

	m_resourceBagHandle->UnlockListeners();

	// This never loads if the resource system is paused and we hit this function
	// This should not be needed as the main thread now waits for all pending resources to be loaded (AmaxWorldLayer.cpp - WaitForAllResourceData)
	//while (m_resourceBagHandle->IsWaitingForLoad())
	//{
	//	Resource::C_Manager::GetInstance()->PartialUpdate();
	//}

	return true;
}

void C_DriverActor::OnResourceBagLoaded(const C_ResourceBag& bag)
{
	// Notify animation tree that resource bag has been loaded and its resources can be accessed.
	m_animTree.OnResourceBagLoaded();

	// Finish creation.
	Create();
}

// Finishes off initialisation of the driver actor once all resources are loaded.
bool C_DriverActor::Create()
{
	if (m_created)
	{
		// Avoid constructing models twice, for example if new animations are being loaded due to changes in Sorcery.
		return true;
	}

	// Gather resource pointers.
	for (U32 i = 0; i < E_ModelPart_Count; ++i)
	{
		m_modelParts[i].Create(*m_resourceBagHandle);
	}
	m_skeletonResource = m_resourceBagHandle->GetResource<Animation::C_SkeletonResource>(m_skeletonTag);
	if(!m_skeletonResource) return false;

	const Animation::C_Skeleton* skeleton = m_skeletonResource->GetSkeleton();
	if (skeleton != NULL)
	{
		//
		// Find indices of interesting joints and set up IK chains.
		// 
	
		m_headJointIndex = skeleton->FindJointByName("head");
		
		Maths::Point3 leftLegPoleVectorOffset = Maths::MakePoint3(0.f, -0.4f, -0.4f);
		Maths::Point3 leftArmPoleVectorOffset = Maths::MakePoint3(-0.4f, 0.f, 0.4f);
		Maths::Point3 rightArmPoleVectorOffset = Maths::MakePoint3(0.4f, 0.f, 0.4f);

		m_leftArmIkChain.Create(*skeleton, "leftarm", "leftforearm", "lefthand", "leftforearm", leftArmPoleVectorOffset);
		m_rightArmIkChain.Create(*skeleton, "rightarm", "rightforearm", "righthand", "rightforearm", rightArmPoleVectorOffset);
		m_leftLegIkChain.Create(*skeleton, "leftupleg", "leftleg", "leftfoot", "leftupleg", leftLegPoleVectorOffset);
		m_rightLegIkChain.Create(*skeleton, "rightupleg", "rightleg", "rightfoot", "rightupleg", leftLegPoleVectorOffset);
		
		m_leftArmIkChain.SetNumTargets(E_ArmIkTarget_Count);
		m_rightArmIkChain.SetNumTargets(E_ArmIkTarget_Count);
		m_leftLegIkChain.SetNumTargets(E_LegIkTarget_Count);
		m_rightLegIkChain.SetNumTargets(E_LegIkTarget_Count);
		
		// Set up steering controller.
		m_steeringController.Clear();
		m_steeringController.AddArm(m_leftArmIkChain);
		m_steeringController.AddArm(m_rightArmIkChain);

		const C_DriverActorCreate* params = GetCreationParams();

		Maths::Point3 refPos = Maths::MakePoint3(gReferenceSteeringPos->vx, gReferenceSteeringPos->vy, gReferenceSteeringPos->vz);
		Maths::Mat43 refMatrix = MakeSteeringWheelMatrix(refPos, gReferenceSteeringTilt);
		m_steeringController.SetReferenceWheel(refMatrix, gReferenceSteeringRadius);
		
		Maths::Mat43 targetMatrix = MakeSteeringWheelMatrix(params->GetSteeringWheelPos(), params->GetSteeringWheelTilt());
		m_steeringController.SetTargetWheel(targetMatrix, params->GetSteeringWheelRadius());

		m_steeringController.SetDistanceFromWheelToHands(gDriverIkWheelToHandsDist);
		m_steeringController.SetFalloffDistance(gDriverIkFalloffDist);
		
		UpdateSteeringController();
	}
	else
	{
		m_headJointIndex = -1;
	}

	// Create pose format and pose.
	Animation::C_PoseFormat poseFormat;
	poseFormat.SetSkeleton(skeleton);

	m_evaluationState->Create(poseFormat, skeleton);
	
	//
	// Build animated model instance from the driver body and optional seatbelt models.
	//
	m_pDriverModel = new AnimatedModel::C_AnimatedModelInstance;
	m_pDriverModel->Create(m_pSceneGraph, poseFormat);
	
	for (U32 i = 0; i < E_ModelPart_Count; ++i)
	{
		C_DriverModelPart& mp = m_modelParts[i];
		if (mp.m_pResource)
		{
			U32 modelIndex = 0;
			const U32 numModels = mp.m_pResource->GetModelData()->GetModelCount();
			if (numModels > 1)
			{
				modelIndex = 1;
			}

			mp.m_partIndex = m_pDriverModel->AddPart(mp.m_pResource, modelIndex);
			mp.m_shNodeParameter = m_pDriverModel->GetModelInstance(mp.m_partIndex)->GetRenderModel()->GetParameter("g_SHNode");
			mp.m_envMapParameter = m_pDriverModel->GetModelInstance(mp.m_partIndex)->GetRenderModel()->GetParameter("g_tEnvironment");

			m_pDriverModel->GetModelInstance(mp.m_partIndex)->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, E_RenderPass_CharacterSolid);
			m_pDriverModel->GetModelInstance(mp.m_partIndex)->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_ALPHA_TEST, E_RenderPass_CharacterPunch);
			
			m_pDriverModel->SetVisibilityFlags(E_ViewTypeFlag_Main | E_ViewTypeFlag_Mirror);
		}
	}

	m_pDriverModel->InsertIntoSceneGraph();
	m_pDriverModel->SetNearFarDistance(0.f, gDebugDriverFarCullDistance);
	m_pVehicleEnvMapTexture = NULL;
	
	m_receivedCollision = false;
	
	// Initialise various parameters.
	m_maxCollisionImpulse = 0.f;
	m_localCollisionDir = Maths::Const<Maths::Normal3>::AxisZ();
	
	// Animation pose is currently undefined and must be evaluated at least once before rendering.
	m_hasValidAnimationPose = false;
	m_wasAnimatedLastUpdate = false;

	m_created = true;
	
	return true;
}

void C_DriverActor::SetVehicleModelInput(const Vehicle::C_ModelInput& modelInput)
{
	m_pVehicleModelInput = &modelInput;
}

void C_DriverActor::SetVehicleLightSampler(const WorldRender::C_LightSampler* pVehicleLightSampler)
{
	m_pVehicleLightSampler = pVehicleLightSampler;
}

void C_DriverActor::SetVehicleCubeMapTexture(const void* pCubeMapTexture)
{
	if (pCubeMapTexture == m_pVehicleEnvMapTexture)
	{
		return;
	}

	m_pVehicleEnvMapTexture = pCubeMapTexture;

	// Update env map texture.
	for (U32 i = 0; i < E_ModelPart_Count; ++i)
	{
		const C_DriverModelPart& mp = m_modelParts[i];

		WorldRender::C_ModelInstance* pModelInstance = m_pDriverModel->GetModelInstance(mp.m_partIndex);
		if (pModelInstance != NULL)
		{
			if (mp.m_envMapParameter.index >= 0)
			{
				pModelInstance->SetParameter(mp.m_envMapParameter, &m_pVehicleEnvMapTexture);
			}
		}
	}
}

void C_DriverActor::SetVisibility(bool isVisible, U32 viewportIndex)
{
	U32 viewFlag = g_MainViewFlags[viewportIndex];
	if (isVisible)
	{
		m_pDriverModel->EnableVisibilityFlags(viewFlag);
	}
	else
	{
		m_pDriverModel->DisableVisibilityFlags(viewFlag);
	}
}

void C_DriverActor::SetVisibilityDetectionFlags(U32 flags)
{
	m_visibilityDetectionFlags = flags;
}

DECLARE_DEBUG_BOOL(gDebugForceDriverBasePose, "ActorDebug.Actors.Driver.ForceBasePose", false);
DECLARE_DEBUG_BOOL(gDebugDriverEnableIk, "ActorDebug.Actors.Driver.EnableIk", true);
DECLARE_DEBUG_BOOL(gDebugDriverEnableOffsetOverride, "ActorDebug.Actors.Driver.EnableOffsetOverride", false);
DECLARE_DEBUG_VECTOR3(gDebugDriverOffsetOverride, "ActorDebug.Actors.Driver.OffsetOverride", 0.f, 0.f, 0.f);
DECLARE_DEBUG_FLOAT(gDebugDriverFarAnimationDistance, "ActorDebug.Actors.Driver.FarAnimationDistance", 10.f, Debug::Range(0.f, 100.f));
DECLARE_DEBUG_FLOAT(gDebugDriverSteeringWheelFullLockAngle, "ActorDebug.Actors.Driver.SteeringWheelFullLockAngle", 85.f, Debug::Range(0.f, 360.f));
DECLARE_DEBUG_BOOL(gDebugDriverOnlyUpdateIfVisible, "ActorDebug.Actors.Driver.OnlyUpdateIfVisible", true);

void C_DriverActor::Update(F32 deltaTime, const Maths::HVec4* pLodScaleAndOffset, U32 numViewports)
{
	PERF_EVENT("DriverActor.Update");

	if (!m_evaluationState)
	{
		// Driver not created yet.
		return;
	}

	if (!m_pVehicleModelInput)
	{
		// No information about vehicle - this happens quite often during loading, where vehicles haven't been fully created yet.
		return;
	}

	// Store previous frame's driver position for calculating (lagged) view distance later on.
	Maths::HVec3 driverPosLastUpdate = m_matrix.Translation();

	// Compute world matrix.
	Maths::HMat43 worldMatrix = m_pVehicleModelInput->m_mMatrix;
	worldMatrix.Translation(GetDriverOffset() * worldMatrix);	// always being computed, even though offset is constant?
	m_matrix = worldMatrix;

#ifndef CFG_SPEED
	UpdateFromDebugVariables();
#endif

	U32 visFlags = (m_pDriverModel->GetVisibilityFlags() & m_visibilityDetectionFlags);
	bool isVisible = (visFlags != 0);
	m_receivedCollision = false;

	if (m_hasValidAnimationPose)
	{
		// Don't necessarily need to Animate() unless visible and close enough.
		
		if (isVisible)
		{
			// Visible - see if we're close enough to animate.

			// Calculate LOD distance now using previous frame's position and camera position.
			// Easier to do this and accept a frame's latency instead of running yet another update
			// after the camera position has been updated later on.
			Maths::HF32 maxDist((F32)gDebugDriverFarAnimationDistance);
			Maths::HF32 minDist(maxDist);
			Maths::HF32 zero;
			for (U32 i = 0; i < numViewports; ++i)
			{
				if (visFlags & g_MainViewFlags[i])
				{
					Maths::HF32 d = (driverPosLastUpdate | pLodScaleAndOffset[i].XYZ()) + pLodScaleAndOffset[i].W();
					minDist = Maths::Min(d, minDist);
				}
			}

			bool isNearbyToViewer = minDist > zero && minDist < maxDist;
			if (isNearbyToViewer)
			{
				Animate(deltaTime);
				return;
			}
		}

		BIZ_ASSERT(m_hasValidAnimationPose);
		m_pDriverModel->SetWorldMatrix(worldMatrix);
		m_wasAnimatedLastUpdate = false;
	}
	else
	{
		// No valid pose - must Animate()
		Animate(deltaTime);
	}
}

void C_DriverActor::Reset()
{
	m_currentGear = -1;
	m_handbrake = -1;
	m_steeringWheelRotationAngle = 0.f;

	m_animParameters.CancelActions(m_animTree);
}

void C_DriverActor::OnImpactMessage(const Game::C_ImpactMsg* msg)
{
	F32 newImpulse = msg->mCollisionInfo.m_GameplayImpulse;
	F32 oldImpulse = 0.f;
	if (m_receivedCollision)
	{
		oldImpulse = m_maxCollisionImpulse;
	}
	if (newImpulse > oldImpulse)
	{
		m_maxCollisionImpulse = newImpulse;

		// Calculate a local space collision direction.
		if (msg->mCollisionInfo.m_ContactPoints.Size() > 0)
		{
			Maths::HVec3 localContactPt = msg->mCollisionInfo.m_ContactPoints[0].m_ContactPoint * m_matrix.InverseRotTrans();
			m_localCollisionDir = Maths::Normalise(localContactPt);
		}
		else
		{
			m_localCollisionDir = Maths::Const<Maths::Normal3>::AxisZ();
		}
	}

	m_receivedCollision = true;
	m_animParameters.actions[E_DriverAction_Jolt].Request(gDriverCollisionRequestLifetime);	// random delay?
}

struct C_CutSceneAnimPair
{
	ActorMsg::E_CutSceneDriverAnim	first;
	ActorMsg::E_CutSceneDriverAnim	second;
};

void C_DriverActor::OnPlayDriverAnimation(const ActorMsg::C_SetDriverAnimMsg* pMsg)
{
	using namespace ActorMsg;
	
	static const C_CutSceneAnimPair kAnimPairs[] =
	{
		{ E_CutSceneDriverAnim_LookBehindLeft,	E_CutSceneDriverAnim_LookBehindRight },
		{ E_CutSceneDriverAnim_GlanceLeft,		E_CutSceneDriverAnim_GlanceRight }	
	};
	
	ActorMsg::E_CutSceneDriverAnim anim = pMsg->mPlayDriverAnim;
	
	for (U32 i = 0, count = sizeof(kAnimPairs)/sizeof(kAnimPairs[0]); i < count; ++i)
	{
		if (anim == kAnimPairs[i].first)
		{
			anim = kAnimPairs[i].second;
		}
		else if (anim == kAnimPairs[i].second)
		{
			anim = kAnimPairs[i].first;
		}
	}
	
	m_animTree.SetAnimationIndex(0, anim);
	m_animParameters.actions[E_DriverAction_PlayCutSceneAnim].Request(gDriverCutsceneAnimRequestLifetime);
}

void C_DriverActor::HitByPerk()
{
	m_animParameters.actions[E_DriverAction_ShowFrustration].Request(gDriverFrustrationRequestLifetime);
}

void C_DriverActor::Overtake(const Maths::HVec3& otherPos, bool isOvertaker)
{
	if (!m_wasAnimatedLastUpdate)
	{
		return;
	}

	m_otherDriverPos = otherPos;

	const Maths::HVec3 localPos = Maths::HVec3(Maths::C_Vertex(otherPos) * m_matrix.InverseRotTrans());
	const Maths::HF32 zero;
				
	// Filter out vehicles that might be miles away,
	const F32 kMaxDistance = 30.f;
	const F32 kMaxDistanceSquared = kMaxDistance * kMaxDistance;
	
	if (localPos.LengthSquared() > kMaxDistanceSquared)
	{
		return;
	}
	
	U32 opponentOnRight = (localPos.X() > zero) ? 1 : 0;
	
	// Reverse messages as animation will be mirrored with right hand drive cars.
	if (m_vehicleIsRightHandDrive) opponentOnRight ^= 1;
	
	static const E_DriverAction action[2][2] =
	{
		{ E_DriverAction_PassedOnLeft, E_DriverAction_PassedOnRight },
		{ E_DriverAction_OvertakeOnRight, E_DriverAction_OvertakeOnLeft }
	};
	
	m_animParameters.actions[action[isOvertaker ? 1 : 0][opponentOnRight]].Request(gDriverOvertakeRequestLifetime);
}

const F32 kEndOfRaceRequestLifetime = 20.f;

void C_DriverActor::PassedEvent()
{
	m_animParameters.actions[E_DriverAction_Celebrate].Request(kEndOfRaceRequestLifetime);
}

void C_DriverActor::FailedEvent()
{
	m_animParameters.actions[E_DriverAction_ShowFrustration].Request(kEndOfRaceRequestLifetime);
}

void C_DriverActor::VehicleFatallyDamaged()
{
	m_animParameters.actions[E_DriverAction_ShowFrustration].Request(gDriverFrustrationRequestLifetime);
}

#ifndef CFG_GOLD

Debug::Int gDebugDriverSpecificIndex("ActorDebug.Actors.Driver.SpecificIndex", 0, Debug::Range(-1,20));
Debug::Bool gDebugShowDriverBones("ActorDebug.Actors.Driver.ShowBones", false);
Debug::Bool gDebugShowDriverBoneTransforms("ActorDebug.Actors.Driver.ShowBoneTransforms", false);
Debug::Bool gDebugShowDriverBoneNames("ActorDebug.Actors.Driver.ShowBoneNames", false);
Debug::Bool gDebugShowDriverIk("ActorDebug.Actors.Driver.ShowIk", false);
Debug::Bool gDebugShowDriverCommandList("ActorDebug.Actors.Driver.ShowCommandList", false, Debug::Persist());
Debug::Bool gDebugShowDriverAnimTree("ActorDebug.Actors.Driver.ShowAnimTree", false);
Debug::Bool gDebugShowDriverAnimTreeMemory("ActorDebug.Actors.Driver.ShowAnimTreeMemory", false);
Debug::Bool gDebugShowDriverCycleStats("ActorDebug.Actors.Driver.ShowCycleStats", false);
Debug::Bool gDebugShowDriverOpponentLocation("ActorDebug.Actors.Driver.ShowOpponentLocation", false);
Debug::Bool gDebugShowDriverModelNames("ActorDebug.Actors.Driver.ShowModelNames", false);
Debug::Bool gDebugShowDriverSkeletonNames("ActorDebug.Actors.Driver.ShowSkeletonNames", false);
Debug::Bool gDebugShowDriverVehicleInfo("ActorDebug.Actors.Driver.ShowVehicleInfo", false);
Debug::Bool gDebugShowDriverCollisionImpulse("ActorDebug.Actors.Driver.ShowCollisionImpulse", false);
Debug::Bool gDebugShowDriverVehicleAxes("ActorDebug.Actors.Driver.ShowVehicleAxes", false);
Debug::Bool gDebugShowDriverViewDistance("ActorDebug.Actors.Driver.ShowViewDistance", false);
Debug::Bool gDebugShowDriverMotion("ActorDebug.Actors.Driver.ShowMotion", false);
Debug::Bool gDebugShowDriverAnimParams("ActorDebug.Actors.Driver.ShowAnimParams", false);
Debug::Bool gDebugShowDriverFeedbackParams("ActorDebug.Actors.Driver.ShowFeedbackParams", false);
Debug::Float gDebugDriverDebugTextSize("ActorDebug.Actors.Driver.DebugTextSize", 20.f, Debug::Range(1.f, 40.f));
Debug::Float gDebugDriverDebugTextPosY("ActorDebug.Actors.Driver.DebugTextPosY", 158.f, Debug::Range(-300.f, 300.f));
Debug::Bool gDebugShowDriverActions("ActorDebug.Actors.Driver.ShowActions", false);

void C_DriverActor::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if (gDebugDriverSpecificIndex != -1 && gDebugDriverSpecificIndex != m_vehicleIndex)
	{
		return;
	}

	debugRenderPtr->SetDepthEnable(false);
	
	if (gDebugShowDriverVehicleAxes)
	{
		debugRenderPtr->Draw3dMatrix(GetVehicleWorldMatrix(), 1.f);
	}
	
	if (gDebugShowDriverOpponentLocation)
	{
		Maths::HVec3 opponentPos = m_otherDriverPos;
		debugRenderPtr->Draw3dArrow(m_matrix.Translation(), opponentPos - m_matrix.Translation(), 1.f, 1.f, Maths::RGBA(0xff,0xff,0x00));
		debugRenderPtr->Draw3dMatrix(m_matrix, 1.f);
	}
	
	const C_DriverActorCreate* params = GetCreationParams();
	
	{	
		const C_DriverActorCreate* p = GetCreationParams();
		if (p != NULL)
		{
			Util::C_String msg;
			if (p && gDebugShowDriverModelNames)
			{
				msg += p->GetDriverModelFilename();
				msg += "\n";
			}
			if (p && gDebugShowDriverSkeletonNames)
			{
				msg += p->GetSkeletonFilename();
				msg += "\n";
			}
			if (p && gDebugShowDriverVehicleInfo)
			{
				msg += Util::C_String::Format("Vehicle %d, %s hand drive\n", p->vehicleIndex, p->IsRightHandDrive() ? "right" : "left");
			}
			if (gDebugShowDriverCollisionImpulse)
			{
				msg += Util::C_String::Format("CollisionImpulse: %.3f\n", m_animParameters.parameters[E_DriverParam_CollisionImpulse]);
			}
			if (gDebugShowDriverViewDistance)
			{
				F32 adjustedDist = m_lodDistance;
				msg += Util::C_String::Format("%.3f\n", adjustedDist);
			}
			if (msg.Length() > 0)
			{
				debugRenderPtr->Draw3dText(m_matrix.Translation(), msg);
			}
		}
	}
	
	if (m_skeletonResource != NULL)
	{
		if (gDebugShowDriverBoneNames || gDebugShowDriverBoneTransforms || gDebugShowDriverBones)
		{
			const Maths::RGBA boneColour(0xaa, 0xf0, 0xaa, 0xff);
			
			Animation::DebugRenderPoseTransforms(
				debugRenderPtr,
				m_evaluationState->GetPose(),
				m_skeletonResource->GetSkeleton(),
				m_matrix,
				gDebugShowDriverBoneTransforms,
				gDebugShowDriverBoneNames,
				gDebugShowDriverBones,
				boneColour
			);
			
		}

	}
	
	if (gDebugShowDriverIk && m_skeletonResource != NULL)
	{
		if (m_skeletonResource->GetSkeleton() != NULL)
		{
			m_leftLegIkChain.DebugRender(debugRenderPtr, m_matrix);
			m_rightLegIkChain.DebugRender(debugRenderPtr, m_matrix);
			m_leftArmIkChain.DebugRender(debugRenderPtr, m_matrix);
			m_rightArmIkChain.DebugRender(debugRenderPtr, m_matrix);
			
			// Draw wheel.
			
			if (params->IsSteeringWheelIkEnabled() || gDebugDriverForceEnableSteeringIk)
			{
				m_steeringController.DebugRender(debugRenderPtr, m_matrix, GetSteeringWheelRotationAngle());
			}
						
		}
	}
	
	const Maths::RGBA kWhite( 255, 255, 255, 255 );
	const Maths::RGBA kRed( 255, 64, 64, 255 );
	const Maths::RGBA kTextColour = kWhite;
	const F32 kTextHeight = gDebugDriverDebugTextSize;

	Maths::HVec2 textPos(50.f, (F32) gDebugDriverDebugTextPosY);

	if (gDebugShowDriverCommandList || gDebugShowDriverAnimTree || gDebugShowDriverAnimParams || gDebugShowDriverFeedbackParams)
	{
		debugRenderPtr->Draw2dBar(textPos, Maths::HVec2(450.f, 400.f), Maths::RGBA(0, 0, 0, 0x60));
	}
	
	if (gDebugShowDriverCommandList)
	{
		// Draw command list.
		static const char* blendModes[] = 
		{
			"ACC",
			"LRP",
			"ADD",
			"N/A"
		};		
		
		Btl::String128 str;
		
		//str.Printf( "UpdateCounter: %d", graph->m_cmdList.GetUpdateCounter() );
		//fr->Print( pos, str.Chars() );
		//pos += Maths::HVecY( fr->GetHeight() );

		const Animation::C_Skeleton* skeleton = NULL;
		if (m_skeletonResource != NULL)
		{
			skeleton = m_skeletonResource->GetSkeleton();
		}
				
		const Animation::C_CommandList& cmdList = m_evaluationState->GetCommandList();
		for ( U32 i = 0; i < cmdList.GetNumCommands(); ++i )
		{
			const Animation::C_Command& cmd = cmdList.GetCommand( i );
			
			switch ( cmd.type )
			{
				case Animation::E_CommandType_BlendCycle:
					str.Printf( "Blend (%s, %s [%.1fKb], %.3f, %.3f/%.3f)", blendModes[cmd.blendCycle.blendMode], cmd.blendCycle.cycleInstance->GetCycleName().GetString(), cmd.blendCycle.cycleInstance->GetCycleMemoryUsage() / 1024.f, cmd.blendCycle.weight, fmodf( cmd.blendCycle.time * Maths::Reciprocal(cmd.blendCycle.cycleInstance->GetDuration()), 1.f ), cmd.blendCycle.time * cmd.blendCycle.cycleInstance->GetFrameRate() );
					break;
				case Animation::E_CommandType_PopAndBlendPose:
					str.Printf( "Pop (%s)", blendModes[cmd.popPose.blendMode] );
					break;
				case Animation::E_CommandType_PushCurrentPose:
					str.Printf( "Push (%.3f)", cmd.pushPose.weight );
					break;
				case Animation::E_CommandType_SetIkTargetFromJoint:
					str.Printf( "SetIkTargetFromJoint %d %s%s %s (%d)",
						cmd.setIkTargetFromJoint.targetIndex,
						cmd.setIkTargetFromJoint.setRotation ? "R" : ".",
						cmd.setIkTargetFromJoint.setPosition ? "T" : ".",
						skeleton != NULL ? skeleton->GetJointName(cmd.setIkTargetFromJoint.jointIndex).GetString() : "unknown joint",
						cmd.setIkTargetFromJoint.jointIndex
					);
					break;

				case Animation::E_CommandType_SetIkTargetWeights:
				{
					str.Printf("SetIkTargetWeights ");
					for (U32 i = 0; i < cmd.setIkTargetWeights.count; ++i)
					{
						// chain, target, weight
						str += Btl::String32::Format("%d->%.2f%s", cmd.setIkTargetWeights.targets[i], cmd.setIkTargetWeights.weights[i], i == cmd.setIkTargetWeights.count - 1 ? "" : ", ");
					}
					break;
				}
						
				case Animation::E_CommandType_EvaluateIkChain:
					str.Printf( "EvaluateIkChain");
					break;
					
				case Animation::E_CommandType_MirrorPose:
					str.Printf( "MirrorPose (%c)", "XYZ"[cmd.mirrorPose.poseMirror->GetMirrorAxis()] );
					break;
					
				default:
					str = "? Unknown ?";
					break;
			}
			
			debugRenderPtr->Draw2dText(textPos, str.Chars(), kTextColour, kTextHeight);
			textPos += Maths::HVecY(kTextHeight * 1.1f);
		}
	}
	
	if (gDebugShowDriverAnimTree)
	{
		m_animTree.DebugRender(debugRenderPtr, textPos, kTextHeight, AnimationTree::C_Tree::E_DebugRenderMode_ActiveTree);
	}

	if (gDebugShowDriverAnimTreeMemory)
	{
		m_animTree.DebugRender(debugRenderPtr, textPos, kTextHeight, AnimationTree::C_Tree::E_DebugRenderMode_NodeMemoryStats);
	}

	if (gDebugShowDriverCycleStats)
	{
		m_animTree.DebugRender(debugRenderPtr, textPos, kTextHeight, AnimationTree::C_Tree::E_DebugRenderMode_CycleStats);
	}
	
	if (gDebugShowDriverMotion)
	{
		Maths::HVec3 minusOne(-1.f, -1.f, -1.f);
		Maths::HVec3 plusOne(1.f, 1.f, 1.f);
	
		debugRenderPtr->Draw3dBox(minusOne, plusOne, m_matrix, Maths::RGBA(0x40, 0x40, 0x40));
		debugRenderPtr->Draw3dBox(minusOne * 2.f, plusOne * 2.f, m_matrix, Maths::RGBA(0x40, 0x40, 0x40));
	
		//debugRenderPtr->Draw3dArrow(m_matrix.Translation(), m_vehicleVelocity, 1.f, 0.1f, Maths::RGBA(0x40, 0xff, 0x40));
		//debugRenderPtr->Draw3dArrow(m_matrix.Translation(), m_vehicleAcceleration * (1.f / 9.81f), 1.f, 0.1f, Maths::RGBA(0xff, 0x80, 0x40));
		
		debugRenderPtr->Draw3dArrow(m_matrix.Translation(), Maths::HVec3(m_futurePosition) * m_matrix.Rotation(), 1.f, 0.1f, Maths::RGBA(0xff, 0xff, 0xff));
	}
	
	if (gDebugShowDriverAnimParams)
	{
		// Making a guerilla array.
		const F32* params = m_animParameters.parameters;
		static const CHAR8* kParamNames[] =
		{
			"Steering",
			"SpeedMph",
			"CollisionImpulse",
			"RandomSeed",
			"VehicleIndex",
			"G-ForceX",
			"G-ForceY",
			"G-ForceZ",
			"JoltX",
			"JoltZ",
			"PredictedTurn",
			"Roll",
			"Airbourne",
			"GravityDirX",
			"GravityDirZ"
		};
		
		Btl::String128 str;
		
		for (U32 i = 0; i < sizeof(kParamNames)/sizeof(kParamNames[0]); ++i)
		{
			str.Printf("%16s: %.3f", kParamNames[i], params[i]);
			debugRenderPtr->Draw2dText(textPos, str.Chars(), kTextColour, kTextHeight);
			textPos += Maths::HVecY(kTextHeight * 1.1f);
		}

		static const CHAR8* kFixedParamNames[] =
		{
			"DrivingStyle",
			"IsRelaxedAtLowSpeed",
			"RandomSeed",
			"CannotPutArmThroughWindow"
		};

		U32 numFixedParams = sizeof(kFixedParamNames)/sizeof(kFixedParamNames[0]);
		if (numFixedParams > m_animTree.GetNumFixedParameters())
		{
			numFixedParams = m_animTree.GetNumFixedParameters();
		}

		debugRenderPtr->Draw2dText(textPos, "-- Fixed Parameters --", kTextColour, kTextHeight);
		textPos += Maths::HVecY(kTextHeight * 1.1f);

		for (U32 i = 0; i < numFixedParams; ++i)
		{
			str.Printf("%16s: %.3f", kFixedParamNames[i], m_animTree.GetFixedParameter(i));
			debugRenderPtr->Draw2dText(textPos, str.Chars(), kTextColour, kTextHeight);
			textPos += Maths::HVecY(kTextHeight * 1.1f);
		}
	}

	if (gDebugShowDriverFeedbackParams)
	{
		Util::C_String str;
		for (U32 i = 0; i < m_animTree.GetNumFeedbackParameters(); ++i)
		{
			str.Printf("[%d] %f", i, m_animTree.GetFeedbackParameter(i));
			debugRenderPtr->Draw2dText(textPos, str.Chars(), kTextColour, kTextHeight);
			textPos += Maths::HVecY(kTextHeight * 1.1f);
		}
	}

	if (gDebugShowDriverActions)
	{
		static const CHAR8* actionNames[] =
		{
			"Celebrate",
			"ShowFrustration",
			"LookLeft",
			"LookRight",
			"GlanceOutWindowLeft",
			"GlanceOutWindowRight",
			"Jolt",
			"OvertakeOnLeft",
			"OvertakeOnRight",
			"PassedOnLeft",
			"PassedOnRight",
			"ActivatedPickup",
			"PlayCustsceneAnim",
			"ChangeGear",
			"EngageHandbrake",
			"ReleaseHandbrake"
		};

		Util::C_String str;
		for (U32 i = 0; i < E_DriverAction_Count; ++i)
		{
			if (m_animParameters.actions[i].IsRequested() || m_animParameters.actions[i].IsCompleted())
			{
				str = actionNames[i];
				if (m_animParameters.actions[i].IsRequested())
				{
					str += Util::C_String::Format("(R %.3f) ", m_animParameters.actions[i].GetRequestLifetime());
				}
				if (m_animParameters.actions[i].IsCompleted())
				{
					str += "(C) ";
				}

				debugRenderPtr->Draw2dText(textPos, str.Chars(), kTextColour, kTextHeight);
				textPos += Maths::HVecY(kTextHeight * 1.1f);
			}
		}
	}
	
	debugRenderPtr->SetDepthEnable(true);
}
#endif

Maths::C_QuatTrans C_DriverActor::GetHeadTransform() const
{
	const Maths::C_QuatTrans modelWorldTransform(m_matrix);
	if (m_headJointIndex < 0)
	{
		return modelWorldTransform;
	}
	const Animation::C_Pose& pose = m_evaluationState->GetPoseReadOnly();
	const Animation::C_JointTransform& headTransform = pose.GetModelJointTransform(m_headJointIndex, m_skeletonResource->GetSkeleton());
	const Maths::C_QuatTrans headModelTransform(
		Maths::C_Quaternion(Maths::MakeQuaternion(Maths::MakeVec4F32(0.f, 1.f, 0.f, 0.f)) * headTransform.GetRotation()),
		Maths::HVec3(headTransform.GetTranslation())
	);
	return headModelTransform * modelWorldTransform;
}

F32 C_DriverActor::GetSteeringWheelRotationAngle() const
{
	return m_steeringWheelRotationAngle;
}


void C_DriverActor::OnXtAnimTreeCreated(XtRecord* record, const AnimationTree::C_XtTree* tree)
{
	// Check evaluation state is initialised as Xt updates are possible before Create() is called.
	if (!m_evaluationState)
	{
		return;
	}
	U8 stateBuffer[8192];
	const U32 kStateBufferSize = sizeof(stateBuffer) / sizeof(stateBuffer[0]);
	
	Animation::C_MemoryBlockWriter writer(stateBuffer, kStateBufferSize);
	m_animTree.SaveState(writer);
	
	BuildAnimTree(tree);
	
	Animation::C_MemoryBlockReader reader(stateBuffer, kStateBufferSize);
	m_animTree.LoadState(reader);
}

void C_DriverActor::OnXtAnimTreeDestroyed(XtRecord* record, const AnimationTree::C_XtTree* tree)
{
}

const char* C_DriverActor::GetXtAnimTreeName() const
{
	return !gDebugUseTestAnimationTree ? "driver_master" : "driver_test";
}

void AddToResourceBag(ResourceUtils::I_ResourceBag& resourceBag, const char* filename, ResourceUtils::C_ResourceBagTag& tag, const Resource::C_ResourceTypeId resTypeId)
{
	tag = MakeResourceBagTag(filename);
	resourceBag.AddResource(tag, filename, resTypeId);
}



void C_DriverActor::CreateAnimTree(ResourceUtils::I_ResourceBag& resourceBag)
{
	m_animTree.SetResourceBag(resourceBag);
	m_animTree.Clear();
	
	// Add varying parameters and actions.
	m_animParameters.Bind(m_animTree);
	
	// Add fixed parameters.
	S32 drivingStyleIndex = m_animTree.AddFixedParameter("DrivingStyle");
	S32 relaxedIndex = m_animTree.AddFixedParameter("IsRelaxedAtLowSpeed");
	S32 randomSeedIndex = m_animTree.AddFixedParameter("RandomSeed");
	S32 armThroughWindowIndex = m_animTree.AddFixedParameter("CannotPutArmThroughWindow");
	
	m_drivingStyle = GetCreationParams()->GetDrivingStyle();
	m_animTree.SetFixedParameter(drivingStyleIndex, static_cast<F32>(m_drivingStyle));

	U32 randomSeed256 = (m_animTree.Rand() >> 7) & 0xff;
	m_animTree.SetFixedParameter(randomSeedIndex, static_cast<F32>(randomSeed256));

	F32 allowRelaxed = 1.f;
	if (GetCreationParams()->CannotRestArmOnDoor())
	{
		allowRelaxed = 0.f;
	}
	else
	{
		if ((randomSeed256 & 7) <= 3)
		{
			allowRelaxed = 0.f;
		}
	}
	m_animTree.SetFixedParameter(relaxedIndex, allowRelaxed);
	m_animTree.SetFixedParameter(armThroughWindowIndex, GetCreationParams()->CannotPutArmThroughWindow() ? 1.f : 0.f);
	
	// Add IK chains.
	S32 leftArmIkChainIndex = m_animTree.AddIkChain("LeftArm", m_leftArmIkChain);
	S32 rightArmIkChainIndex = m_animTree.AddIkChain("RightArm", m_rightArmIkChain);
	S32 leftLegIkChainIndex = m_animTree.AddIkChain("LeftLeg", m_leftLegIkChain);
	S32 rightLegIkChainIndex = m_animTree.AddIkChain("RightLeg", m_rightLegIkChain);
	
	// Add IK targets.
	m_animTree.AddIkTarget("LeftHandSteering", leftArmIkChainIndex, E_ArmIkTarget_Steering);
	m_animTree.AddIkTarget("LeftHandMisc", leftArmIkChainIndex, E_ArmIkTarget_Misc);
	m_animTree.AddIkTarget("RightHandSteering", rightArmIkChainIndex, E_ArmIkTarget_Steering);
	m_animTree.AddIkTarget("RightHandMisc", rightArmIkChainIndex, E_ArmIkTarget_Misc);
	m_animTree.AddIkTarget("LeftFoot", leftLegIkChainIndex, E_LegIkTarget_Floor);
	m_animTree.AddIkTarget("RightFoot", rightLegIkChainIndex, E_LegIkTarget_Floor);
	
	const AnimationTree::C_XtTree* xtTree = AnimationTree::C_XtTree::Find(GetXtAnimTreeName());
	if (xtTree != NULL)
	{
		BuildAnimTree(xtTree);
	}
}


void C_DriverActor::Destroy()
{
	delete m_evaluationState;
	m_evaluationState = NULL;

	if (m_pDriverModel != NULL)
	{
		m_pDriverModel->RemoveFromSceneGraph();
		
		delete m_pDriverModel;
		m_pDriverModel = NULL;
	}
	m_created = false;
}

void C_DriverActor::Animate(F32 deltaTime)
{
	PERF_EVENT("DriverActor.Animate");

	if (!m_wasAnimatedLastUpdate)
	{
		Reset();
	}

	// Update parameters from VehicleModel.
	UpdateParameters(*m_pVehicleModelInput);
	m_animParameters.Update(m_animTree, deltaTime);

	Animation::C_CommandList& cmdList = m_evaluationState->GetCommandList();

	cmdList.Clear();

	if (!gDebugForceDriverBasePose)
	{
		m_animTree.Evaluate(cmdList, m_skeletonResource->GetSkeleton(), m_evaluationState->GetPose(), deltaTime);
	}

	m_steeringWheelRotationAngle = Maths::DegToRad((F32) gDebugDriverSteeringWheelFullLockAngle) * m_animTree.GetFeedbackParameter(0);
	if (m_vehicleIsRightHandDrive)
	{
		m_steeringWheelRotationAngle = -m_steeringWheelRotationAngle;
	}
	
	cmdList.SetModelWorldMatrix(m_matrix);
			
	if (gDebugDriverEnableIk)
	{
		const C_DriverActorCreate* params = GetCreationParams();

		//
		// Arm IK.
		//

		F32 steeringIkWeight = 0.f;

		if (params->IsSteeringWheelIkEnabled() || gDebugDriverForceEnableSteeringIk)
		{
			UpdateSteeringController();
		}

		//
		// Use IK to move feet upwards to avoid clipping through cabin floor.
		//
		
		Maths::Vector3 cabinFloorOffset = Maths::MakeVector3(0.f, params->GetCabinFloorOffset(), -params->GetCabinFootwellOffset());
		
		F32 floorIkWeight = 1.f;
		
		m_leftLegIkChain.SetTargetPositionFromEffector(E_LegIkTarget_Floor);
		m_leftLegIkChain.SetTargetPositionOffset(E_LegIkTarget_Floor, cabinFloorOffset);
		m_leftLegIkChain.SetLocalTargetRotationFromEffector(E_LegIkTarget_Floor);

		m_leftLegIkChain.SetTargetWeight(E_LegIkTarget_Floor, floorIkWeight);
		
		m_rightLegIkChain.SetTargetPositionFromEffector(E_LegIkTarget_Floor);
		m_rightLegIkChain.SetTargetPositionOffset(E_LegIkTarget_Floor, cabinFloorOffset);
		m_rightLegIkChain.SetLocalTargetRotationFromEffector(E_LegIkTarget_Floor);
		
		m_rightLegIkChain.SetTargetWeight(E_LegIkTarget_Floor, floorIkWeight);
	}
	
	// Update and apply dynamic joints.
	m_evaluationState->GetSkeletonInstance().UpdateDynamicJoints(deltaTime);
	cmdList.ApplyConstraints(m_evaluationState->GetSkeletonInstance(), m_matrix);
		
	// Ensure that all cycle instances have been bound to the driver's pose. 
	//cmdList.BindToPose(m_evaluationState->GetPose());
	
	// Schedule asynchronous animation evaluation and model update tasks.
	m_evaluationState->ScheduleEvaluationTask();
	m_pDriverModel->ScheduleUpdateTask(m_matrix, m_evaluationState->GetPoseNonBlocking(), m_evaluationState->GetPoseMatrixCacheNonBlocking());
			
	m_hasValidAnimationPose = true;
	m_wasAnimatedLastUpdate = true;
}

void C_DriverActor::UpdateSteeringController()
{
#ifndef CFG_SPEED
	// Live update of steering wheel controller parameters.
	const C_DriverActorCreate* params = GetCreationParams();

	Maths::Point3 refPos = Maths::MakePoint3(gReferenceSteeringPos->vx, gReferenceSteeringPos->vy, gReferenceSteeringPos->vz);
	Maths::Mat43 refMatrix = MakeSteeringWheelMatrix(refPos, gReferenceSteeringTilt);
	m_steeringController.SetReferenceWheel(refMatrix, gReferenceSteeringRadius);
	
	Maths::Mat43 targetMatrix = MakeSteeringWheelMatrix(params->GetSteeringWheelPos(), params->GetSteeringWheelTilt());
	m_steeringController.SetTargetWheel(targetMatrix, params->GetSteeringWheelRadius());

	m_steeringController.SetDistanceFromWheelToHands(gDriverIkWheelToHandsDist);
	m_steeringController.SetFalloffDistance(gDriverIkFalloffDist);

#endif	

	m_steeringController.Update(m_vehicleIsRightHandDrive);
}

DECLARE_DEBUG_FLOAT(gDriverTurnLookaheadTime, "ActorDebug.Actors.Driver.TurnLookaheadTime", 2.f)
DECLARE_DEBUG_FLOAT(gDriverTurnDistanceThreshold, "ActorDebug.Actors.Driver.TurnDistanceThreshold", 4.f)

#ifndef CFG_GOLD
DECLARE_DEBUG_FLOAT(gDebugDriverJoltStrength, "ActorDebug.Actors.Driver.JoltStrength", 2.f);

bool gDebugApplyJolt = false;
F32 gDebugJoltVector[3];

static void JoltLeft(void*)
{
	gDebugApplyJolt = true;
	gDebugJoltVector[0] = -gDebugDriverJoltStrength;
	gDebugJoltVector[1] = 0.f;
	gDebugJoltVector[2] = 0.f;
}

static void JoltRight(void*)
{
	gDebugApplyJolt = true;
	gDebugJoltVector[0] = gDebugDriverJoltStrength;
	gDebugJoltVector[1] = 0.f;
	gDebugJoltVector[2] = 0.f;
}

static void JoltForwards(void*)
{
	gDebugApplyJolt = true;
	gDebugJoltVector[0] = 0.f;
	gDebugJoltVector[1] = 0.f;
	gDebugJoltVector[2] = gDebugDriverJoltStrength;
}

static void JoltBackwards(void*)
{
	gDebugApplyJolt = true;
	gDebugJoltVector[0] = 0.f;
	gDebugJoltVector[1] = 0.f;
	gDebugJoltVector[2] = -gDebugDriverJoltStrength;
}

DECLARE_DEBUG_FUNCTION(gDebugDriverJoltLeft, "ActorDebug.Actors.Driver.Triggers.JoltLeft", &JoltLeft, 0);
DECLARE_DEBUG_FUNCTION(gDebugDriverJoltRight, "ActorDebug.Actors.Driver.Triggers.JoltRight", &JoltRight, 0);
DECLARE_DEBUG_FUNCTION(gDebugDriverJoltForwards, "ActorDebug.Actors.Driver.Triggers.JoltForwards", &JoltForwards, 0);
DECLARE_DEBUG_FUNCTION(gDebugDriverJoltBackwards, "ActorDebug.Actors.Driver.Triggers.JoltBackwards", &JoltBackwards, 0);

static S32 gDebugDriverAction = -1;

static void DriverDoAction(void* param)			{ gDebugDriverAction = (S32)(INTPTR) param; }	// [Blur2PC][x64] action id carried in the pointer
DECLARE_DEBUG_FUNCTION(gDebugDriverDoAction1, "ActorDebug.Actors.Driver.Triggers.Celebrate", &DriverDoAction,		(void*)E_DriverAction_Celebrate);
DECLARE_DEBUG_FUNCTION(gDebugDriverDoAction2, "ActorDebug.Actors.Driver.Triggers.Frustration", &DriverDoAction, (void*)E_DriverAction_ShowFrustration);
DECLARE_DEBUG_FUNCTION(gDebugDriverDoAction3, "ActorDebug.Actors.Driver.Triggers.OvertakeOnLeft", &DriverDoAction, (void*)E_DriverAction_OvertakeOnLeft);
DECLARE_DEBUG_FUNCTION(gDebugDriverDoAction4, "ActorDebug.Actors.Driver.Triggers.OvertakeOnRight", &DriverDoAction, (void*)E_DriverAction_OvertakeOnRight);
DECLARE_DEBUG_FUNCTION(gDebugDriverDoAction5, "ActorDebug.Actors.Driver.Triggers.PassedOnLeft", &DriverDoAction, (void*)E_DriverAction_PassedOnLeft);
DECLARE_DEBUG_FUNCTION(gDebugDriverDoAction6, "ActorDebug.Actors.Driver.Triggers.PassedOnRight", &DriverDoAction, (void*)E_DriverAction_PassedOnRight);
DECLARE_DEBUG_FUNCTION(gDebugDriverDoAction9, "ActorDebug.Actors.Driver.Triggers.ChangeGear", &DriverDoAction, (void*)E_DriverAction_ChangeGear);
DECLARE_DEBUG_FUNCTION(gDebugDriverDoActionA, "ActorDebug.Actors.Driver.Triggers.EngageHandbrake", &DriverDoAction, (void*)E_DriverAction_EngageHandbrake);
DECLARE_DEBUG_FUNCTION(gDebugDriverDoActionB, "ActorDebug.Actors.Driver.Triggers.ReleaseHandbrake", &DriverDoAction, (void*)E_DriverAction_ReleaseHandbrake);
DECLARE_DEBUG_FLOAT(gDebugDriverRequestLifetime, "ActorDebug.Actors.Driver.Triggers.RequestLifetime", 4.f, Debug::Range(0.f, 5.f))

#endif

void C_DriverActor::UpdateParameters(const Vehicle::C_ModelInput& modelInput)
{
	PERF_EVENT("DriverActor.UpdateParameters");
	
	m_animParameters.parameters[E_DriverParam_Airbourne] = modelInput.m_bAllWheelsOnGround ? 0.f : 1.f;

	Maths::Mat43 vehicleWorldMatrix= Maths::Mat43(modelInput.m_mMatrix);
	Maths::Mat43 worldToVehicleMatrix = Maths::Inverse(vehicleWorldMatrix);
	
	m_animParameters.parameters[E_DriverParam_RandomSeed] = m_animTree.FRand();
	m_animParameters.parameters[E_DriverParam_CollisionImpulse] = m_maxCollisionImpulse;
	m_animParameters.parameters[E_DriverParam_VehicleIndex] = static_cast<F32>(m_vehicleIndex);	// TODO: this is constant?
	
	const Maths::Normal3 worldUp = Maths::Const<Maths::Normal3>::AxisY();
	
	Maths::VF32 one = Maths::Const<Maths::VF32>::One();
	Maths::VF32 xFlip = one;
	Maths::Vector3 flipSign = Maths::Const<Maths::Vector3>::One();
	
	if (m_vehicleIsRightHandDrive)
	{
		xFlip = -xFlip;
		Maths::SetX(&flipSign, -one);
	}
	
	
	//
	// Calculate acceleration and velocity parameters.
	//
	static const F32 fInvGravity = 1.f / 9.81f;
	Maths::VF32 invGravity = Maths::MakeVF32(&fInvGravity);
	Maths::Vector3 worldAcceleration = Maths::Vector3(modelInput.m_vAcceleration);
	Maths::Vector3 localGForce = (worldAcceleration * invGravity) * worldToVehicleMatrix;
	Maths::Vector3 worldVelocity = Maths::Vector3(modelInput.m_vVelocity);
	Maths::Vector3 localVelocity = worldVelocity * worldToVehicleMatrix;
	
	Maths::Vector3 localVelocityDir = localVelocity;
	Maths::Normalise(&localVelocityDir, Maths::Const<Maths::Vector3>::Zero());

	localGForce *= flipSign;

	Maths::Store(localGForce, &m_animParameters.parameters[E_DriverParam_GForceX]);

	Maths::Vector3 localJolt = m_localCollisionDir * Maths::MakeVF32(m_maxCollisionImpulse);


#ifndef CFG_GOLD
	if ((S32)gDebugDriverSpecificIndex == m_vehicleIndex)
	{
		if (gDebugApplyJolt)
		{
			gDebugApplyJolt = false;
			localJolt = Maths::MakeVector3(gDebugJoltVector[0], gDebugJoltVector[1], gDebugJoltVector[2]);
			m_animParameters.actions[E_DriverAction_Jolt].Request((F32)gDebugDriverRequestLifetime);
		}
		if (gDebugDriverAction >= 0)
		{
			m_animParameters.actions[gDebugDriverAction].Request((F32)gDebugDriverRequestLifetime);
			gDebugDriverAction = -1;
		}
	}
#endif

	if (m_animParameters.actions[E_DriverAction_Jolt].IsRequested())
	{
		m_animParameters.parameters[E_DriverParam_JoltX] = Maths::GetF32(Maths::X(localJolt) * xFlip);
		m_animParameters.parameters[E_DriverParam_JoltZ] = Maths::GetF32(Maths::Z(localJolt));
	}
	
	//
	// Predict future position of car for head turning.
	//
	Maths::Vector3 localGroundVelocity = Maths::Reject(worldVelocity, worldUp) * worldToVehicleMatrix;
	Maths::Vector3 localGroundAcceleration = Maths::Reject(worldAcceleration, worldUp) * worldToVehicleMatrix;
	
	Maths::VF32 lookaheadTime = Maths::MakeVF32((F32)gDriverTurnLookaheadTime);
	
	// Calc future position of car in local space using s = ut + 1/2at^2
	const Maths::VF32 half = Maths::Const<Maths::VF32>::Half();
	const Maths::VF32 halfTimeSquared = half * lookaheadTime * lookaheadTime;
	
	Maths::Vector3 localFutureTranslation = (localGroundVelocity * lookaheadTime) + halfTimeSquared * localGroundAcceleration;

	// Apply dead zone at low speeds.
	localFutureTranslation = Maths::Select(Maths::Length(localFutureTranslation) < Maths::MakeVF32((F32)gDriverTurnDistanceThreshold), Maths::Const<Maths::Vector3>::Zero(), localFutureTranslation);

	m_futurePosition = Maths::MakePoint3(localFutureTranslation);
	
	Maths::Vector3 futureDir = Maths::MakeVector3(m_futurePosition);
	Maths::Normalise(&futureDir, Maths::Const<Maths::Vector3>::Zero());
	
	// Use direction of future car position as predicted turn parameter, in the range -1 to +1.
	m_animParameters.parameters[E_DriverParam_PredictedTurn] = Maths::GetF32(Maths::X(futureDir) * xFlip);
	
	m_animParameters.parameters[E_DriverParam_Roll] = Maths::GetF32(Maths::Y(Maths::RowY(vehicleWorldMatrix)));

	// Local gravity direction (assuming -Y in world space).
	Maths::Vector3 localGravityDir = -Maths::RowY(worldToVehicleMatrix);
	m_animParameters.parameters[E_DriverParam_GravityDirX] = Maths::GetF32(Maths::X(localGravityDir) * xFlip);
	m_animParameters.parameters[E_DriverParam_GravityDirZ] = Maths::GetF32(Maths::Z(localGravityDir));

	//
	// Generate events for gear changes and handbrake
	//
	if (m_currentGear >= 0 && modelInput.m_iGear != m_currentGear)
	{
		m_animParameters.actions[E_DriverAction_ChangeGear].Request(gDriverChangeGearRequestLifetime);
	}
	m_currentGear = modelInput.m_iGear;

	S32 handbrake = modelInput.m_bHandbrake ? 1 : 0;
	if (m_handbrake >= 0)
	{
		if (handbrake != m_handbrake)
		{
			if (handbrake)
			{
				m_animParameters.actions[E_DriverAction_EngageHandbrake].Request(gDriverHandbrakeRequestLifetime);
				m_animParameters.actions[E_DriverAction_ReleaseHandbrake].Cancel();
			}
			else
			{
				m_animParameters.actions[E_DriverAction_EngageHandbrake].Cancel();
				m_animParameters.actions[E_DriverAction_ReleaseHandbrake].Request(gDriverHandbrakeRequestLifetime);
			}
		}
	}
	m_handbrake = handbrake;

	//
	// Steering.
	//
	F32 steering = modelInput.m_fSteering;
	if (m_vehicleIsRightHandDrive)
	{
		// Reverse steering, since animation will be mirrored.
		steering = -steering;
	}
	m_animParameters.parameters[E_DriverParam_Steering] = steering;

	//
	// Speed
	//
	F32 speedMph = modelInput.m_fSpeed;
	if (m_currentGear < 0 && speedMph > 0.f)
	{
		speedMph = -speedMph;
	}
	m_animParameters.parameters[E_DriverParam_SpeedMph] = speedMph;
}

#ifndef CFG_SPEED

DECLARE_DEBUG_BOOL(gDebugDriverHide, "ActorDebug.Actors.Driver.Hide", false);

void C_DriverActor::UpdateFromDebugVariables()
{
	// If driving style has changed, then rebuild tree as it may need to have a different structure.
	if (m_drivingStyle != GetCreationParams()->GetDrivingStyle())
	{
		const AnimationTree::C_XtTree* xtTree = AnimationTree::C_XtTree::Find(GetXtAnimTreeName());
		if (xtTree != NULL)
		{
			BuildAnimTree(xtTree);
			m_drivingStyle = GetCreationParams()->GetDrivingStyle();
		}
		else
		{
			m_drivingStyle = -1;
		}
	}

	m_vehicleIsRightHandDrive = GetCreationParams()->IsRightHandDrive();
		
	// Live update of draw distance.
	m_pDriverModel->SetNearFarDistance(0.f, gDebugDriverFarCullDistance);

	if (gDebugDriverEnableOffsetOverride)
	{
		Maths::HMat43 newOffsetMatrix(Maths::HMat33(), Maths::HVec3(gDebugDriverOffsetOverride->vx, gDebugDriverOffsetOverride->vy, gDebugDriverOffsetOverride->vz));
		m_matrix = newOffsetMatrix * m_pVehicleModelInput->m_mMatrix;
	}

	if (gDebugDriverHide)
	{
		m_pDriverModel->DisableVisibilityFlags(E_ViewTypeFlag_All);
	}
}
#endif

void C_DriverActor::BuildAnimTree(const AnimationTree::C_XtTree* tree)
{
	MEMORY_TRACKER_PUSH_SUB("anim_tree");
	m_evaluationState->Flush();
	m_evaluationState->GetCommandList().Clear();

	// Add fixed parameters.
	S32 drivingStyleIndex = m_animTree.FindFixedParameter("DrivingStyle");
	if (drivingStyleIndex >= 0)
	{
		m_animTree.SetFixedParameter(drivingStyleIndex, static_cast<F32>(GetCreationParams()->GetDrivingStyle()));
	}

	m_animTree.SetGlobalMirrorEnabled(m_vehicleIsRightHandDrive);

	m_resourceBagHandle->LockListeners();

	tree->Build(m_animTree);

	m_resourceBagHandle->UnlockListeners();

	MEMORY_TRACKER_POP_SUB("anim_tree");
}


Maths::HVec3 C_DriverActor::GetDriverOffset() const
{
#ifndef CFG_SPEED
	return GetCreationParams()->GetOffset();
#else
	return m_driverOffset;
#endif
}

Maths::HMat43 C_DriverActor::GetVehicleWorldMatrix() const
{
	Maths::HMat43 m;
	if (m_pVehicleModelInput)
	{
		m = m_pVehicleModelInput->m_mMatrix;
	}
	return m;
}

void C_DriverActor::PhotoMoveDriver(const Maths::HVec3& offset)
{
	Maths::HMat43 mat = m_matrix;
	mat.AW( mat.AW() + offset );
	m_pDriverModel->SetWorldMatrix(mat);
}

void C_DriverActor::SetDynamicSHLightShaderParameters( const F32 additiveSHLightData[16] )
{
	if (!m_pVehicleLightSampler)
	{
		return;
	}
	const F32* lightSampleData = m_pVehicleLightSampler->GetLightSampleData().m_SampleData;
	F32 shData[16];
	for (U32 j = 0; j < 16; j++)
		shData[j] = lightSampleData[j] + additiveSHLightData[j];

	const C_DriverActorCreate* params = GetCreationParams();

	// Update SH lighting using sample from vehicle.
	for (U32 i = 0; i < E_ModelPart_Count; ++i)
	{
		const C_DriverModelPart& mp = m_modelParts[i];

		WorldRender::C_ModelInstance* pModelInstance = m_pDriverModel->GetModelInstance(mp.m_partIndex);
		if (pModelInstance != NULL)
		{
			if(m_pVehicleLightSampler && mp.m_shNodeParameter.index >= 0)
			{
				pModelInstance->SetParameter(mp.m_shNodeParameter, (void*)shData);
			}
		}
	}
}


//
// C_DriverActor::C_DriverModelPart
//

void C_DriverActor::C_DriverModelPart::AddToResourceBag(ResourceUtils::I_ResourceBag& resourceBag, const char* filename)
{
	m_tag = MakeResourceBagTag(filename);
	resourceBag.AddResource(m_tag, filename, GameResource::C_ModelResource::StaticGetResourceTypeId());
	m_primed = true;
}

void C_DriverActor::C_DriverModelPart::Create(ResourceUtils::I_ResourceBag& resourceBag)
{
	if (m_primed)
	{
		m_pResource = resourceBag.GetResource<GameResource::C_ModelResource>(m_tag);
	}
	else
	{
		m_pResource = 0;
	}
}

} // end Game


