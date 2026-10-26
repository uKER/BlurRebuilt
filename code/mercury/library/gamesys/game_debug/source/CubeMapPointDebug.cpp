#include "GameDebugPch.h"

#include <game_debug/CubeMapPointDebug.h>

#include <world_render/ModelInstance.h>
#include <game_resource/ModelData.h>
#include <SceneGraph/SceneGraph.h>
#include <game_resource/LevelSceneManager.h>
#include <world_render/Scene.h>
#include <world_render/LightSamplePointScene.h>
#include <world_render/MaterialParameter.h>
#include <world_render/RenderModelData.h>
#include <world_render/CubemapPoints.h>
#include <world_render/RenderLevel.h>


namespace GameDebug
{

Debug::Bool g_CubemapRenderModels("Graphics.WorldRender.LocalisedCubemaps.RenderModels", false);
Debug::String g_CubemapModelFilename("Graphics.WorldRender.LocalisedCubemaps.Settings.ModelFilename", "dontship/models/system/EnvSphere.model" );
Debug::Float g_CubemapModelDrawDistance("Graphics.WorldRender.LocalisedCubemaps.Settings.ModelDrawDistance", 1000.0f );

C_CubemapPointDebug::C_CubemapPointDebug()
{
	m_pModelResource = NULL;
}

C_CubemapPointDebug::~C_CubemapPointDebug()
{
}

void C_CubemapPointDebug::Init( GameResource::C_LevelSceneManager* pLevelSceneManager )
{
	m_pLevelSceneManager = pLevelSceneManager;
}

void C_CubemapPointDebug::Close()
{
	UpdateModelInstances( Maths::HVec3(0.0f, 0.0f, 0.0f), false );
	ReleaseModel();
}

void C_CubemapPointDebug::Update( const Maths::HVec3 PCREF pos )
{
	if(g_CubemapRenderModels)
	{
		if(m_pModelResource == NULL)
		{
			LoadModel();
		}

		UpdateModelInstances( pos, true );
	}
	else
	{
		UpdateModelInstances( pos, false );
		ReleaseModel();
	}
}

void C_CubemapPointDebug::UpdateModelInstances( const Maths::HVec3 PCREF pos, bool renderInstances )
{
	Ut::Vector<const WorldRender::CubemapLocation*> cubemapPointsToRender;

	WorldRender::C_Scene* pWorldRenderScene = m_pLevelSceneManager->GetWorldRenderScene();
	SceneGraph::C_SceneGraph* pSceneGraph = pWorldRenderScene->GetSceneGraph();

	// build list of cubemap points to render
	if(renderInstances)
	{
		for (Btl::List<WorldRender::C_LevelInstance>::Iterator it = pWorldRenderScene->GetRenderLevelInstanceList().Begin();
			it != pWorldRenderScene->GetRenderLevelInstanceList().End(); ++it)
		{
			const WorldRender::C_LocalisedCubemapData *p = &it->GetRenderLevel()->GetCubemapData();

			const S32 locationCount = p->GetLocationCount();
			for(S32 locationIndex=0; locationIndex<locationCount; locationIndex++)
			{
				const WorldRender::CubemapLocation* pLocation = p->GetLocation(locationIndex);				
				if( pLocation->vPos.Distance( pos ) < g_CubemapModelDrawDistance )
				{
					cubemapPointsToRender.PushBack(pLocation);
				}
			}
		}
	}

	// iterate through current list creating or deleting model instances as required
	S32 locationIndex = 0;
	while( locationIndex < m_CubemapPointInfo.Size() )
	{
		C_CubemapPointInfo* pCubemapPointInfo = &m_CubemapPointInfo[locationIndex];

		bool found = false;
		for(S32 i=0; i<cubemapPointsToRender.Size(); i++)
		{
			if( cubemapPointsToRender[i] == pCubemapPointInfo->m_pCubemapLocation )
			{
				found = true;
				// remove from to render list so we dont draw it at the end
				cubemapPointsToRender.EraseNonOrdered(i);
				break;
			}
		}

		if(!found)
		{
			// old sample point no longer required. remove from scene
			if(pCubemapPointInfo->m_pModelInstance)
			{
				SceneGraph::Remove( pSceneGraph, pCubemapPointInfo->m_pModelInstance );
				SAFE_DELETE_PTR( pCubemapPointInfo->m_pModelInstance );
			}

			if(pCubemapPointInfo->m_pCubemapLocation->pTexture)
			{
				WorldRender::C_LocalisedCubemapData::ReleaseGPUTexture(pCubemapPointInfo->m_pCubemapLocation->pTexture);
			}

			// remove from active list
			m_CubemapPointInfo.EraseByIndexNonOrdered( locationIndex );
		}
		else
		{
			locationIndex++;
		}		
	}

	// create new remaining model instances
	if( m_pModelResource != NULL )
	{
		S32 modelIndex = 0;
		{
			S32 modelCount = m_pModelResource->GetModelData()->GetModelCount();
			if(modelCount > 1)
			{
				modelIndex = 1;
			}
		}		

		/*Maths::C_AxisBox localBoundingBox = m_pModelResource->GetModelData()->GetModel( modelIndex )->GetLocalBounds();
		Maths::C_Sphere localBoundingSphere = localBoundingBox.BoundingSphere(); */

		Maths::C_Sphere localBoundingSphere = m_pModelResource->GetModelData()->GetModel( modelIndex )->GetBoundingSphere(); 

		// remaining items in sample point list must be added
		for(S32 i=0; i<cubemapPointsToRender.Size(); i++)
		{
			C_CubemapPointInfo newCubemapPointInfo;		
			const WorldRender::CubemapLocation* pLocation = cubemapPointsToRender[i];
			newCubemapPointInfo.m_pCubemapLocation = pLocation;

			WorldRender::C_ModelInstance* pModelInstance = new WorldRender::C_ModelInstance(m_pModelResource->GetRenderModelData(), modelIndex, pSceneGraph );
			newCubemapPointInfo.m_pModelInstance = pModelInstance;

			Maths::HMat43 mWorld;
			mWorld.SetIdentity();
			mWorld.AW( pLocation->vPos );

			SceneGraph::Insert( pSceneGraph, newCubemapPointInfo.m_pModelInstance);

			pModelInstance->SetWorldSphere( localBoundingSphere * mWorld );
			pModelInstance->SetElementMatrix( -1, mWorld );

			if(pLocation->pTexture)
			{
				WorldRender::MaterialParameter handle = pModelInstance->GetRenderModel()->GetParameter("g_tEnvironment");
				const void* gpuData = WorldRender::C_LocalisedCubemapData::GetGPUTexture(pLocation->pTexture);
				if(handle.index != -1)
				{
					pModelInstance->SetParameter(handle, &gpuData);
				}
			}

			WorldRender::MaterialParameter shHandle = pModelInstance->GetRenderModel()->GetParameter("g_SHNode");
			if (shHandle.index != -1)
			{
				WorldRender::C_LightSampleData lightSampleData;
				pModelInstance->SetParameter(shHandle, &lightSampleData.m_SampleData[0]);	
			}

			m_CubemapPointInfo.PushBack( newCubemapPointInfo );
		}
	}

/*
	
	SceneGraph::C_SceneGraph* pSceneGraph = pWorldRenderScene->GetSceneGraph();
	WorldRender::C_LightSamplePointScene& lightSamplePointScene = pWorldRenderScene->GetLightSamplePointScene();		
	const WorldRender::C_LightSamplePointList& lightSamplePointsList = lightSamplePointScene.GetLightSamplePointsList();
	
	// build up list of sample points to render
	if( renderInstances )
	{		
		WorldRender::C_LightSamplePointList::ConstIterator it = lightSamplePointsList.Begin();
		while(it != lightSamplePointsList.End())
		{
			const WorldRender::C_LightSamplePoints* pLightSamplePoints = *it;
			const S32 lightSamplePointCount = pLightSamplePoints->GetLightSamplePointCount();
			for(S32 sampleIndex=0; sampleIndex<lightSamplePointCount; sampleIndex++)
			{
				const WorldRender::C_LightSamplePoint& lightSamplePoint = pLightSamplePoints->GetLightSamplePoint(sampleIndex);

				if( lightSamplePoint.m_Position.Distance( pos ) < g_CubemapModelDrawDistance )
				{
					WorldRender::CubemapLocation* toRender;
					toRender.m_pLightSamplePoint = &lightSamplePoint;
					toRender.m_pLightSamplePoints = pLightSamplePoints;
					samplePointsToRender.PushBack( toRender );
				}
			}
			++it;
		}
	}

	// iterate through current list creating or deleting model instances as required
	S32 samplePointIndex = 0;
	while( samplePointIndex < m_SamplePointInfo.Size() )
	{
		C_SamplePointInfo* pSamplePointInfo = &m_SamplePointInfo[samplePointIndex];

		bool found = false;
		for(S32 i=0; i<samplePointsToRender.Size(); i++)
		{
			if( samplePointsToRender[i].m_pLightSamplePoint == pSamplePointInfo->m_pLightSamplePoint )
			{
				found = true;
				// remove from to render list so we dont draw it at the end
				samplePointsToRender.EraseNonOrdered(i);
				break;
			}
		}

		if(!found)
		{
			// old sample point no longer required. remove from scene
			if(pSamplePointInfo->m_pModelInstance)
			{
				SceneGraph::Remove( pSceneGraph, pSamplePointInfo->m_pModelInstance );
				SAFE_DELETE_PTR( pSamplePointInfo->m_pModelInstance );
			}
			// remove from active list
			m_SamplePointInfo.EraseByIndexNonOrdered( samplePointIndex );
		}
		else
		{
			samplePointIndex++;
		}		
	}

	// create new remaining model instances
	if( m_pModelResource != NULL )
	{
		S32 modelIndex = 0;
		{
			S32 modelCount = m_pModelResource->GetModelData()->GetModelCount();
			if(modelCount > 1)
			{
				modelIndex = 1;
			}
		}		

		Maths::C_AxisBox localBoundingBox = m_pModelResource->GetModelData()->GetModel( modelIndex )->GetLocalBounds();
		Maths::C_Sphere localBoundingSphere = localBoundingBox.BoundingSphere();

		// remaining items in sample point list must be added
		for(S32 i=0; i<samplePointsToRender.Size(); i++)
		{
			C_SamplePointInfo newSamplePointInfo;			
			newSamplePointInfo.m_pLightSamplePoint = samplePointsToRender[i].m_pLightSamplePoint;

			WorldRender::C_ModelInstance* pModelInstance = new WorldRender::C_ModelInstance(m_pModelResource->GetRenderModelData(), modelIndex, pSceneGraph );
			newSamplePointInfo.m_pModelInstance = pModelInstance;

			Maths::HMat43 mWorld;
			mWorld.SetIdentity();
			mWorld.AW( newSamplePointInfo.m_pLightSamplePoint->m_Position );

			pModelInstance->SetWorldSphere( localBoundingSphere * mWorld );

			pModelInstance->SetElementMatrix( -1, mWorld );

			WorldRender::MaterialParameter handle = pModelInstance->GetRenderModel()->GetParameter("g_SHNode");
			if (handle.index != -1)
			{
				WorldRender::C_LightSampleData lightSampleData;
				samplePointsToRender[i].m_pLightSamplePoints->GetSampleData( lightSampleData, samplePointsToRender[i].m_pLightSamplePoint->m_SampleIndex );
				pModelInstance->SetParameter(handle, &lightSampleData.m_SampleData[0]);	
			}

			SceneGraph::Insert( pSceneGraph, newSamplePointInfo.m_pModelInstance);

			m_SamplePointInfo.PushBack( newSamplePointInfo );
		}
	}*/
}

void C_CubemapPointDebug::LoadModel()
{
	m_ModelHandle.Load( g_CubemapModelFilename.Value() );
	m_pModelResource = m_ModelHandle;
	Resource::WaitForResourceData(m_pModelResource); // debug code, resource wait allowed

	if( !m_pModelResource->IsDataReady() )
	{
		m_pModelResource = NULL;
		m_ModelHandle = NULL;
	}

}

void C_CubemapPointDebug::ReleaseModel()
{
	m_ModelHandle = NULL;
	m_pModelResource = NULL;
}

} // namespace GameDebug
