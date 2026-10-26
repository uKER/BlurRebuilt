#include "GameDebugPch.h"

#include <game_debug/LightSamplePointDebug.h>

#include <world_render/ModelInstance.h>
#include <game_resource/ModelData.h>
#include <SceneGraph/SceneGraph.h>
#include <game_resource/LevelSceneManager.h>
#include <world_render/Scene.h>
#include <world_render/LightSamplePointScene.h>
#include <world_render\MaterialParameter.h>
#include <world_render\RenderModelData.h>

namespace GameDebug
{

Debug::Bool g_RenderModels("Graphics.WorldRender.LightSamplePoints.RenderModels", false);
Debug::String g_ModelFilename("Graphics.WorldRender.LightSamplePoints.Settings.ModelFilename", "dontship/models/system/ShSphere.model" );
Debug::Float g_ModelDrawDistance("Graphics.WorldRender.LightSamplePoints.Settings.ModelDrawDistance", 10.0f );

C_LightSamplePointDebug::C_LightSamplePointDebug()
{
	m_pModelResource = NULL;
}

C_LightSamplePointDebug::~C_LightSamplePointDebug()
{
}

void C_LightSamplePointDebug::Init( GameResource::C_LevelSceneManager* pLevelSceneManager )
{
	m_pLevelSceneManager = pLevelSceneManager;
}

void C_LightSamplePointDebug::Close()
{
	UpdateModelInstances( Maths::HVec3(0.0f, 0.0f, 0.0f), false );
	ReleaseModel();
}

void C_LightSamplePointDebug::Update( const Maths::HVec3 PCREF pos )
{
	if(g_RenderModels)
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

class C_LightSampleToRender
{
public:
	const WorldRender::C_LightSamplePoints* m_pLightSamplePoints;
	WorldRender::C_LightSamplePoint m_LightSamplePoint;
};

void C_LightSamplePointDebug::UpdateModelInstances( const Maths::HVec3 PCREF pos, bool renderInstances )
{
	Ut::Vector<C_LightSampleToRender> samplePointsToRender;
	
	WorldRender::C_Scene* pWorldRenderScene = m_pLevelSceneManager->GetWorldRenderScene();
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
				const WorldRender::C_LightSamplePoint lightSamplePoint = pLightSamplePoints->GetLightSamplePoint(sampleIndex);

				if( lightSamplePoint.m_Position.Distance( pos ) < g_ModelDrawDistance )
				{
					C_LightSampleToRender toRender;
					toRender.m_LightSamplePoint = lightSamplePoint;
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
			if( samplePointsToRender[i].m_LightSamplePoint.m_Position.Distance(  pSamplePointInfo->m_LightSamplePoint.m_Position ) < 0.01f )
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

		/*Maths::C_AxisBox localBoundingBox = m_pModelResource->GetModelData()->GetModel( modelIndex )->GetLocalBounds();
		Maths::C_Sphere localBoundingSphere = localBoundingBox.BoundingSphere(); */
		Maths::C_Sphere localBoundingSphere = m_pModelResource->GetModelData()->GetModel( modelIndex )->GetBoundingSphere(); 

		// remaining items in sample point list must be added
		for(S32 i=0; i<samplePointsToRender.Size(); i++)
		{
			C_SamplePointInfo newSamplePointInfo;			
			newSamplePointInfo.m_LightSamplePoint = samplePointsToRender[i].m_LightSamplePoint;

			WorldRender::C_ModelInstance* pModelInstance = new WorldRender::C_ModelInstance(m_pModelResource->GetRenderModelData(), modelIndex, pSceneGraph );
			newSamplePointInfo.m_pModelInstance = pModelInstance;

			Maths::HMat43 mWorld;
			mWorld.SetIdentity();
			mWorld.AW( newSamplePointInfo.m_LightSamplePoint.m_Position );

			pModelInstance->SetWorldSphere( localBoundingSphere * mWorld );

			pModelInstance->SetElementMatrix( -1, mWorld );

			WorldRender::MaterialParameter handle = pModelInstance->GetRenderModel()->GetParameter("g_SHNode");
			if (handle.index != -1)
			{
				WorldRender::C_LightSampleData lightSampleData;
				samplePointsToRender[i].m_pLightSamplePoints->GetSampleData( lightSampleData, samplePointsToRender[i].m_LightSamplePoint.m_SampleIndex );
				pModelInstance->SetParameter(handle, &lightSampleData.m_SampleData[0]);	
			}

			SceneGraph::Insert( pSceneGraph, newSamplePointInfo.m_pModelInstance);

			m_SamplePointInfo.PushBack( newSamplePointInfo );
		}
	}
}

void C_LightSamplePointDebug::LoadModel()
{
	m_ModelHandle.Load( g_ModelFilename.Value() );
	m_pModelResource = m_ModelHandle;
	Resource::WaitForResourceData(m_pModelResource); // debug code, resource wait allowed

	if( !m_pModelResource->IsDataReady() )
	{
		m_pModelResource = NULL;
		m_ModelHandle = NULL;
	}

}

void C_LightSamplePointDebug::ReleaseModel()
{
	m_ModelHandle = NULL;
	m_pModelResource = NULL;
}

} // namespace GameDebug
