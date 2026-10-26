#include "stdafx.h"
#include <Game/GameActors/AnimationTestActor.h>

#include <animation/common/ObjectTransform.h>
#include <animation/common/ObjectTransform.inl>
#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>
#include <animation/deferred/CommandProcessor.h>
#include <animation/cycle/Cycle.h>
#include <animation/pose/SkeletonPose.h>
#include <animation/pose/PoseFormat.h>
#include <animation/resource/SkeletonResource.h>
#include <animation/resource/CycleResource.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/skeleton/WindParams.h>
#include <animation/skeleton/WindGenerator.h>
#include <animation/task/EvaluationState.h>
#include <animation/util/DebugRender.h>

#include <game_resource/ModelResource.h>
#include <game_resource/ModelData.h>

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <Game/GameDef/PhysicsConstants.h>
#include <Game/GameDef/Materials.h>
#include <Game/GameControls/GameControls.h>

#include <shared_actors/XtActorBase.h>
#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>
#include <misc_shared/messaging/GameMessaging.h>

#include <Resource/Resource.h>

#include <Game/GameWorld/GameWorld.h>

#include <world_render/ModelInstance.h>
#include <world_render/EnvironmentSampler.h>
#include <scenegraph/SceneGraph.h>

#include <animated_model/AnimatedModelInstance.h>
#include <animated_model/TaskManager.h>

#include <gravity/narrowphase/Ray.h>
#include <gravity/sdk/CharacterController.h>
#include <gravity/sdk/ShapeDescriptors.h>

#include <camera2.h>
#include <camera2/stock/CharacterCamera.h>
#include <game/MGCam/CameraSystem.h>
#include <game/MGCam/CameraSwitcher.h>
#include <game/MGCam/PhysicsChecks.h>

#include <framework/application/ApplicationBase.h>
#include <input/GameInput.h>

#include <core/maths/hvec/new/IncludeAnimation.inl>

namespace Game
{

namespace Actors
{

Debug::Logger& gAnimationLog = Debug::GetLogger("mercurygame.animationtestactor");

Animation::C_WindParams		gAnimationWindParams;

// Actor Info

C_AnimationTestActor::C_AnimationTestActor()
  : m_cycleResource(NULL),
  m_skeletonResource(NULL),
  m_animatedModelInstance(NULL),
  m_evaluationState(NULL),
  m_pEnvSampler(NULL),
  m_body(NULL),
  m_characterController(NULL),
  m_camera(NULL),
  m_physicsChecks(NULL),
  m_isCharacter(false),
  m_spinAngle(0.f)
 {
	LOGINFO(gAnimationLog, "Constructed animation test actor" );
}

C_AnimationTestActor::~C_AnimationTestActor()
{
	LOGINFO(gAnimationLog, "Destroyed animation test actor" );
	
	DestroyCharacter();
	
	delete m_pEnvSampler;
	m_pEnvSampler = NULL;
	
	delete m_animatedModelInstance;
	m_animatedModelInstance = NULL;
	
	DestroyPose();
	DestroyModel();
	
}

bool C_AnimationTestActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
	
		case ActorMsg::E_ResourceMsg_AllLoaded:
		{
			// Resources all ready - Create should be called now?
			return true;
		}
	
		case ActorMsg::E_ResourceMsg_Loaded:
		{
			U32 resIndex = Messaging::MessageCast<ActorMsg::C_ResourceLoadedMsg>(pMessage)->GetValue();
			LOGINFO(gAnimationLog, "Resource loaded message %X", resIndex );
			return true;
		}

		case ActorMsg::E_ResourceMsg_Unloaded:
		{
			U32 resIndex = Messaging::MessageCast<ActorMsg::C_ResourceUnloadedMsg>(pMessage)->GetValue();
			LOGINFO(gAnimationLog, "Resource unloaded message %X", resIndex );
			return true;
		}

		case GameMessaging::E_GameMsg_Update:
			{
				F32 deltaTime = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();
				Update( deltaTime );
			}
			return true;
	}

	return C_GameActor::OnMessage(pMessage);
}

Debug::Bool gDebugForceBasePose("Game.Animation.ForceBasePose", false);
Debug::Bool gDebugEnableJointConstraints("Game.Animation.EnableJointConstraints", true);
Debug::Float gDebugIkWeight("Game.Animation.IkWeight", 1.f, Debug::Range(0.f, 1.f));
Debug::Float gDebugIkFadeTime("Game.Animation.IkFadeTime", 1.f, Debug::Range(0.f, 1.f));
Debug::Bool gDebugEnableLeftArmGrabIk("Game.Animation.EnableLeftArmGrabIk", false);
Debug::Bool gDebugEnableSteeringIk("Game.Animation.EnableSteeringIk", false);
Debug::Bool gDebugIkTargetFromJoint("Game.Animation.SetIkTargetFromJoint", false);
Debug::String gDebugIkTargetJoint("Game.Animation.IkTargetJoint", "face");
Debug::Vector gDebugIkTargetRotationOffset("Game.Animation.IkTargetRotationOffset", -90.f, 180.f, 45.f);
Debug::Vector gDebugIkTargetPositionOffset("Game.Animation.IkTargetPositionOffset", 0.f, 0.f, 0.2f);
Debug::Vector gDebugLeftArmTarget("Game.Animation.LeftArmIkTarget", -0.4f, 1.f, 0.1f);
Debug::String gDebugLeftArmPoleVector("Game.Animation.LeftArmPoleVector", "leftarm");
Debug::Vector gDebugLeftArmPoleVectorOffset("Game.Animation.LeftArmPoleVectorOffset", -0.4f, 0.1f, -0.1f);


Debug::Vector gDebugSteeringRefPos("Game.Animation.Steering.ReferencePosition", 0.f, 1.f, 0.4f);
Debug::Vector gDebugSteeringTargetPos("Game.Animation.Steering.TargetPosition", 0.f, 1.f, 0.4f);
Debug::Float gDebugSteeringRefTilt("Game.Animation.Steering.ReferenceTilt", 0.f, Debug::Range(-90.f, +90.f));
Debug::Float gDebugSteeringTargetTilt("Game.Animation.Steering.TargetTilt", 0.f, Debug::Range(-90.f, +90.f));
Debug::Float gDebugSteeringRefRadius("Game.Animation.Steering.ReferenceRadius", 0.1f, Debug::Range(0.f, 0.5f));
Debug::Float gDebugSteeringTargetRadius("Game.Animation.Steering.TargetRadius", 0.1f, Debug::Range(0.f, 0.5f));

Debug::Bool gDebugEnableFootIk("Game.Animation.EnableFootIk", false);
Debug::Vector gDebugLeftFootPoleVectorOffset("Game.Animation.LeftFootPoleVectorOffset", 0.f, -1.f, 1.f);
Debug::Vector gDebugRightFootPoleVectorOffset("Game.Animation.RightFootPoleVectorOffset", 0.f, -1.f, 1.f);
Debug::Vector gDebugFootPlaneAlignAxis("Game.Animation.FootPlaneAlignAxis", 0.f, 1.f, 0.f);
Debug::Vector gDebugFootSphereCentre("Game.Animation.FootSphereCentre", 0.f, 0.f, -0.03f);
Debug::Float gDebugFootSphereRadius("Game.Animation.FootRadius", 0.11f, Debug::Range(0.f, 0.5f));

Debug::Float gDebugTimeScale("Game.Animation.TimeScale", 1.f, Debug::Range(0.f, 1.f));

Debug::Bool gDebugEnableCharacterControl("Game.Animation.EnableCharacterControl", false, Debug::Persist());
Debug::Bool gDebugEnableIk("Game.Animation.EnableIK", false);

Debug::Bool  gDebugEnableWind("Game.Animation.Wind.Enable", true);
Debug::Float gDebugWindDirVariation("Game.Animation.Wind.DirectionVariation", 0.4f, Debug::Range(0.f, 1.f));
Debug::Float gDebugWindStrength("Game.Animation.Wind.Strength", 100.f, Debug::Range(0.f, 2000.f));
Debug::Float gDebugWindStrengthVariation("Game.Animation.Wind.StrengthVariation", 0.2f, Debug::Range(0.f, 1.f));
Debug::Float gDebugWindVariationRate("Game.Animation.Wind.VariationRate", 10.f, Debug::Range(0.f, 100.f));

Maths::Point3 MakePoint3(const Debug::Vector& v)
{
	return Maths::MakePoint3(v->vx, v->vy, v->vz);
}

Maths::Vector3 MakeVector3(const Debug::Vector& v)
{
	return Maths::MakeVector3(v->vx, v->vy, v->vz);
}


#ifndef CFG_GOLD
Debug::Bool gDebugRenderSkeleton("Game.Animation.ShowSkeleton", false);
Debug::Bool gDebugRenderBones("Game.Animation.ShowBones", true);
Debug::Bool gDebugRenderBoneTransforms("Game.Animation.ShowBoneTransforms", true);
Debug::Bool gDebugRenderBoneNames("Game.Animation.ShowBoneNames", false);
Debug::Bool gDebugShowIk("Game.Animation.ShowIk", false);
Debug::Bool gDebugShowTrajectory("Game.Animation.ShowTrajectory", false);
Debug::Bool gDebugShowWind("Game.Animation.ShowWind", false);
void C_AnimationTestActor::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	const Animation::C_Skeleton* skeleton = NULL;

	if (!m_evaluationState) return;
	
	if (m_skeletonResource != NULL)
	{
		skeleton = m_skeletonResource->GetSkeleton();
	
	}

	if (gDebugRenderSkeleton && skeleton != NULL )
	{
		DebugRenderSkeleton( debugRenderPtr, m_evaluationState->GetPose(), *skeleton, gDebugRenderBoneTransforms, gDebugRenderBones, gDebugRenderBoneNames );
	}
	
	if (gDebugShowIk)
	{
		m_leftArmIkChain.DebugRender(debugRenderPtr, m_worldMatrix);
		m_rightArmIkChain.DebugRender(debugRenderPtr, m_worldMatrix);
		m_leftLegIkChain.DebugRender(debugRenderPtr, m_worldMatrix);
		m_rightLegIkChain.DebugRender(debugRenderPtr, m_worldMatrix);
		
		//m_leftFootstepController.DebugRender(debugRenderPtr, m_worldMatrix);
		//m_rightFootstepController.DebugRender(debugRenderPtr, m_worldMatrix);
		
		if (gDebugEnableSteeringIk)
		{
			m_steeringController.DebugRender(debugRenderPtr, m_worldMatrix, 0.f);
		}
				
		m_walkController.DebugRender(debugRenderPtr, m_worldMatrix);
	}
	
	if (gDebugShowTrajectory)
	{
		U32 numSteps = 32;
		for (U32 i = 0; i < numSteps; ++i)
		{
			F32 t = i * m_cycleInstance.GetDuration() / (numSteps - 1.f);
			
			Animation::C_JointTransform traj = m_cycleInstance.GetCycle()->EvaluateTrajectoryTransform(t);
			
			Animation::DebugRenderJointMatrix(debugRenderPtr, traj.AsMat43() * m_worldMatrix, 0.1f);
		}
	}

	if (gDebugShowWind)
	{
		Animation::C_WindGenerator windGen;
		windGen.SetTime(m_evaluationState->GetSkeletonInstance().GetWindTime());
		windGen.Update(gAnimationWindParams, 0.f);

		Maths::Vector3 f = windGen.GenerateWindForce(0);

		debugRenderPtr->Draw3dArrow(m_worldMatrix.Translation(), Maths::HVec3(f), 0.4f / gAnimationWindParams.GetWindStrength(), 0.05f, Maths::RGBA(0xcc,0x00,0x00,0xff));
	}
}

#endif

bool C_AnimationTestActor::InitialiseImpl()
{
	if (!C_GameActor::InitialiseImpl())
		return false;

	const C_AnimationTestActorCreate * pCreate = (const C_AnimationTestActorCreate *)(GetCreationData().GetData());

	m_worldMatrix = pCreate->GetCreationMatrix();

	// create resource loader
	{
		Component::C_ComponentCreatorFactoryHelper<Components::C_ResourceListCreate> resourceListCreator;
		
		bool prime = true;
		
		resourceListCreator->m_List.Reserve(3);
		resourceListCreator->AddItem( pCreate->m_skeletonFilename, Animation::C_SkeletonResource::StaticGetResourceTypeId(), prime );
		resourceListCreator->AddItem( pCreate->m_modelFilename, GameResource::C_ModelResource::StaticGetResourceTypeId(), prime );
		resourceListCreator->AddItem( pCreate->m_cycleFilename, Animation::C_CycleResource::StaticGetResourceTypeId(), prime );
		
		if( InitialiseComponent( resourceListCreator.Get() ) == NULL )
			return false;
	}

	return true;
}

bool C_AnimationTestActor::CreateImpl()
{
	if (!C_GameActor::CreateImpl())
		return false;

	const C_AnimationTestActorCreate * pCreate = (const C_AnimationTestActorCreate *)(GetCreationData().GetData());
	
	// Get resources, which have been primed and are now ready for use.
	const Components::I_ResourceLoader* resLoader = QueryInterface<Components::I_ResourceLoader>();	
	m_modelResource = resLoader->GetResource<GameResource::C_ModelResource>( E_ResourceIndex_Model );
	m_skeletonResource = resLoader->GetResource<Animation::C_SkeletonResource>( E_ResourceIndex_Skeleton );
	m_cycleResource = resLoader->GetResource<Animation::C_CycleResource>( E_ResourceIndex_Cycle );

	if (m_cycleResource != NULL)
	{
		// Create pose from skeleton, adding additional fields from the cycle.
		CreatePose(m_cycleResource->GetCycle());

		m_cycleInstance.Create( m_cycleResource->GetCycle(), &m_evaluationState->GetPose() );
	}
	else
	{
		CreatePose(NULL);
		m_cycleInstance.Clear();
	}

	delete m_pEnvSampler;
	m_pEnvSampler = new WorldRender::C_EnvironmentSampler();
	m_pEnvSampler->Initialise( pCreate->m_pWorldRenderScene );
	
	if (m_modelResource)
	{
		CreateModel();
	}
	
	if (gDebugEnableCharacterControl)
	{
		CreateCharacter();
	}
	
	AddSubscriber( GameMessaging::E_GameMsg_Update );
	
	m_cycleTime = 0.f;

	return true;
}


void C_AnimationTestActor::CreateModel()
{
	S32 modelIndex = 0;
	if (m_modelResource->GetModelData()->GetModelCount() > 1)
	{
		modelIndex = 1;
	}
	
	const GameResource::C_Model* model = m_modelResource->GetModelData()->GetModel(modelIndex);
	
	m_animatedModelInstance = new AnimatedModel::C_AnimatedModelInstance(m_modelResource, modelIndex, GetSceneGraph(), m_poseFormat);
	//m_animatedModelInstance->SetBoundingSphereRadius(8.f);
		
	SceneGraph::Insert(GetSceneGraph(), m_animatedModelInstance->GetModelInstance());
}

void C_AnimationTestActor::DestroyModel()
{
	if (m_animatedModelInstance != NULL)
	{
		SceneGraph::Remove(GetSceneGraph(), m_animatedModelInstance->GetModelInstance());
		delete m_animatedModelInstance;
		m_animatedModelInstance = NULL;
	}
}

void C_AnimationTestActor::CreatePose( const Animation::C_Cycle* cycle )
{
	m_poseFormat.Clear();
	
	
	const Animation::C_Skeleton* skeleton = NULL;
	
	if (m_skeletonResource != NULL)
	{
		skeleton = m_skeletonResource->GetSkeleton();
	}
	
	delete m_evaluationState;
	m_evaluationState = Animation::C_EvaluationState::New(32);
	
	// Add fields for skeleton joints.
	m_poseFormat.SetSkeleton( skeleton );
	
	// Include additional fields stored in the cycle.
	if (cycle != NULL)
	{
		m_poseFormat.AddCycleFields(cycle);
	}

	// Create the pose.	
	m_evaluationState->GetPose().Create( m_poseFormat );
	m_evaluationState->GetPoseMatrixCache().Create( m_evaluationState->GetPose() );
		
	// Allocate a buffer to store the skeleton's model space joint transforms in.
	U32 numJoints = 0;
	if (skeleton != NULL)
	{
		numJoints = skeleton->GetNumJoints();
		m_evaluationState->GetSkeletonInstance().Create(*skeleton);
		
		m_leftArmIkChain.Create(*skeleton, "leftarm", "leftforearm", "lefthand", "l_elbow_ikaxis");
		m_rightArmIkChain.Create(*skeleton, "rightarm", "rightforearm", "righthand", "r_elbow_ikaxis");
		m_leftLegIkChain.Create(*skeleton, "leftupleg", "leftleg", "leftfoot", "l_knee_ikaxis");
		m_rightLegIkChain.Create(*skeleton, "rightupleg", "rightleg", "rightfoot", "r_knee_ikaxis");
		
		m_leftArmIkChain.SetNumTargets(E_ArmIkTarget_Count);
		m_rightArmIkChain.SetNumTargets(E_ArmIkTarget_Count);
		m_leftLegIkChain.SetNumTargets(E_LegIkTarget_Count);
		m_rightLegIkChain.SetNumTargets(E_LegIkTarget_Count);
		
		//Animation::C_FootstepEvents leftFootstepEvents;
		//Animation::C_FootstepEvents rightFootstepEvents;
		
		if (cycle != NULL)
		{
			// Extract gait information.
			S16 footJointIndices[2];
			footJointIndices[0] = skeleton->FindJointByName("lefttoebase");
			footJointIndices[1] = skeleton->FindJointByName("righttoebase");
			m_walkGait.CreateFromCycle(footJointIndices, 2, *skeleton, *cycle);
			//leftFootstepEvents.CreateFromCycle(m_leftLegIkChain, *skeleton, *cycle);
			//rightFootstepEvents.CreateFromCycle(m_rightLegIkChain, *skeleton, *cycle);
		}
			
		//m_leftFootstepController.SetFootstepEvents(leftFootstepEvents);
		//m_rightFootstepController.SetFootstepEvents(rightFootstepEvents);
		
		m_walkController.Clear();
		
		// lefttoebase, leftheelpoint, leftfoot
		m_walkController.AddLeg(m_leftLegIkChain, E_LegIkTarget_FootPlant, skeleton->FindJointByName("leftfoot"), 0.f);
		m_walkController.AddLeg(m_rightLegIkChain, E_LegIkTarget_FootPlant, skeleton->FindJointByName("rightfoot"), 0.f);
		
		m_steeringController.Clear();
		m_steeringController.AddArm(m_leftArmIkChain);
		m_steeringController.AddArm(m_rightArmIkChain);
	}
	else
	{
		m_evaluationState->GetSkeletonInstance().Destroy();
	}
	m_modelJointTransforms.Resize(numJoints);
}

void C_AnimationTestActor::DestroyPose()
{
	delete m_evaluationState;
	m_evaluationState = NULL;
}

const F32 kCharacterCapsuleHeight = 0.5f;
const F32 kCharacterCapsuleRadius = 0.4f;

void C_AnimationTestActor::CreateCharacter()
{	
	//
	// Create physics body and character controller.
	//
	SharedPhysics::C_Deactivator deactivator;
	deactivator.m_ActivateThresholdSq = 0.0f;//0.005f;
	deactivator.m_SleepThresholdSq = 0.0f;//0.001f;
	Maths::HF32 height(kCharacterCapsuleHeight);
	Maths::HF32 radius(kCharacterCapsuleRadius);
	Maths::HF32 rayHeight(0.4f);
	Maths::HF32 rayRadius(0.3f);
	Maths::HF32 zero(0.0f);
	Maths::HF32 mass(40.0f);
	m_body = GetPhysicsScene()->CreateBody();
	m_body->SetExternalConstants(GetPhysicsExternalConstants());
	Gravity::C_CharacterControllerDesc controllerDesc;
	controllerDesc.m_CapsuleHeight = height;
	controllerDesc.m_CapsuleRadius = radius;
	controllerDesc.m_RayHeight = Maths::Max(rayHeight - rayRadius, zero);
	controllerDesc.m_RayRadius = rayRadius;
	controllerDesc.m_MaxCosAngle = Maths::HF32(0.5f); // cos 60
	controllerDesc.m_MinPen = rayHeight;
	controllerDesc.m_FeetMaterial = 0;
	Gravity::C_BodyHelper bodyHelper(&controllerDesc);
	
	Maths::HMat33 orient( Maths::AxisY( Maths::C_Normal( 0.0f, 1.0f, 0.0f) ), Maths::AxisZ( Maths::C_Normal( m_worldMatrix.AZ() ) ));

	bodyHelper.CreateBody(mass, m_worldMatrix.AW(), orient, m_body, &PhysicsConstants::kCharacterCollisionFilter, E_PhysicsMaterial_CharacterController, E_EffectMaterial_Default);
	m_body->SetDeactivator(deactivator);
	m_body->Slop( 0.00000001f ); //PhysicsConstants::kDefaultSlop );
	m_body->Baumgarte( 2.0f );//PhysicsConstants::kDefaultBaumgarte ); // 2.0f
	GetPhysicsScene()->AddBody( m_body );

	m_characterController = new Gravity::C_CharacterController(m_body, Maths::HF32(10.f), Maths::HF32(500.f));
	GetPhysicsScene()->AddCharacterController(m_characterController);
	
	//
	// Create camera.
	//
	m_camera = new Cam2::C_CharacterCamera;
	m_physicsChecks = new MGCam::C_PhysicsChecks(GetPhysicsScene(), 0);
	m_camera->SetPhysics(m_physicsChecks);
	
	MGCam::System().GameFeedShow(m_camera);
	MGCam::Switcher().SetType(MGCam::E_DebugCam_GameFeed);
	
	m_isCharacter = true;
}

void C_AnimationTestActor::DestroyCharacter()
{
	//
	// Destroy physics objects.
	//
	if (m_characterController != NULL)
	{
		GetPhysicsScene()->RemoveCharacterController(m_characterController);
		delete m_characterController;
		m_characterController = NULL;
	}
	
	if (m_body != NULL)
	{
		GetPhysicsScene()->RemoveBody(m_body);
		//GetPhysicsScene()->ReleaseBody(m_body);
		m_body = NULL;
	}
	
	//
	// Destroy camera objects.
	//
	if (m_camera != NULL)
	{
		MGCam::System().GameFeedRemove(m_camera); // Make sure not being shown
		delete m_camera;
		m_camera = NULL;
	}
	
	delete m_physicsChecks;
	m_physicsChecks = NULL;
	
	m_isCharacter = false;
}

//
// Collision interface for Animation::C_WalkController
//
class C_GravityGroundCollision: public Animation::I_GroundCollision
{
public:
	C_GravityGroundCollision(Gravity::C_Scene* scene):
		m_scene(scene)
	{
	}

	virtual bool TraceRay(const Maths::Point3& start, const Maths::Point3& end, Maths::Point3* outContactPoint, Maths::Normal3* outContactNormal)
	{
		Collision::C_CollisionFilter collisionFilter = PhysicsConstants::kCharacterCollisionFilter;
		Collision::C_Ray ray;
		ray.InitialiseRay(
			Maths::HVec3(start),
			Maths::HVec3(end),
			&collisionFilter
		);
		
		ray.SetMaxCosAngle(-0.3f);

		m_scene->DoRayTrace( ray, m_scene->m_Stack, false );
		
		bool hit = false;
			
		if(ray.m_result.Hit())
		{
			hit = true;
			if (outContactPoint)
			{
				*outContactPoint = Maths::Point3(ray.m_result.HitPoint());
			}
			if (outContactNormal)
			{
				*outContactNormal = Maths::Normal3(ray.m_result.Normal());
			}
		}			
		return hit;
	}
	
private:
	Gravity::C_Scene* m_scene;
};


Debug::Bool gDebugSpin("Game.Animation.EnableSpin", false);
Debug::Float gDebugSpinRate("Game.Animation.SpinRate", 20.f, Debug::Range(0.f, 360.f));
Debug::Float gDebugSpinAngle("Game.Animation.SpinAngle", -1.f, Debug::Range(0.f, 360.f));

void C_AnimationTestActor::Update( F32 deltaTime )
{
	const Animation::C_Skeleton* skeleton = GetSkeleton();
	
	Animation::C_CommandList& commandList = m_evaluationState->GetCommandList();

	commandList.Clear();
	
	if (!gDebugForceBasePose)
	{
		if (IsCharacter())
		{
			AnimateCharacter(commandList, deltaTime);
		}
		else
		{
			AnimateObject(commandList, deltaTime);
		}
	}

	if (gDebugSpin)
	{
		if (gDebugSpinAngle >= 0.f)
		{
			m_spinAngle = Maths::DegToRad((F32)gDebugSpinAngle);
		}
		else
		{
			m_spinAngle += deltaTime * Maths::DegToRad((F32)gDebugSpinRate);
		}
		m_worldMatrix = Maths::HMat43(Maths::HMat33(Maths::RotY(m_spinAngle)), m_worldMatrix.Translation());
	}
	
	commandList.SetModelWorldMatrix(m_worldMatrix);
	
	
	Animation::C_Pose& pose = m_evaluationState->GetPose();
	
	F32 ikDeltaTime = deltaTime * gDebugTimeScale;
	
	if (gDebugEnableIk)
	{
		Maths::Point3 poleVectorOffset = MakePoint3(gDebugLeftArmPoleVectorOffset);
		m_leftArmIkChain.SetPoleVector(pose.FindJointTransformByName(gDebugLeftArmPoleVector.Value()), poleVectorOffset);

		if (gDebugEnableLeftArmGrabIk)
		{
			Maths::Point3 targetPos = MakePoint3(gDebugLeftArmTarget);
					
			m_leftArmIkChain.SetTargetPosition(E_ArmIkTarget_Grab, targetPos);
			m_leftArmIkChain.SetTargetWeight(E_ArmIkTarget_Grab, gDebugIkWeight);
			
			if (gDebugIkTargetFromJoint && skeleton != NULL)
			{
				Maths::Vector3 targetPosOffset = MakeVector3(gDebugIkTargetPositionOffset);
				Maths::Vector3 targetRotOffset = MakeVector3(gDebugIkTargetRotationOffset);
				
				targetRotOffset *= Maths::Const<Maths::VF32>::Pi() / Maths::MakeVF32(180.f);
				
				Maths::Mat33 rotationMatrix = Maths::MakeMat33(Maths::MakeEulerXYZ(targetRotOffset));
				
				Maths::Quaternion targetRotOffsetQ = Maths::MakeUnitQuaternion(rotationMatrix);
			
				commandList.SetIkTargetTransformFromJoint(m_leftArmIkChain, E_ArmIkTarget_Grab, skeleton->FindJointByName(gDebugIkTargetJoint), targetRotOffsetQ, Maths::MakePoint3(targetPosOffset));
			}
		}
		else
		{
			m_leftArmIkChain.SetTargetWeight(E_ArmIkTarget_Grab, 0.f);
		}
		
		if (gDebugEnableSteeringIk)
		{
			Maths::Mat43 refWheel = Maths::MakeMat43(
				Maths::MakeMat33_AxisAngle(Maths::Const<Maths::Normal3>::AxisX(), Maths::MakeVF32(Maths::DegToRad(gDebugSteeringRefTilt.Value()))),
				Maths::MakeVector3(gDebugSteeringRefPos->vx, gDebugSteeringRefPos->vy, gDebugSteeringRefPos->vz)
			);

			Maths::Mat43 targetWheel = Maths::MakeMat43(
				Maths::MakeMat33_AxisAngle(Maths::Const<Maths::Normal3>::AxisX(), Maths::MakeVF32(Maths::DegToRad(gDebugSteeringTargetTilt.Value()))),
				Maths::MakeVector3(gDebugSteeringTargetPos->vx, gDebugSteeringTargetPos->vy, gDebugSteeringTargetPos->vz)
			);
			
			m_steeringController.SetReferenceWheel(refWheel, gDebugSteeringRefRadius);
			m_steeringController.SetTargetWheel(targetWheel, gDebugSteeringTargetRadius);

		
			m_steeringController.Update();
		
			m_leftArmIkChain.SetTargetWeight(E_ArmIkTarget_Steering, gDebugIkWeight);
		}
		else
		{
			m_leftArmIkChain.SetTargetWeight(E_ArmIkTarget_Steering, 0.f);
		}
				
		commandList.EvaluateIkChain(m_leftArmIkChain);	
		
		if (gDebugEnableFootIk)
		{
			m_leftLegIkChain.SetPoleVector(m_leftLegIkChain.GetRootJointIndex(), MakePoint3(gDebugLeftFootPoleVectorOffset));
			m_rightLegIkChain.SetPoleVector(m_rightLegIkChain.GetRootJointIndex(), MakePoint3(gDebugRightFootPoleVectorOffset));
		
			if (GetSkeleton() != NULL /* && poseUpdatedAtLeastOnce */)
			{
				// Update leg positions. Instead of this, just pass pose as parameter to Update()?
				m_walkController.UpdateFootPositions(pose, GetSkeleton());
			}
			
			C_GravityGroundCollision groundCollision(GetPhysicsScene());
					
			m_walkController.Update(
				deltaTime,
				//m_characterController->GetRelativeLinearVelocity(),	// relative to model world matrix?
				(m_cycleInstance.GetFinalTrajectoryTransform().GetTranslation() - m_cycleInstance.GetFirstTrajectoryTransform().GetTranslation()) * m_worldMatrix,
				m_worldMatrix,
				&m_walkGait,	// No gait information for the time being.
				m_cycleSpeed,
				&groundCollision,
				m_characterController->GetOnGroundFlags(),
				gDebugIkWeight
			);
			
			if (GetSkeleton() != NULL)
			{
				S32 hipJointIndex = GetSkeleton()->FindJointByName("hips");
				commandList.TranslateJointInModelSpace(hipJointIndex, m_walkController.GetHipOffset());
			}
					
			//
			// Add leg IK evaluation to command list.
			//
			commandList.EvaluateIkChain(m_leftLegIkChain);
			commandList.EvaluateIkChain(m_rightLegIkChain);
		}
	}
	
	if (skeleton != NULL && gDebugEnableJointConstraints)
	{
		m_evaluationState->GetSkeletonInstance().UpdateDynamicJoints( deltaTime );

		// Apply wind.
		if (gDebugEnableWind)
		{
			gAnimationWindParams.SetWindDirectionVariation(gDebugWindDirVariation);
			gAnimationWindParams.SetWindStrength(gDebugWindStrength);
			gAnimationWindParams.SetWindStrengthVariation(gDebugWindStrengthVariation);
			gAnimationWindParams.SetWindVariationRate(gDebugWindVariationRate);

			commandList.ApplyWindForces(gAnimationWindParams, deltaTime);
		}

		commandList.ApplyConstraints(m_evaluationState->GetSkeletonInstance(), m_worldMatrix);
	}

	

	// Clear game_root so character stays rooted to the spot.
	commandList.SetRootToIdentity();
	
	AnimatedModel::E_Sync sync = AnimatedModel::E_Sync_PreRender;
	
	if (m_pEnvSampler != NULL)
	{
		m_pEnvSampler->SetPosition(m_worldMatrix.Translation());
		m_pEnvSampler->Update();
		m_pEnvSampler->ApplyToModelInstance(m_animatedModelInstance->GetModelInstance());
	}
		
	m_evaluationState->ScheduleEvaluationTask();
	m_animatedModelInstance->ScheduleUpdateTask(m_worldMatrix, m_evaluationState->GetPoseNonBlocking(), m_evaluationState->GetPoseMatrixCacheNonBlocking());

#if 0	
	AnimatedModel::TaskManager().ScheduleAnimateAndUpdateTask(
		AnimatedModel::E_Sync_PreRender,	// unless we want to debug render....
		*m_animatedModelInstance,
		m_worldMatrix,
		pose,
		m_poseMatrixCache,
		m_commandList,
		skeleton
	);
#endif
}

void C_AnimationTestActor::DebugRenderSkeleton( DbUtil::I_DebugRenderPtr debugRender, const Animation::C_Pose& pose, const Animation::C_Skeleton& skeleton,
	bool showBoneTransforms, bool showBones, bool showBoneNames ) const
{
	U32 numJoints = skeleton.GetNumJoints();
	
	if ( numJoints == 0 )
	{
		return;
	}
	
#ifndef CFG_GOLD
	debugRender->SetDepthEnable(false);
	debugRender->Draw3dMatrix(m_worldMatrix);
	Animation::DebugRenderPoseTransforms(debugRender, pose, &skeleton, m_worldMatrix, showBoneTransforms, showBoneNames, showBones);
	debugRender->SetDepthEnable(true);
#endif
}

bool C_AnimationTestActor::IsCharacter() const
{
	return m_isCharacter;
}

const Animation::C_Skeleton* C_AnimationTestActor::GetSkeleton() const
{
	const Animation::C_Skeleton* s = NULL;
	if (m_skeletonResource != NULL)
	{
		s = m_skeletonResource->GetSkeleton();
	}
	return s;
}

void C_AnimationTestActor::AnimateObject(Animation::C_CommandList& commandList, F32 deltaTime)
{
	m_cycleSpeed = 1.f;
	m_cycleTime += deltaTime * gDebugTimeScale;;
	commandList.BlendCycle( Animation::E_BlendMode_Accumulate, &m_cycleInstance, m_cycleTime, 1.f );
}

DECLARE_DEBUG_FLOAT(gTargetSpeed, "Game.Animation.Character.TargetSpeed", 2.f, Debug::Range(0.f, 8.f));
DECLARE_DEBUG_FLOAT(gTurnSpeed, "Game.Animation.Character.TurnSpeed", 10.f, Debug::Range(0.f, 20.f));

void C_AnimationTestActor::AnimateCharacter(Animation::C_CommandList& commandList, F32 deltaTime)
{
	assert(m_body);

	// Adjust cycle speed according to character velocity.
	Maths::Vector3 characterVelocity = m_characterController->GetRelativeLinearVelocity();
	Maths::Vector3 cycleVelocity( m_cycleInstance.GetLinearVelocity() );
	
	bool onGround = m_characterController->GetOnGroundFlags();
	
	if (onGround)
	{
		Maths::Vector3 flatVelocity = characterVelocity;
		Maths::ZeroY(&flatVelocity);
		F32 characterSpeed = Maths::GetF32(Maths::Length(flatVelocity));
		F32 cycleSpeed = Maths::GetF32(Maths::Length(cycleVelocity));
		m_cycleSpeed = characterSpeed / cycleSpeed;
	}
	else
	{
		m_cycleSpeed = 0.f;
	}
			
	m_cycleTime += deltaTime * m_cycleSpeed * gDebugTimeScale.Value();
	commandList.BlendCycle( Animation::E_BlendMode_Accumulate, &m_cycleInstance, m_cycleTime, 1.f );
	
	m_worldMatrix = m_body->ElementMatrix();
	m_worldMatrix.Translation(m_worldMatrix.Translation() - Maths::HVecY(kCharacterCapsuleHeight + kCharacterCapsuleRadius));

	// Update character controller
	{
		bool inputActive = MGCam::System().DebugFeed().PadUsed() != 0;

		const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
		Maths::Mat43 cameraMatrix = view.DebugFeed().Mat43();

		Maths::Vector3 xDir = Maths::RowX(cameraMatrix);
		Maths::Vector3 zDir = Maths::RowZ(cameraMatrix);
		Maths::ZeroY(&xDir);
		Maths::ZeroY(&zDir);
		
		Maths::Normal3 desiredXDir = Maths::Normalise(xDir);
		Maths::Normal3 desiredYDir = Maths::Normalise(zDir);

		F32 padX = 0.f, padY = 0.f;
		if (inputActive)
		{
			padX = Input::GetGameInput(0)->GetAxisSignedValue( Game::E_Axis_MoveX );
			padY = Input::GetGameInput(0)->GetAxisSignedValue( Game::E_Axis_MoveY );
		}
		
		Maths::Vector3 currentDir = m_worldMatrix.AZ();
		Maths::Vector3 desiredDirection = desiredXDir * Maths::MakeVF32(padX) + desiredYDir * Maths::MakeVF32(padY);
		Maths::VF32 desiredSpeed = Maths::Length(desiredDirection);
		
		if (desiredSpeed < Maths::MakeVF32(0.001f))
		{
			desiredDirection = currentDir;
		}
		else
		{
			desiredDirection = Maths::MakeVector3(Maths::Normalise(desiredDirection));
		}
		
		Maths::Vector3 targetVelocity = desiredDirection * desiredSpeed * Maths::MakeVF32(gTargetSpeed);
		Maths::Vector3 targetAngularVelocity = Maths::MakeVF32(gTurnSpeed) * (desiredDirection ^ currentDir);
		
		m_characterController->SetTargetVelocity( Maths::HVec3(targetVelocity), Maths::HVec3(targetAngularVelocity) );

		if( inputActive && Input::GetGameInput(0)->GetButtonPressedDebounced( E_Button_Jump ) )
		{
			m_characterController->Jump(Maths::HF32(5.f));
		}
	}

	// update cam
	{
		// Chase camera:
		Cam2::C_CamObj object;
		object.position = m_worldMatrix.AW();
		object.orient = Maths::C_Quaternion( Maths::HMat33(m_body->ElementMatrix()) );

		m_camera->SetObject(object);

		F32 sx = 0.f, sy = 0.f;
		if (MGCam::System().DebugFeed().PadUsed() != 0)
		{
			sx = Input::GetGameInput(0)->GetAxisSignedValue( E_Axis_CameraLookX );
			sy = Input::GetGameInput(0)->GetAxisSignedValue( E_Axis_CameraLookY );
		}
		m_camera->SetLookAround(sx * 2.f, sy * 2.f);

		m_camera->Update(deltaTime);
	}

	commandList.BlendCycle( Animation::E_BlendMode_Accumulate, &m_cycleInstance, m_cycleTime, 1.f );
}

} // namespace Actors

} // namespace Game


