#include "stdafx.h"
#include "AnimationSet.h"
#include <animation/resource/ResourceManager.h>
#include <animation/pose/PoseFormat.h>

static const CHAR8* cycleNames[ E_CycleId_Count ] =
{
	"renwick_test/Ply_M_Ar1_Walk_F01_Nm",
	"renwick_test/Ply_M_Ar1_Walk_B01_Nm",
	"renwick_test/Ply_M_Ar1_Walk_L01_Nm",
	"renwick_test/Ply_M_Ar1_Walk_R01_Nm",
	"renwick_test/Ply_M_Ar1_Run_F01_Nm",
	"renwick_test/Ply_M_Ar1_Run_B01_Nm",
	"renwick_test/Ply_M_Ar1_Run_L01_Nm",
	"renwick_test/Ply_M_Ar1_Run_R01_Nm",
	"additive_test/AimH",
	"additive_test/RunF_AimMUpper",
	"renwick_test/Ply_M_Hg1_StandIdle_04_Nm"
	
};

static const CHAR8* skeletonName =
	//"RenwickOnCoreRig";
	"renwick_dynamic/Renwick_Skeleton";

static Debug::Logger& gLog = Debug::GetLogger( "mercurygame.animation" );

#ifndef PLATFORM_SPU

//
// C_AnimationStore
//

static const Util::C_String gDataFolder = "Animations/";
static const Util::C_String gCycleExt = ".cycle";
static const Util::C_String gSkeletonExt = ".skeleton";


C_AnimationStore gAnimationStore;

bool C_AnimationStore::LoadResources()
{
	bool ok = m_skeletonHandle.Load( gDataFolder + skeletonName + gSkeletonExt );
	
	if ( !ok )
	{
		LOGERROR( gLog, "Error loading skeleton '%s'", skeletonName );
		return false;
	}
	
	Resource::WaitForResourceData( static_cast<Animation::C_SkeletonResource*>( m_skeletonHandle ) );

	for ( U32 i = 0; i < E_CycleId_Count; ++i )
	{
		if ( !m_cycleHandles[ i ].Load( gDataFolder + cycleNames[ i ] + gCycleExt ) )
		{
			LOGERROR( gLog, "Error loading cycle '%s'", cycleNames[ i ] );
		}
		
		Resource::WaitForResourceData( static_cast<Animation::C_CycleResource*>( m_cycleHandles[ i ] ) );
	}
	
	return true;
}

#endif

//
// C_AnimationSet
//

C_AnimationSet::C_AnimationSet()
{
}

#ifndef PLATFORM_SPU
void C_AnimationSet::Create()
{
	m_skeleton = &gAnimationStore.GetSkeleton();	
	
	m_poseFormat.Clear();
	m_poseFormat.SetSkeleton( m_skeleton );
	
	m_poseFormat.AddCustomScalarField( "game_root_FLAG_LEFT_FOOTSTEP" );
	m_poseFormat.AddCustomScalarField( "game_root_FLAG_RIGHT_FOOTSTEP" );	
	m_poseFormat.AddCustomScalarField( "game_root_GUNBUTTIK" );
	m_poseFormat.AddCustomScalarField( "game_root_GUNAIMIK" );
	m_poseFormat.AddCustomScalarField( "game_root_LEFTHANDAIMPOSITIONIK" );
	
	for ( U32 i = 0; i < E_CycleId_Count; ++i )
	{
		const Animation::C_Cycle* cycle = &gAnimationStore.GetCycle( (E_CycleId) i );
		
		m_cycles[ i ] = Animation::C_CycleInstance( cycle );
		
		if ( !cycle )
		{
			LOGERROR( gLog, "Error loading cycle '%s'", cycleNames[ i ] );
		}
	}	
}
#endif

C_AnimationSet::~C_AnimationSet()
{
}


