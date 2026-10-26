
#include "stdafx.h"

#include <system/serialise/C_FileBlock.h>
#include <world_render/WorldRender.h>
#include <world_render/LightSamplePoints.h>
#include <world_render/LightSamplePointScene.h>
#include <world_render/Scene.h>
#include <world_render/RenderLevel.h>
#include <world_render/CubemapPoints.h>
#include <system/debug/I_DebugRender.h>
#include <System/Memory/MemoryTracker.h> 
#include <misc_shared/types/Float16.h>
#include "btl/containers/algorithm.h"

namespace WorldRender
{

Debug::Bool g_RenderLightSamplers( "Graphics.WorldRender.LightSamplePoints.RenderLightSamplers", false );
Debug::Float g_RenderLightSamplersDrawDistance( "Graphics.WorldRender.LightSamplePoints.Settings.LightSamplersDrawDistance", 50.0f, Debug::Range(0.0f, 10000.0f) );
Debug::Float g_RenderLightSamplersSize( "Graphics.WorldRender.LightSamplePoints.Settings.LightSamplersSize", 0.05f, Debug::Range(0.0f, 2.0f) );

Debug::Bool g_RenderLightSamplerValues( "Graphics.WorldRender.LightSamplePoints.Settings.RenderLightSamplerValues", false );

//Debug::Bool g_RenderLightSamplerPositionUpdate( "Graphics.WorldRender.LightSamplePoints.RenderLightSamplerPositionUpdate", false );

// C_LightSamplePointScene
C_LightSamplePointScene::C_LightSamplePointScene(C_Scene *pScene)
: m_pScene( pScene )
{
}

C_LightSamplePointScene::~C_LightSamplePointScene()
{
}

void C_LightSamplePointScene::AddLightSamplePoints( C_LightSamplePoints* pLightSamplePoints )
{
	m_LightSamplePointsList.Insert( pLightSamplePoints );
}

void C_LightSamplePointScene::RemoveLightSamplePoints( C_LightSamplePoints* pLightSamplePoints )
{
	m_LightSamplePointsList.Erase( pLightSamplePoints );
}

void C_LightSamplePointScene::DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HVec3 PCREF vCamera )
{
	#ifndef CFG_GOLD
	Maths::HMat43 m;
	m.SetIdentity();

	// Render all the light sample point lists
	C_LightSamplePointList::Iterator it = m_LightSamplePointsList.Begin();
	while(it != m_LightSamplePointsList.End())
	{
		const C_LightSamplePoints* pLightSamplePoints = *it;
		pLightSamplePoints->DebugRender( pDebugRender, m, vCamera );
		it++;
	}

	// render the light samplers
	if(g_RenderLightSamplers)
	{
		const F32 drawDistanceSquared = (F32)g_RenderLightSamplersDrawDistance * (F32)g_RenderLightSamplersDrawDistance;
		const F32 size = g_RenderLightSamplersSize;

		C_LightSamplerList::Iterator it = m_LightSamplerList.Begin();
		while(it != m_LightSamplerList.End())
		{
			const C_LightSampler& lightSampler = *it;

			Maths::HVec3 vLightSamplerPosition = lightSampler.GetPosition();

			if(vLightSamplerPosition.DistanceSquared(vCamera) < drawDistanceSquared )
			{
				Maths::HVec3 vLightSamplePosition;
				GetClosestSamplePosition(vLightSamplePosition, vLightSamplerPosition);

				pDebugRender->Draw3dStar(vLightSamplePosition, size, Maths::RGBA(0.f,1.f,0.f,1.f));
				pDebugRender->Draw3dSphereOutline(vLightSamplerPosition, size, Maths::RGBA(0.f,0.f,1.f,1.f));
				pDebugRender->Draw3dLine(vLightSamplerPosition, vLightSamplePosition, Maths::RGBA(0.f,0.5f,0.5f,1.f));

				if(g_RenderLightSamplerValues)
				{
					C_LightSampleData lightSampleData = lightSampler.GetLightSampleData();
					Util::C_String str;
					str.Printf( "[%0.2f, %0.2f, %0.2f, %0.2f]\n[%0.2f, %0.2f, %0.2f, %0.2f]\n[%0.2f, %0.2f, %0.2f, %0.2f]\n[%0.2f, %0.2f, %0.2f, %0.2f]", 
							lightSampleData.m_SampleData[0], lightSampleData.m_SampleData[1], lightSampleData.m_SampleData[2], lightSampleData.m_SampleData[3], 
							lightSampleData.m_SampleData[4], lightSampleData.m_SampleData[5], lightSampleData.m_SampleData[6], lightSampleData.m_SampleData[7], 
							lightSampleData.m_SampleData[8], lightSampleData.m_SampleData[9], lightSampleData.m_SampleData[10], lightSampleData.m_SampleData[11], 
							lightSampleData.m_SampleData[12], lightSampleData.m_SampleData[13], lightSampleData.m_SampleData[14], lightSampleData.m_SampleData[15] );
						
					pDebugRender->Draw3dText( vLightSamplerPosition, 0.1f, str );
				}
			}

			++it;
		}
	}

	#endif
}

bool C_LightSamplePointScene::GetClosestSample( C_LightSampleData& result, const Maths::HVec3 PCREF vPosition )
{
	//TODO: this should be passed as a parameter, not hardcoded!
	F32 closest = 10.0f;//Maths::Constants<F32>::Max();

	C_LightSamplePointList::Iterator it = m_LightSamplePointsList.Begin();
	while(it != m_LightSamplePointsList.End())
	{
		const C_LightSamplePoints* pLightSamplePoints = *it;
		pLightSamplePoints->GetClosestSample( result, closest, vPosition );
		it++;
	}

	return true;
}

bool C_LightSamplePointScene::GetClosestSamplePosition( Maths::HVec3& vResult, const Maths::HVec3 PCREF vPosition )
{
	//TODO: this should be passed as a parameter, not hardcoded!
	F32 closest = 10.0f;//Maths::Constants<F32>::Max();
	Maths::HVec3 vClosest = vPosition;

	C_LightSamplePointList::Iterator it = m_LightSamplePointsList.Begin();
	while(it != m_LightSamplePointsList.End())
	{
		const C_LightSamplePoints* pLightSamplePoints = *it;

		C_LightSamplePoint lightSamplePoint;
		if( pLightSamplePoints->GetClosestSamplePoint( lightSamplePoint, closest, vPosition ))
		{
			vClosest = lightSamplePoint.m_Position;
		}

		it++;
	}

	vResult = vClosest;

	return true;
}

RenderingData::Target::SharedTextureData *C_LightSamplePointScene::GetClosestCubemap( const Maths::HVec3 PCREF vPosition )
{
	RenderingData::Target::SharedTextureData *pRtn = NULL;

	for (Btl::List<C_LevelInstance>::Iterator it = m_pScene->GetRenderLevelInstanceList().Begin();
		it != m_pScene->GetRenderLevelInstanceList().End(); ++it)
	{
		const C_LocalisedCubemapData *p = &it->GetRenderLevel()->GetCubemapData();
		if (p->GetCubemap(vPosition, &pRtn))
			return pRtn;
	}

	return pRtn;
}

void C_LightSamplePointScene::AddLightSampler( C_LightSampler* pLightSampler )
{
	m_LightSamplerList.PushBack( *pLightSampler );
}

void C_LightSamplePointScene::RemoveLightSampler( C_LightSampler* pLightSampler )
{
	m_LightSamplerList.Erase( Btl::List<C_LightSampler>::Iterator( pLightSampler ) );
}

const C_LightSamplePointList& C_LightSamplePointScene::GetLightSamplePointsList() const
{
	return m_LightSamplePointsList;
}

//////////////////////////////////////////////////////////////////////////
// C_LightSampler
//////////////////////////////////////////////////////////////////////////
C_LightSampler::C_LightSampler()
	: m_pLightSamplePointScene( NULL )
	, m_FirstUpdate( true )
{
}

C_LightSampler::~C_LightSampler()
{
	if( m_pLightSamplePointScene )
	{
		m_pLightSamplePointScene->RemoveLightSampler( this );
	}
}

void C_LightSampler::Initialise( C_LightSamplePointScene* pLightSamplePointScene )
{
	if( m_pLightSamplePointScene == pLightSamplePointScene )
		return;

	if( m_pLightSamplePointScene )
	{
		m_pLightSamplePointScene->RemoveLightSampler( this );
	}

	m_pLightSamplePointScene = pLightSamplePointScene;
	m_pLightSamplePointScene->AddLightSampler( this );
	m_FirstUpdate = true;
}

const C_LightSampleData& C_LightSampler::GetLightSampleData() const
{
	return m_CurrentLightSampleData;
}

void C_LightSampler::Reset()
{ 
	m_FirstUpdate = true;
}

const Maths::HVec3& C_LightSampler::GetPosition() const
{
	return m_Position;
}

void C_LightSampler::SetPosition( const Maths::HVec3& position )
{
	//PERF_EVENT("LightSampler.UpdatePosition");
	const Maths::HF32 fDistSqrd = m_Position.DistanceSquared(position);
	const Maths::HF32 fAllowedMovementSqrd = (1.5f * 1.5f);

	if( (fDistSqrd > fAllowedMovementSqrd) || m_FirstUpdate)
	{
		if( m_pLightSamplePointScene )
		{
			//Time::C_Time startTime = Time::GetPerformanceCounter();
			m_pLightSamplePointScene->GetClosestSample( m_NearestLightSampleData, position );

			//#ifndef CFG_GOLD
			//if(g_RenderLightSamplerPositionUpdate)
			//{
			//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dPoint(position, 2.0f, Maths::RGBA(0xff, 0x00, 0x00, 0xff) );
			//}
			//#endif
			//Time::C_Time endTime = Time::GetPerformanceCounter();
			//Time::C_Time elapsedTime = endTime - startTime;

			//if( elapsedTime > 100)
			//{
			//	DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dText( position, 1.0f, Time::FormatProfileDurationString(elapsedTime));
			//}
		}
		m_Position = position;
	}
}

void C_LightSampler::Update()
{
	if (m_FirstUpdate)
	{
		m_CurrentLightSampleData = m_NearestLightSampleData;
		m_FirstUpdate = false;
	}
	else
	{
		//Lerp
		m_CurrentLightSampleData.Lerp( m_NearestLightSampleData, 0.2f );
	}
}

} // namespace WorldRender
