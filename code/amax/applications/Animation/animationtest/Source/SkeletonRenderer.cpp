#include "stdafx.h"
#include "SkeletonRenderer.h"
//#include <display/render/BasicRender.h>
#include <animation/skeleton/skeleton.h>
#include <animation/pose/SkeletonPose.h>
#include "Actor.h"



void RenderSkeletonVertices(C_SkeletonVertex*& pVertices,
					const Animation::C_Skeleton& skeleton,
					const Maths::Mat43* worldMatrices,
					const Maths::RGBA& colour
)
{
	//PERF_EVENT( "RenderSkeleton" );
	//renderer.SetDepthEnable( true );
	
	Maths::RGBA col = colour;
#if PLATFORM_PS3 || PLATFORM_SPU
	col.R( colour.A() );
	col.G( colour.B() );
	col.B( colour.G() );
	col.A( colour.R() );
#endif
		
	volatile C_SkeletonVertex* pVertex = pVertices;
	
	for (U32 i = skeleton.GetFirstParentedJointIndex(), count = skeleton.GetNumJoints(); i < count; ++i)
	{
		//RenderSkeletonJoint( renderer, i, skeleton, worldTransforms, modelWorldMatrix, colour, showBoneNames, size );
	#if 1
		Maths::HVec3 jointTranslation = Maths::HVec3( Maths::RowW( worldMatrices[ i ] ));

		S32 parentIndex = skeleton.GetJointParentIndex( i );

		Maths::HVec3 parentTranslation = Maths::HVec3( Maths::RowW( worldMatrices[parentIndex] ) );
		
		Maths::Store( parentTranslation, (MathsStorage::Float3*) &pVertex->x );
		pVertex->rgba = col;
		pVertex++;
		
		Maths::Store( jointTranslation, (MathsStorage::Float3*) &pVertex->x );
		pVertex->rgba = col;
		pVertex++;
	#else
			
		Maths::Vector3 jointTranslation = Maths::RowW( worldMatrices[ i ] );
		S32 parentIndex = skeleton.GetJointParentIndex( i );
		Maths::Vector3 parentTranslation = Maths::RowW( worldMatrices[parentIndex] );
		
		Maths::Store( parentTranslation, &pVertex->x );
		_WriteBarrier();
		pVertex->rgba = col;
		_WriteBarrier();
		pVertex++;
		
		Maths::Store( jointTranslation, &pVertex->x );
		_WriteBarrier();
		pVertex->rgba = col;
		_WriteBarrier();
		pVertex++;
	#endif
	}
	
	pVertices += 2 * (skeleton.GetNumJoints() - skeleton.GetFirstParentedJointIndex());
}

JQ_DECLARE_MODULE(SkeletonRenderModule, SkeletonRenderTask, skeletonrendertask);

void SkeletonRenderTask( Scheduler::Task* task )
{
	PERF_EVENT( "SkeletonRenderTask" );
	
	// Fetch parameters.
	C_SkeletonRenderParams* params = (C_SkeletonRenderParams*) task->ParamData;
	
#ifdef PLATFORM_SPU
	// Allocate temporary vertex buffer.
	C_SkeletonVertex* localVertices = (C_SkeletonVertex*) Spu::Alloc( params->m_vertexBufferSize );
#else
	C_SkeletonVertex* localVertices = (C_SkeletonVertex*) params->m_vertices;
#endif
	C_SkeletonVertex* startLocalVertices = localVertices;		
	
	// Render skeletons.
	static const Maths::RGBA white( 255, 255, 255, 255 );
	static const Maths::RGBA green( 0, 255, 0, 255 );
	
	const C_Actor* pActorEa = params->m_actors + params->m_firstActorIndex;
	
	for ( U32 i = 0; i < params->m_numActors; ++i )
	{
		Spu::MemBaseMarker memRestore;
	
		C_Actor* actor = (C_Actor*) Spu::FetchAlloc( pActorEa, sizeof( C_Actor ) );
		
		actor->SpuFetch();
		
		const Animation::C_Skeleton* skeleton = actor->GetSkeleton();
		
	#ifdef PLATFORM_SPU
		Maths::Mat43* worldJointMatrices = (Maths::Mat43*) Spu::Alloc( sizeof( Maths::Mat43 ) * skeleton->GetNumJoints() );
	#else
		Maths::Mat43* worldJointMatrices = &actor->m_worldJointMatrices[ 0 ];
	#endif
		
		Animation::C_SkeletonPose skeletonPose( *skeleton, actor->GetPose() );
				
		#if 1
		const Maths::Mat43 modelWorldMatrix = Maths::MakeMat43( 
			Maths::MakeMat33( Maths::Quaternion(actor->GetWorldRotation()) ),
			static_cast<Maths::Vector3>(actor->GetWorldPos())
		);
		#else
			const Maths::Mat43 modelWorldMatrix = Maths::Const<Maths::Mat43>::Identity();
		#endif
		
		skeletonPose.GetWorldJointMatrices( Animation::MakeArrayProxy( &worldJointMatrices[ 0 ], skeleton->GetNumJoints() ), modelWorldMatrix );
		
		RenderSkeletonVertices(
			localVertices,
			*skeleton,
			&worldJointMatrices[ 0 ],
			( i == 0 && params->m_firstActorIndex == 0 ) ? green : white
		);
		
		++pActorEa;
	}
	
	assert( localVertices - startLocalVertices == params->m_vertexBufferSize / sizeof( C_SkeletonVertex ) );
	
#ifdef PLATFORM_SPU
	Spu::Store( params->m_vertices, startLocalVertices, params->m_vertexBufferSize );
#endif
}

