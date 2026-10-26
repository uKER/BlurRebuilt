#include "stdafx.h"
#include "Actor.h"
#include <animation/cycle/Cycle.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/pose/SkeletonPose.h>
#include <animation/core/CommandProcessor.h>

#ifndef PLATFORM_SPU
C_Actor::C_Actor():
//	m_skeleton( 0 ),
//	m_cycle( 0 ),
//	m_cycleSkeletonMapping( 0 ),
	m_commandList( 32 )
{
}

C_Actor::~C_Actor()
{
}

void C_Actor::Create(
	U32 index
)
{
	m_index = index;
	
	m_animSet.Create();
	
	static const F32 timeOffsetDelta = 0.99f;
	
	m_time = index * timeOffsetDelta;
	
	U32 col = index / 10;
	U32 row = index % 10;
		
	Maths::HVec3 pos = Maths::HVec3( row * 1.5f, 0.f, col * 1.5f );
	m_worldPos = pos;
	
	m_pose.Create( m_animSet.m_poseFormat );	
	
	for ( U32 i = 0; i < E_CycleId_Count; ++i )
	{
		m_animSet.m_cycles[ i ].BindToPose( m_pose );
	}
	
	m_gameRootJointIndex = m_animSet.m_skeleton->FindJointByName( Animation::C_StringId( "game_root" ) );
	
	Animation::C_StringId eventNames[] =
	{
		Animation::C_StringId( "FLAG_GRENADEPULLPIN" ),
		Animation::C_StringId( "FLAG_FASTTURN" ),		
		Animation::C_StringId( "FLAG_AIM" ),			
		Animation::C_StringId( "FLAG_THROWGRENADE" ),	
		Animation::C_StringId( "FLAG_BODYROLL" ),		
		Animation::C_StringId( "FLAG_DAMAGE" ),		
		Animation::C_StringId( "game_root_FLAG_LEFT_FOOTSTEP" ),	
		Animation::C_StringId( "game_root_FLAG_RIGHT_FOOTSTEP" ),
		Animation::C_StringId( "FLAG_GRUNT" )
	};
	
	m_eventLogger.Create( m_pose, sizeof(eventNames)/sizeof(eventNames[0]), eventNames );
	
	m_cycleTimes[ 0 ] = m_time;
	m_cycleTimes[ 1 ] = m_time;
	m_cycleTimes[ 2 ] = m_time;
	
	m_direction = 0.f + index;
	
	m_actorRootTransform.SetIdentity();
	m_actorRootTransform.SetTranslation( pos );
	m_prevGameRootTransform.SetIdentity();
		
	m_worldJointMatrices.Resize( m_animSet.m_skeleton->GetNumJoints() );
	m_skeletonInstance.Create( *m_animSet.m_skeleton );
	
	m_isPlayerControlled = false;
	m_runBlend = 0.f;
	m_runDamping = 0.5f;
	m_dirDamping = 0.5f;
	m_dirX = 0.f;
	m_dirZ = 1.f;
	m_targetDirX = 0.f;
	m_targetDirZ = 1.f;
	
	//m_constraints.AddSkeletonConstraints( *m_animSet.m_skeleton );
	
	InitialiseDynamicJoints();
}

#endif

void C_Actor::SpuFetch()
{
	//PERF_EVENT( "Actor.SpuFetch" );
	
	m_commandList.SpuFetchAsync(0);		// not needed for rendering...
	m_pose.SpuFetchAsync(0);
	m_skeletonInstance.SpuFetch();
	
	Spu::Wait(0);
}

void C_Actor::SpuStore()
{
	//PERF_EVENT( "Actor.SpuStore" );
	m_pose.SpuStore();	
}

inline F32 ClippedWave( F32 time, F32 period, F32 ramp )
{
	return 0.5f * ( 1.f + Maths::Clamped( ramp * Maths::Cos( Maths::DegToRad( 360.f / period ) * time ), -1.f, 1.f ) );
}

void C_Actor::Update( F32 deltaTime )
{
	m_deltaTime = deltaTime;

	if ( !m_isPlayerControlled )
	{
		m_runBlend = ClippedWave( m_time, 10.f, 3.f );
		m_runBlend = Maths::Clamped( m_runBlend, 0.f, 1.f );
	
		m_direction += deltaTime * Maths::DegToRad( 360.f / 40.f );
		m_direction = fmodf( m_direction, Maths::Pi2 );
		
		m_dirX = sinf( m_direction );
		m_dirZ = cosf( m_direction );
	}
	else
	{
		m_dirX = Maths::Lerp( m_dirX, m_targetDirX, m_dirDamping );
		m_dirZ = Maths::Lerp( m_dirZ, m_targetDirZ, m_dirDamping );
		
		F32 runInput = Maths::Sqrt( m_dirX*m_dirX + m_dirZ*m_dirZ );
		m_runBlend = Maths::Lerp( m_runBlend, runInput, m_runDamping );
	}
	
	F32 dirX = m_dirX;
	F32 dirZ = m_dirZ;
	
	F32 dirLength = Maths::Sqrt( dirX*dirX + dirZ*dirZ );
	
	Animation::C_SkeletonPose skeletonPose( *m_skeletonInstance.GetSkeleton(), m_pose );

	//if ( m_cycle && m_cycle.RawPtr()->IsValid() )
	if ( true )
	{
		PERF_EVENT( "EvaluateCycles" );
		
		m_commandList.Clear();
		
		if ( true )
		{
			E_CycleId walkCycles[ 2 ];
			E_CycleId runCycles[ 2 ];
		
			F32 dirBlends[ 2 ];
			
			// F32 walkBlend

			dirBlends[ 0 ] = Maths::Abs( dirZ );
			dirBlends[ 1 ] = Maths::Abs( dirX );
			
			if ( dirZ > 0.f )
			{
				walkCycles[ 0 ] = E_CycleId_WalkF;
				runCycles[ 0 ] = E_CycleId_RunF;
			}
			else
			{
				walkCycles[ 0 ] = E_CycleId_WalkB;
				runCycles[ 0 ] = E_CycleId_RunB;
			}

			if ( dirX > 0.f )
			{
				walkCycles[ 1 ] = E_CycleId_WalkR;
				runCycles[ 1 ] = E_CycleId_RunR;
			}
			else
			{
				walkCycles[ 1 ] = E_CycleId_WalkL;
				runCycles[ 1 ] = E_CycleId_RunL;
			}
			
			F32 blendSum = dirBlends[ 0 ] + dirBlends[ 1 ];
			
			F32 invBlendSum = 0.f;
			if ( blendSum > 0.f )
			{
				invBlendSum = Maths::Reciprocal( blendSum );
			}
			
			F32 walkDur =
				dirBlends[ 0 ] * m_animSet.m_cycles[ walkCycles[ 0 ] ].GetDuration() +
				dirBlends[ 1 ] * m_animSet.m_cycles[ walkCycles[ 1 ] ].GetDuration();
			walkDur = walkDur * invBlendSum;
				
			F32 runDur = 
				dirBlends[ 0 ] * m_animSet.m_cycles[ runCycles[ 0 ] ].GetDuration() +
				dirBlends[ 1 ] * m_animSet.m_cycles[ runCycles[ 1 ] ].GetDuration();
			runDur = runDur * invBlendSum;
			
			F32 finalDur = walkDur + ( runDur - walkDur ) * m_runBlend;
			
			F32 invFinalDur = 0.f;
			if ( finalDur > 0.f )
			{
				invFinalDur = Maths::Reciprocal( finalDur );
			}
			
			F32 walkTimeScale = walkDur * invFinalDur;
			F32 runTimeScale = runDur * invFinalDur;

			m_cycleTimes[ 0 ] += walkTimeScale * deltaTime;
			m_cycleTimes[ 1 ] += runTimeScale * deltaTime;
			
		
			m_commandList.BlendCycle(
				Animation::E_BlendMode_Accumulate, 
				&m_animSet.m_cycles[ walkCycles[ 0 ] ],
				m_cycleTimes[ 0 ],
				dirBlends[ 0 ] * ( 1.f - m_runBlend )
			);
			
			m_commandList.BlendCycle(
				Animation::E_BlendMode_Accumulate, 
				&m_animSet.m_cycles[ walkCycles[ 1 ] ],
				m_cycleTimes[ 0 ],
				dirBlends[ 1 ] * ( 1.f - m_runBlend )
			);

			m_commandList.BlendCycle(
				Animation::E_BlendMode_Accumulate, 
				&m_animSet.m_cycles[ runCycles[ 0 ] ],
				m_cycleTimes[ 1 ],
				dirBlends[ 0 ] * m_runBlend
			);

			m_commandList.BlendCycle(
				Animation::E_BlendMode_Accumulate, 
				&m_animSet.m_cycles[ runCycles[ 1 ] ],
				m_cycleTimes[ 1 ],
				dirBlends[ 1 ] * m_runBlend
			);

			m_commandList.BlendCycle(
				Animation::E_BlendMode_Accumulate, 
				&m_animSet.m_cycles[ E_CycleId_StandIdle ],
				m_time,
				Maths::Max( 1.f - 10.f * blendSum, 0.f )
			);
			
			m_commandList.BlendCycle(
				Animation::E_BlendMode_Additive,
				&m_animSet.m_cycles[ E_CycleId_AimHAdditive ],
				m_time,
				Maths::Min( blendSum, 1.f ) * ClippedWave( m_time, 4.f, 2.f )
			);
		}
		
		m_commandList.EvaluateBasePose();

		Animation::C_CommandProcessor cmdProcessor;
		cmdProcessor.ExecuteCommandList( m_commandList, m_pose, *m_skeletonInstance.GetSkeleton() );
	}
	else
	{
		skeletonPose.ClearToBasePose();
	}
	
	m_eventLogger.Update( m_pose );

	// Calculate how root transform has changed since last update.
	Animation::C_JointTransform deltaRootTransform = m_pose.GetTrajectoryDeltaTransform();
//	deltaRootTransform.Normalise();
	
	// Apply this delta to the current actor transform.
	m_actorRootTransform = deltaRootTransform * m_actorRootTransform;
	
	// Remove any movement on the synthetic root.
	skeletonPose.SetLocalJointTranslation( 0, Maths::Const< Maths::Vector3 >::Zero() );
	skeletonPose.SetLocalJointRotation( 0, Maths::Const< Maths::Quaternion >::Identity() );
	
	if ( true )
	{
		PERF_EVENT( "EvaluateDrivenJoints" );
		m_skeletonInstance.UpdateDynamicJoints( deltaTime );
		m_skeletonInstance.ApplyJointConstraints( m_pose, m_actorRootTransform );
	}

	m_time += deltaTime;
}


#if !PLATFORM_SPU

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

void C_Actor::InitialiseDynamicJoints()
{
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
		
		S32 jointIndex = m_skeletonInstance.GetSkeleton()->FindJointByName( jointId );
		S32 targetJointIndex = m_skeletonInstance.GetSkeleton()->FindJointByName( targetJointId );
		
		if ( jointIndex >= 0 && targetJointIndex >= 0 )
		{
			m_skeletonInstance.SetDynamicAimJointTarget( jointIndex, targetJointIndex );
			m_skeletonInstance.SetDynamicJointMass( jointIndex, info->mass );
			m_skeletonInstance.SetDynamicAimJointLimits( jointIndex, info->minHeading, info->maxHeading, info->minPitch, info->maxPitch );
		}		
	}
	
}
#endif

