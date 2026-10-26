#include "stdafx.h"
#include "PreviewState.h"
#include <animation/tools/CompressionParameters.h>
#include <animation/tools/CycleBuilder.h>
#include <animation/tools/Scene.h>
#include <animation/cycle/Cycle.h>
//#include <animation/cycle/CycleDeserialise.inl>
#include <animation/skeleton/JointWeights.h>
//#include <animation/pose/PoseLayout.h>
#include <animation/common/ObjectTransform.h>
#include <animation/common/ObjectTransform.inl>
#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>
#include <animation/pose/PoseFormat.h>
#include <animation/pose/SkeletonPose.h>
#include <system/misc/StringUtil.h>
#include <system/serialise/C_MemoryStream.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <btl/containers/vector.h>
#include "AnimationGraph.h"
#include "GlobalSignals.h"
#include "Db.h"

#define BUILD_ADDITIVE_CYCLES

template < typename T >
bool Serialise2ToFile( const Util::C_String& filename, const T& object )
{
	Sys::I_File* file = Sys::C_FileSystem::Instance().Open( filename, Sys::E_FileAccess_Write );
	
	bool ok = false;

	if (file)
	{
		file->SetEndianSwapFlag( false );
		
		ok = object.Serialise2( *file );
	}
	delete file;
	
	return ok;
}

bool SaveAmaxCarAnimation(
	const Util::C_String& filename,
	const AnimationTools::C_CycleBuilder& cycleBuilder,
	const Animation::C_Skeleton& skeleton,
	const char* jointName )
{
	Sys::I_File* file = Sys::C_FileSystem::Instance().Open( filename, Sys::E_FileAccess_Write );
	
	bool ok = false;

	if (file)
	{
		file->SetEndianSwapFlag( true );
		
		Sys::BinarySerialiser< Ut::NativeToBigEndian > out( *file );
		
		U32 numFrames = cycleBuilder.GetDurationFrames();
		F32 frameRate = cycleBuilder.GetFrameRate();
		
		out << U32( numFrames );
		out << U32( frameRate );
		
		S32 rootJointIndex = skeleton.FindJointByName( jointName );
		
		if ( rootJointIndex != -1 )
		{
			for ( U32 i = 0; i < numFrames; ++i )
			{
				const F32 time = i / frameRate;
				Animation::C_JointTransform transform = cycleBuilder.GetLocalJointTransform( rootJointIndex, time, AnimationTools::C_CycleBuilder::E_Source_Original );
				
				// Flip quat?
				
				out << transform;
			}
		}
		
		ok = true;
				
		//ok = object.Serialise2( *file );
	}
	delete file;
	
	return ok;
}

//
// C_ClipPreviewState
//

C_ClipPreviewState* gClipPreviewState = 0;

C_ClipPreviewState::C_ClipPreviewState( C_XtDatabase& db, C_AnimationGraphEditor& animGraphEditor ):
	m_db( db ),
	m_animGraphEditor( animGraphEditor ),
	m_globalSkeleton(NULL),
	m_isValid(false),
	m_loading(false)
{
	Reset();
	m_selectedJointIndex = -1;
	
	showSkeleton = true;
	showSkinnedMesh = true;
	m_cameraMode = E_CameraMode_OrbitCharacter;
	paused = false;
	showJointNames= false;
	showOnlySelectedJointName = false;
	hideUnselectedJoints = false;
	showConstraints = true;
	showWireframe = false;
	hudShowCommandList = false;
	hudShowCustomCurves = false;
	hudShowSelectedJointTransform = false;
	poseInWorldSpace = false;
	freezeRoot = false;
	absoluteRootMotion = false;
	enableCommandListPushPopElimination = true;
	commandListOk = true;
	showWorldAxes = false;
	requestReloadAll = false;

	centre = Maths::Const<Maths::Point3>::Zero();
	
	playbackSpeed = 1.f;
	m_smoothedDeltaTime = 1.f;
	m_globalTime = 0;
	
	AnimationTools::C_SceneConfig sceneConfig;

	// XSI things:
	//m_skeletonParameters.AddExcludedNodeNamePattern( "eff*" );
	//m_skeletonParameters.EnableRemovalOfRootNodeNamePrefixes( true );
	
	// also *_end, *_target, *_null
	sceneConfig.excludedNodeNames.Add( "*_cust" );
	sceneConfig.excludedNodeNames.Add( "*_null" );

	sceneConfig.dynamicAimJointNames.Add( "*_dynamic" );
	sceneConfig.dynamicAimJointNames.Add( "*_aimdynamic" );
	sceneConfig.dynamicSliderJointNames.Add( "*_slidedynamic" );
	//sceneConfig.excludedNodeNames.Add( "hip_solver" );
	
	
	sceneConfig.excludedAttributeNames.Add("liw");
	sceneConfig.excludedAttributeNames.Add("DefaultAttributeIndex");
	sceneConfig.excludedAttributeNames.Add("LimbLength");
	
	sceneConfig.rootNodeNames.Add( "skeleton_base" );
	sceneConfig.rootNodeNames.Add( "game_root" );
	sceneConfig.rootNodeNames.Add( "BASE_SKELETON*" );

	sceneConfig.referenceNodeNames.Add( "referencemarker*" );
	
	// Ignore aftkroot
	//sceneConfig.excludedNodeNames.Add( "aftkroot" );
	
	// Ignore Maya junk
	sceneConfig.excludedNodeNames.Add( "cubecompass" );
	sceneConfig.excludedNodeNames.Add( "universalmanip" );
	sceneConfig.excludedNodeNames.Add( "manipulator*" );
	
	sceneConfig.helperNodeNames.Add("l_ammo");
	sceneConfig.helperNodeNames.Add("r_ammo");
	sceneConfig.helperNodeNames.Add("l_gun");
	sceneConfig.helperNodeNames.Add("r_gun");
	sceneConfig.helperNodeNames.Add("head_point");
	
	sceneConfig.flipX = true;
	sceneConfig.flipY = false;
	sceneConfig.flipZ = false;

	sceneConfig.highFidelityRootNodeNames.Add("*facial_root");
	sceneConfig.highFidelityRootNodeNames.Add("*camera*");

	sceneConfig.worldOrigin.x = 0.f;
	sceneConfig.worldOrigin.y = 0.f;
	sceneConfig.worldOrigin.z = 0.f;

	sceneConfig.worldOriginNodeNames.Add("NEW_WORLD_ORIGIN");
	
	C_ResourceCache::Instance().SetSceneConfig( sceneConfig );
	
	//
	// Connect slots to signals.
	//
	
	C_GlobalSignals::Instance().jointSelected.connect(this, &C_ClipPreviewState::SetSelectedJointIndex);
	C_GlobalSignals::Instance().timelineUpdated.connect(this, &C_ClipPreviewState::SetTimelineValue);
	C_GlobalSignals::Instance().cameraChanged.connect(this, &C_ClipPreviewState::SetCameraMode);
}


C_ClipPreviewState::~C_ClipPreviewState()
{
}

void C_ClipPreviewState::Reset()
{
	rootTransform.SetIdentity();
	m_animTime = 0.f;
}

static const U32 kMotionRootJointIndex = 0;

//Debug::Float gPlaybackSpeed("AnimationPreview.PlaybackSpeed", 1.f, Debug::Range(0.f, 1.f));
Debug::Bool gPausePlayback("AnimationPreview.PausePlayback", false);
Debug::Bool gEnableDrivenJoints("AnimationPreview.EnableDrivenJoints", true);

namespace
{
	struct C_DynamicJointInfo
	{
		const char* name;
		const char* target;
		F32 mass;
		F32 airDamping;
		F32 pitchSpring;
		F32 pitchDamping;
		F32 maxPitch;
		F32 minPitch;
		F32 headingSpring;
		F32 headingDamping;
		F32 maxHeading;
		F32 minHeading;
	};
	
#if 0
	void InitDynamicJoint(
		Animation::C_SpringJoint& dj,
		U32 index,
		const C_DynamicJointInfo* info,
		Animation::C_JointConstraintList& constraintList,
		const Animation::C_Skeleton& skeleton )
	{
		dj.Initialise( skeleton, Animation::C_StringId( info->name ), Animation::C_StringId( info->target ) );
		
		S16 targetJointIndex = dj.GetTargetJointIndex();
		
		dj.SetMass( info->mass );
		dj.SetLimits( -info->minHeading, -info->maxHeading, -info->minPitch, -info->maxPitch );

		constraintList.AddCustomConstraint(
			index,
			dj.GetJointIndex(),
			1,
			&targetJointIndex );
	}

	void InitDynamicJoint(
		Animation::C_DynamicJoint& dj,
		U32 index,
		const C_DynamicJointInfo* info,
		Animation::C_JointConstraintList& constraintList,
		const Animation::C_Skeleton& skeleton )
	{
#if 0
		dj.Initialise( skeleton, Animation::C_StringId( info->name ), Animation::C_StringId( info->target ) );
		dj.m_Mass = info->mass;
			
		if ( info->airDamping > 0.f )
		{
			dj.m_kAirDamping = Maths::Clamp( 1.f - info->airDamping, 0.f, 1.f );
		}
		dj.m_kPitchSpring = info->pitchSpring;
		dj.m_kPitchDamping = Maths::Clamp( 1.f - info->pitchDamping, 0.f, 1.f );
#if 1
		dj.m_kUpperPitchLimit = info->maxPitch;
		dj.m_kLowerPitchLimit = info->minPitch;
#else
		dj.m_kUpperPitchLimit = -info->minPitch;
		dj.m_kLowerPitchLimit = -info->maxPitch;
#endif			
		dj.m_kHeadingSpring = info->headingSpring;
		dj.m_kHeadingDamping = Maths::Clamp( 1.f - info->headingDamping, 0.f, 1.f );
#if 1			
		dj.m_kUpperHeadingLimit = info->maxHeading;
		dj.m_kLowerHeadingLimit = info->minHeading;
#else
		dj.m_kUpperHeadingLimit = -info->minHeading;
		dj.m_kLowerHeadingLimit = -info->maxHeading;
#endif

		constraintList.AddCustomConstraint(
			index,
			dj.m_JointIndex,
			1,
			&dj.m_TargetIndex );
#endif
	}

	void InitDynamicJoint(
		Animation::C_SliderJoint& dj,
		U32 index,
		const C_DynamicJointInfo* info,
		Animation::C_JointConstraintList& constraintList,
		const Animation::C_Skeleton& skeleton )
	{
		dj.Initialise( skeleton, Animation::C_StringId( info->name ), Maths::Const<Maths::Normal3>::AxisZ() );

		constraintList.AddCustomConstraint(
			index,
			dj.GetJointIndex(),
			0,
			NULL );
	}
#endif
}

void C_ClipPreviewState::InitialiseDynamicJoints()
{
	if ( m_globalSkeleton == NULL ) return;

	C_DynamicJointInfo kDynamicJointDefs[] =
	{
		// name,					target name,		mass, air,					| pitch							| heading
		
		// Renwick.
		{ "nightstick_dynamic",		"l_knee_ikaxis",				3.f, 0.01f,		0.01f, 10.f, 0.03f, -0.2f,		0.01f, 10.f, 2.f, -2.f			},
		{ "pack_1_dynamic",			"r_knee_ikaxis",				3.f, 0.001f,	0.01f, 10.f, -0.05f, -0.4f, 	0.01f, 10.f, -0.4f, -1.f		},
		{ "pack_2_dynamic",			"r_knee_ikaxis",				3.f, 0.001f,	0.01f, 10.f, 0.f, -0.3f,		0.01f, 10.f, 0.3f, -0.3f		},
		{ "necklace_01_dynamic",	"necklace_aim_null",			3.f, 0.0f,		2.f, 15.f, -0.1f, -0.5f,		2.f, 15.f, 0.2f, -0.2f			},
		{ "necklace_02_dynamic",	"necklace_aim_null",			2.f, 0.0f,		1.f, 10.f, -0.05f, -1.f,		1.f, 10.f, 0.4f, -0.4f			},
		{ "ring_dynamic",			"back3",						1.f, 0.0f,		0.f, 0.f, 0.f, 0.f,				0.f, 0.f, 2.5f, 0.f				},

		// Finn.
		{ "r_jacket_1_dynamic",		"r_aim_side_null",				6.f, 0.05f,		0.01f, 100.f, 0.1f, 0.f,		0.01f, 100.f, 0.025f, -0.025f		},
		{ "r_jacket_2_dynamic",		"r_jacket_2_null",				5.f, 0.05f,		0.01f, 100.f, 0.2f, 0.1f, 		0.01f, 100.f, 0.05f, -0.05f			},
		{ "r_jacket_3_dynamic",		"r_leg_side_null",				4.f, 0.05f,		0.01f, 100.f, 0.4f, 0.1f,		0.01f, 100.f, 0.3f, 0.1f			},
		{ "l_jacket_1_dynamic",		"l_aim_side_null",				6.f, 0.05f,		0.01f, 100.f, 0.1f, 0.f,		0.01f, 100.f, -0.025f, -0.025f		},
		{ "l_jacket_2_dynamic",		"l_jacket_2_null",				4.f, 0.05f,		0.01f, 100.f, 0.2f, 0.1f,		0.01f, 100.f, 0.05f, -0.05f			},
		{ "l_jacket_3_dynamic",		"l_leg_side_null",				2.f, 0.05f,		0.01f, 100.f, 0.4f, 0.1f,		0.01f, 100.f, -0.1f, -0.3f			},
		{ "jacket_back_1_dynamic",	"jacket_back_1_null",			2.f, 0.05f,		0.01f, 100.f, 0.35f, 0.05f,		0.01f, 100.f, 0.05f, -0.05f			},
		{ "jacket_back_2_dynamic",	"jacket_back_2_null",			2.f, 0.05f,		0.01f, 100.f, 0.7f, 0.1f,		0.01f, 100.f, 0.1f, -0.1f			},
		{ "r_jacket_back_flap_dynamic",	"r_jacket_flap_aim_null",	2.f, 0.05f,		0.01f, 100.f, 1.4f, 0.15f,		0.01f, 100.f, 0.1f, -0.325f			},
		{ "l_jacket_back_flap_dynamic",	"l_jacket_flap_aim_null",	2.f, 0.05f,		0.01f, 100.f, 1.4f, 0.15f,		0.01f, 100.f, 0.325f, -0.1f			},
		{ "l_jacket_front_1_dynamic",	"l_aim_front_side_driven",	1.f, 0.05f,		0.01f, 0.f, 0.3f, 0.f,		0.01f, 100.f, 0.05f, -0.1f			},
		{ "r_jacket_front_1_dynamic",	"r_aim_front_side_driven",	1.f, 0.05f,		0.01f, 0.f, 0.3f, 0.f,		0.01f, 100.f, 0.1f, -0.05f			},
		{ "l_jacket_front_2_dynamic",	"l_thighsolver_null",		1.f, 0.05f,		0.01f, 0.f, 0.25f, 0.1f,		0.01f, 100.f, 0.1f, -0.1f			},
		{ "r_jacket_front_2_dynamic",	"r_thighsolver_null",		1.f, 0.05f,		0.01f, 0.f, 0.25f, 0.1f,		0.01f, 100.f, 0.1f, -0.1f			},

		// Seager.
		{ "rope_dynamic",				"rope_aim_null",			3.f, 0.01f,		0.01f, 2.f, -0.1f, -2.f,	0.01f, 10000.f, 2.f, -2.f },
		{ "rope_2_dynamic",				"rope_aim_null",			2.f, 0.01f,		0.01f, 2.f, -0.2f, -10.f,	0.01f, 10000.f, 5.f, -5.f },


		{ 0 }
	};
	
	for ( C_DynamicJointInfo* info = kDynamicJointDefs; info->name; ++info )
	{
		Animation::C_StringId jointId = Animation::C_StringId( info->name );
		Animation::C_StringId targetJointId = Animation::C_StringId( info->target );
		
		S32 jointIndex = m_globalSkeleton->FindJointByName( jointId );
		S32 targetJointIndex = m_globalSkeleton->FindJointByName( targetJointId );
		
		S32 dynamicJointInstance = m_skeletonInstance.FindDynamicJoint(jointIndex);
		
		if ( dynamicJointInstance >= 0 && targetJointIndex >= 0 )
		{
			m_skeletonInstance.SetDynamicAimJointTarget( dynamicJointInstance, targetJointIndex );
			m_skeletonInstance.SetDynamicJointMass( dynamicJointInstance, info->mass );
			
			
			F32 minHeading = info->minHeading;
			F32 maxHeading = info->maxHeading;
			F32 minPitch = info->minPitch;
			F32 maxPitch = info->maxPitch;
			
			bool flipHeading = C_ResourceCache::Instance().GetSceneConfig().flipY;
			bool flipPitch = C_ResourceCache::Instance().GetSceneConfig().flipX;
			
			if (flipHeading)
			{
				minHeading = -minHeading;
				maxHeading = -maxHeading;
			}
			
			if (flipPitch)
			{
				minPitch = -minPitch;
				maxPitch = -maxPitch;
			}
			
			m_skeletonInstance.SetDynamicAimJointLimits( dynamicJointInstance, minPitch, maxPitch, minHeading, maxHeading );
		}		
	}
}

Debug::Vector gGravity( "AnimationPreview.Gravity", 0.f, -9.8f, 0.f );

void C_ClipPreviewState::UpdateDynamicJoints( F32 deltaTime )
{
#if 0
	const Maths::Vector3 gravity = Maths::MakeVector3( gGravity->vx, gGravity->vy, gGravity->vz );
	for ( S32 i = 0; i < m_dynamicJoints.Size(); ++i )
	{
		m_dynamicJoints[ i ].ApplyAcceleration( gravity  );
		// Maths::MakeVF32( m_dynamicJoints[ i ].m_Mass ) );
		m_dynamicJoints[ i ].Update( deltaTime );
	}
#endif
}

class C_AutoUpdateCamera
{
public:
	C_AutoUpdateCamera(C_ClipPreviewState& s, F32 dt):
		m_s(s),
		m_dt(dt)
	{}
	
	~C_AutoUpdateCamera()
	{
		m_s.UpdateCamera( m_dt );
	}
	
private:
	C_ClipPreviewState&	m_s;
	F32 m_dt;
};

Debug::Float gDebugSpinAngle("AnimationPreview.SpinAngle", -1.f, Debug::Range(0.f, 360.f));

void C_ClipPreviewState::Update(F32 dt)
{
	if (requestReloadAll)
	{
		requestReloadAll = false;
		C_ResourceCache::Instance().ReloadAll();

		AnimationTools::C_Status status = LoadGlobalSkeleton(C_ResourceCache::Instance().GetSkeletonFilename(), C_ResourceCache::Instance().GetSceneConfig());

		C_GlobalSignals::Instance().skeletonLoaded(&gClipPreviewState->GetGlobalSkeleton(), &gClipPreviewState->m_skeletonInstance, &gClipPreviewState->m_pose, C_ResourceCache::Instance().GetSkeletonFilename());
						
		/*
		if ( m_constraintsPanel )
		{
			m_constraintsPanel->SetConstraints( &gClipPreviewState->constraints, &gClipPreviewState->GetGlobalSkeleton() );
		}
		*/
	}

	UpdateFrameRate(dt);
	
	C_AutoUpdateCamera updateCam( *this, dt );
	
	m_db.UpdateFromRecord( m_animGraphEditor.GetCurrentGraphRecord() );
	
	
	dt *= playbackSpeed;

	if (paused) dt = 0.f;
	
	m_deltaTime = dt;
	
	m_globalTime += static_cast<U32>(m_deltaTime * 1000.f);

	F32 prevTime = m_animTime;
	F32 currTime = prevTime + dt;
	
	AnimationGraph::C_Graph* graph = AnimationGraph::C_Graph::s_activeGraph;

	if ( graph )
	{
		// Update pose if custom curves have changed.
		if ( AnimationGraph::C_Graph::m_customScalarCurveNames.Size() != m_poseFormat.GetFieldCount( Animation::E_PoseFieldType_CustomScalar )
			|| AnimationGraph::C_Graph::m_customVector3CurveNames.Size() != m_poseFormat.GetFieldCount( Animation::E_PoseFieldType_CustomVector3 )
			|| AnimationGraph::C_Graph::m_customVector4CurveNames.Size() != m_poseFormat.GetFieldCount( Animation::E_PoseFieldType_CustomVector4 )
			|| AnimationGraph::C_Checksum16(m_customCurveChecksum) != AnimationGraph::C_Graph::m_customCurveChecksum
			
		)
		{
			m_customCurveChecksum = AnimationGraph::C_Graph::m_customCurveChecksum.AsU16();
			
			m_poseFormat.Clear();
			m_poseFormat.SetSkeleton( m_globalSkeleton );
			
			for ( AnimationGraph::C_StringSet::Iterator iter = graph->m_customScalarCurveNames.Begin(); iter != graph->m_customScalarCurveNames.End(); ++iter )
			{
				m_poseFormat.AddCustomScalarField( iter->GetKey().Chars() );
			}		
			for ( AnimationGraph::C_StringSet::Iterator iter = graph->m_customVector3CurveNames.Begin(); iter != graph->m_customVector3CurveNames.End(); ++iter )
			{
				m_poseFormat.AddCustomVector3Field( iter->GetKey().Chars() );
			}		
			for ( AnimationGraph::C_StringSet::Iterator iter = graph->m_customVector4CurveNames.Begin(); iter != graph->m_customVector4CurveNames.End(); ++iter )
			{
				m_poseFormat.AddCustomVector4Field( iter->GetKey().Chars() );
			}
			m_pose.Create( m_poseFormat );
			m_pose.Clear();
		}
	}

	
	if ( NULL == m_globalSkeleton )
	{
		return;
	}

	U32 numJoints = m_globalSkeleton->GetNumJoints();
	
	if ( 0 == numJoints )
	{
		// No joints to animate
		return;
	}

	//assert( dt >= 0.f );
	
	// Update blend tree.
	
	if ( graph )
	{
		graph->UpdateControllerState( controllerState );
		graph->UpdateTime( dt );
	}
	
	//gLayerStack.UpdateTime( dt );
	//gLayerStack.EvaluateMotionDelta(motionDelta, m_globalSkeleton, kMotionRootJointIndex);
	
	// Evaluate local space joint transforms from layers.
	poseInWorldSpace = false;
	if ( graph )
	{
		commandListOk = graph->Evaluate( m_globalSkeleton, m_pose, poseInWorldSpace, enableCommandListPushPopElimination );
	}
	else
	{
		Animation::C_SkeletonPose skeletonPose( *m_globalSkeleton, m_pose );
		
		// Fill local space transforms from base pose.
		skeletonPose.ClearToBasePose();

		commandListOk = true;
	}
	
	// Update character root.
	motionDelta.SetIdentity();	
	motionDelta = m_pose.GetTrajectoryDeltaTransform();
	
	if ( !freezeRoot )
	{
		if ( absoluteRootMotion )
		{
			rootTransform.SetIdentity();
		}
		else
		{
			rootTransform = motionDelta * rootTransform;
			rootTransform.Normalise();
		}
	}

	if (gDebugSpinAngle >= 0.f)
	{
		rootTransform.SetRotation(Maths::MakeUnitQuaternion(Maths::Const<Maths::Normal3>::AxisY(), Maths::MakeVF32(Maths::DegToRad((F32) gDebugSpinAngle))));
	}

	if ( poseInWorldSpace )
	{
		Animation::C_JointTransform rootWorldTransform = m_pose.GetJointTransform( kMotionRootJointIndex );
		Animation::C_JointTransform inverseRootWorldTransform = rootWorldTransform.Inverse();
		
		for ( U32 i = 0; i < m_pose.GetNumJointTransforms(); ++i )
		{
			m_pose.GetJointTransform(i) *= inverseRootWorldTransform;
		}
	}
	else
	{
		if (!absoluteRootMotion)
		{
			m_pose.GetJointTransform( kMotionRootJointIndex ).SetIdentity();	// Careful
		}

		//skeletonPose.SetLocalJointRotation( kMotionRootJointIndex, Maths::Const<Maths::Quaternion>::Identity() );
		//skeletonPose.SetLocalJointTranslation( kMotionRootJointIndex, Maths::Const<Maths::Vector3>::Zero() );
	}
	
	if ( gEnableDrivenJoints )
	{
		PERF_EVENT( "EvaluateDrivenJoints" );
		
		Animation::C_JointTransform modelWorldTransform = rootTransform;
		
		m_dynamicsContext.Update( m_skeletonInstance, modelWorldTransform.AsMat43(), dt );
		m_skeletonInstance.ApplyJointConstraints( m_pose, modelWorldTransform );
	}
	
	m_worldJointMatrices.Resize( numJoints );
	
	Animation::C_SkeletonPose skeletonPose( *m_globalSkeleton, m_pose );
		
	if ( poseInWorldSpace )
	{
		skeletonPose.GetLocalJointMatrices( &m_worldJointMatrices[0], numJoints );
	}
	else
	{
		m_modelJointTransforms.Resize(numJoints);
		skeletonPose.GetModelJointMatrices( &m_worldJointMatrices[ 0 ], &m_modelJointTransforms[0], numJoints );
	}
	
	if ( ( showSkinnedMesh || showWireframe ) && GetSkinnedMesh().IsValid() && numJoints > 0 )
	{
		GetSkinnedMesh().CalculateSkinnedVertices( &m_worldJointMatrices[ 0 ] );
	}

	m_animTime += dt;
	
	if (!paused)
	{
		C_GlobalSignals::Instance().animationUpdated(dt);
	}
}

void C_ClipPreviewState::UpdateCamera( F32 dt )
{
	if (HasValidGlobalSkeleton())
	{
		Maths::Mat43 modelWorldMatrix = rootTransform.AsMat43();

		S32 selectedJointIndex = GetSelectedJointIndex();
		bool validJoint = selectedJointIndex >= 0;
		
		if ( m_cameraMode == E_CameraMode_AttachToJoint && validJoint )
		{
			Animation::C_JointTransform cameraTransform = GetModelJointTransform(GetSelectedJointIndex()) * rootTransform;

			camera.SetType( E_CameraType_LookAt );
			camera.SetEyePos( cameraTransform.GetPosition() );
			camera.SetTargetPos( Maths::MakePoint3( cameraTransform.GetTranslation() ) + Maths::Const<Maths::Normal3>::NegativeAxisZ() * cameraTransform.GetRotation() );
			camera.SetUpVector( Maths::Const<Maths::Normal3>::AxisY() * cameraTransform.GetRotation() );
		}
		else // if (m_View.m_Camera.GetType() == Samples::E_CameraType_Orbit)
		{
			camera.SetType( E_CameraType_Orbit );
			
			Maths::Point3 targetPos;
			if ( m_cameraMode == E_CameraMode_OrbitSelected && validJoint )
			{
				targetPos = GetModelJointTransform(GetSelectedJointIndex()).GetPosition() * modelWorldMatrix;
			}
			else
			{
				targetPos = (GetModelJointTransform(0) * rootTransform).TransformPoint( centre );
			}
			camera.SetTargetPos( targetPos );
		}
	}	
	camera.Update( dt );
}

void C_ClipPreviewState::SetCameraMode(E_CameraMode mode)
{
	if (mode != m_cameraMode)
	{
		switch (mode)
		{
			case E_CameraMode_OrbitCharacter:
			{
				// Save current position as a relative offset in character space, relative to centre
				Maths::Vector3 offset = rootTransform.Inverse().TransformPoint(camera.GetTarget()) - centre;
				camera.SetTargetOffset(offset);
				break;
			}
				
			case E_CameraMode_OrbitSelected:
				// Focus directly on selected joint, clearing any offset from target point.
				camera.ClearTargetOffset();
				break;
			case E_CameraMode_AttachToJoint:
				// Attach to joint, clearing all offsets.
				camera.ClearTargetOffset();
				break;
				
			default:
				break;
		}
		m_cameraMode = mode;
	}
}

U32 C_ClipPreviewState::GetNumJoints() const
{
	return m_globalSkeleton ? m_globalSkeleton->GetNumJoints() : 0;
}

Animation::C_JointTransform C_ClipPreviewState::GetModelJointTransform(U32 jointIndex) const
{
#if 1
	Animation::C_JointTransform transform;
	
	if ( poseInWorldSpace )
	{
		transform = m_pose.GetJointTransform(jointIndex);
	}
	else
	{
		if ( m_globalSkeleton && m_globalSkeleton->GetNumJoints() > 0 )
		{
			transform = Animation::C_SkeletonPose( *m_globalSkeleton, const_cast<Animation::C_Pose&>(m_pose) ).GetModelJointTransform( jointIndex );
		}
		else
		{
			transform.SetIdentity();
		}
	}
	return transform;
#else
	return Animation::JointTransformFromMat43( m_worldJointMatrices[ jointIndex ] );
#endif
}

Animation::C_ObjectTransform C_ClipPreviewState::GetModelObjectTransform(U32 jointIndex) const
{
	Animation::C_ObjectTransform transform;
	transform.SetIdentity();
	
	if ( NULL == m_globalSkeleton ) return transform;
	if ( m_globalSkeleton->GetNumJoints() == 0 ) return transform;
	
	if ( poseInWorldSpace )
	{
		transform = Animation::C_ObjectTransform( m_pose.GetJointTransform(jointIndex) );
	}
	else
	{
		transform = Animation::C_ObjectTransform( Animation::C_SkeletonPose( *m_globalSkeleton, const_cast<Animation::C_Pose&>(m_pose) ).GetModelJointTransform( jointIndex ) );
	}
	return transform;
}

const Animation::C_Skeleton& C_ClipPreviewState::GetGlobalSkeleton() const
{
	assert( m_globalSkeleton );
	return *m_globalSkeleton;
}

AnimationTools::C_Status C_ClipPreviewState::LoadGlobalSkeleton( const char* filename, const AnimationTools::C_SceneConfig& sceneConfig )
{
	AnimationTools::C_Status status;
	m_globalSkeleton = NULL;
	m_globalSkeleton = C_ResourceCache::Instance().GetSkeleton( filename, sceneConfig, &constraints, &status );

	if ( m_globalSkeleton )
	{
		U32 numJoints = m_globalSkeleton->GetNumJoints();
		
		selectedConstraints.Clear();
		selectedConstraints.Reserve(constraints.GetNumConstraints());
		
		filteredJoints.Resize( numJoints );
		filteredJoints.ClearBits();
		
		m_poseFormat.Clear();
		m_poseFormat.SetSkeleton( m_globalSkeleton );
		
		m_pose.Create( m_poseFormat );
		m_skeletonInstance.Create( *m_globalSkeleton );
		
		m_pose.Clear();
		
		Maths::Vector3 theCentre = Maths::Const<Maths::Vector3>::Zero();
		Maths::Vector3 mins = Maths::MakeVector3( FLT_MAX, FLT_MAX, FLT_MAX );
		Maths::Vector3 maxs = Maths::MakeVector3( -FLT_MAX, -FLT_MAX, -FLT_MAX );
		
		radius = 1.f;
		
		if ( m_globalSkeleton->GetNumJoints() > 0 )
		{
			for ( U32 i = 0; i < m_globalSkeleton->GetNumJoints(); ++i )
			{
				Maths::Vector3 pos = Maths::RowW(Maths::Inverse(m_globalSkeleton->GetInverseWorldBasePoseMatrix(i)));
				mins = Maths::Select( pos < mins, pos, mins );
				maxs = Maths::Select( pos > maxs, pos, maxs );
			}
			theCentre = (mins + maxs) * Maths::Const<Maths::VF32>::Half();
			const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
			Maths::SetX( &theCentre, zero );
			Maths::SetZ( &theCentre, zero );
			
			// set x and z to zero.
			radius = Maths::GetF32(Maths::Length(maxs - mins));
		}
		centre = m_globalSkeleton->GetLocalBasePoseJointTransform(0).InverseTransformPoint( Maths::MakePoint3(theCentre) );
				
		// Configure of dynamic joints
		InitialiseDynamicJoints();		

		// Remove all animations as skeleton may be different.
		// TODO: could remap.
		//gLayerStack.ClearAll();
		
		// Load any animations that haven't been loaded yet
		AnimationGraph::C_Graph* graph = AnimationGraph::C_Graph::s_activeGraph;
		if ( graph )
		{
			if ( graph->m_result )
			{
				graph->m_result->LoadResources(C_ResourceCache::Instance());
			}
			graph->UpdateCustomCurveNames();
		}
	}
	
	return status;
}

void C_ClipPreviewState::SetSelectedJointIndex( S32 jointIndex )
{
	if ( jointIndex != m_selectedJointIndex )
	{
		m_selectedJointIndex = jointIndex; 
		
		if (m_cameraMode == E_CameraMode_OrbitSelected)
		{
			camera.ClearTargetOffset();
		}
	}
}

void C_ClipPreviewState::SetTimelineValue(F32 value)
{
	AnimationGraph::C_Graph* graph = AnimationGraph::C_Graph::s_activeGraph;
	if ( graph )
	{
		graph->SetTimelineValue(value);
	}
}

void C_ClipPreviewState::SetSceneScale( F32 scale )
{
	
	C_ResourceCache::Instance().GetSceneConfig().scaleFactor = scale;
}

F32 C_ClipPreviewState::GetSceneScale() const
{
	return C_ResourceCache::Instance().GetSceneConfig().scaleFactor;
}

void C_ClipPreviewState::DestroyGraphicsResources()
{
	C_ResourceCache::Instance().GetSkinnedMesh()->Destroy();
}

void C_ClipPreviewState::UpdateFrameRate(F32 deltaTime)
{
	m_smoothedDeltaTime = m_smoothedDeltaTime + (deltaTime - m_smoothedDeltaTime) * 0.05f;
	m_frameRate = 1.f / m_smoothedDeltaTime;
}

