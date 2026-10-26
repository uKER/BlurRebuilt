#include "stdafx.h"

#include "XsiIoSceneExport.h"
#include "XsiIoSceneNodeExport.h"
#include "XsiIoSceneMeshExport.h"
#include "XsiIoMaterialExport.h"
#include "XsiIoAnimationExport.h"
#include "../XsiIoUtility.h"

using namespace XsiIo;


C_SceneExport::C_SceneExport()
	: m_pAftkScene(0), m_pProgressNotification(0), m_nProcessingStages(0), m_nProcessingStagesCompleted(0), m_DotXsiVersion(0.0f)
{
	m_aftkXsiSceneNodeMap.Clear();
	m_aftkXsiMaterialMap.Clear();
}

Aftk::C_Status C_SceneExport::Export(
	const Aftk::C_Scene* pAftkScene,
	CSLScene* pXsiScene,
	Aftk::I_Progress* pProgress)
{
	Aftk::C_Status status;

	m_pProgressNotification = pProgress;
	m_pAftkScene = pAftkScene;
	m_pXsiScene = pXsiScene;

	m_nProcessingStagesCompleted = 0;
	m_nProcessingStages = 2*pAftkScene->GetNumSceneNodes();
	m_nProcessingStages += pAftkScene->GetNumMaterialNodes();

	//Export material library
	status = ExportMaterials();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN

	//Export scenegraph
	status = ExportSceneHierarchyTopLevel();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN
	
	//Export skinning data
	status = ExportSkinningData();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN

	return Aftk::C_Status::E_Status_Success;
}


void C_SceneExport::CleanUpAfterFailure()
{
}


const Aftk::C_Scene* C_SceneExport::GetAftkScene() const
{
	return m_pAftkScene;
}


CSLScene* C_SceneExport::GetXsiScene() const
{
	return m_pXsiScene;
}



Aftk::C_Status C_SceneExport::GetXsiSceneNodeFromAftk(
	const Aftk::C_SceneNode* pSceneNode,
	CSLModel** ppXsiNode)
{
	Ut::UtMap<const Aftk::C_SceneNode*, CSLModel*>::Iterator it;
	it = m_aftkXsiSceneNodeMap.Find(pSceneNode);
	if (it == m_aftkXsiSceneNodeMap.End())
	{
		return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Could not map Aftk->Xsi scene node.");
	}

	*ppXsiNode = it->GetValue();
	return Aftk::C_Status::E_Status_Success;
}



Aftk::C_Status C_SceneExport::AddToAftkXsiSceneNodeMapping(
	const Aftk::C_SceneNode* pAftkSceneNode,
	CSLModel* pXsiNode)
{
	m_aftkXsiSceneNodeMap[pAftkSceneNode] = pXsiNode;
	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_SceneExport::GetXsiMaterialFromAftk(
	const Aftk::C_SharedResource<Aftk::C_MaterialNode>* pMaterialResource,
	CSLBaseMaterial** ppXsiMaterial)
{
	Ut::UtMap<const Aftk::C_SharedResource<Aftk::C_MaterialNode>*, CSLBaseMaterial*>::Iterator it;
	it = m_aftkXsiMaterialMap.Find( pMaterialResource );
	if( it == m_aftkXsiMaterialMap.End() )
	{
		return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Could not map Aftk->Xsi material.");
	}

	*ppXsiMaterial = it->GetValue();
	return Aftk::C_Status::E_Status_Success;
}



Aftk::C_Status C_SceneExport::AddToAftkXsiMaterialMapping(
	const Aftk::C_SharedResource<Aftk::C_MaterialNode>* pMaterialResource,
	CSLBaseMaterial* pXsiMaterial)
{
	m_aftkXsiMaterialMap[pMaterialResource] = pXsiMaterial;
	return Aftk::C_Status::E_Status_Success;
}



Aftk::C_Status C_SceneExport::ExportMaterials()
{
	CSLMaterialLibrary* pXsiMaterialLibrary = m_pXsiScene->GetMaterialLibrary();
	pXsiMaterialLibrary->Name().SetText( "MATLIB-Scene" );

	if (m_pProgressNotification)
	{
		m_pProgressNotification->OnPrintProgress("Exporting materials...");
	}
	// Create materials
	{
		U32 nMaterials = m_pAftkScene->GetNumMaterialNodes();

		for( U32 i=0; i<nMaterials; ++i )
		{
			CSLXSIMaterial* pXsiMaterial = pXsiMaterialLibrary->AddXSIMaterial();
			if( !pXsiMaterial )
				return Aftk::C_Status( Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi material" );

			const Aftk::C_MaterialResource* pAftkMaterialResource = m_pAftkScene->GetMaterialResourcePtr( i );

			AddToAftkXsiMaterialMapping( pAftkMaterialResource, pXsiMaterial );

			Aftk::C_Status status = C_MaterialExport::Export( this, pAftkMaterialResource, pXsiMaterial );
			AFTK_CHECK_ERROR_CLEAN_UP_RETURN


			//Update progress...
			m_nProcessingStagesCompleted++;
			F32 progress = ((F32)m_nProcessingStagesCompleted / (F32)m_nProcessingStages) * 100.0f;
			if (m_pProgressNotification)
			{
				if (m_pProgressNotification->OnProgress(progress))
				{
					return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Aftk::C_Scene writing aborted.");
				}
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneExport::ExportSceneHierarchyTopLevel()
{
	if (m_pProgressNotification)
	{
		m_pProgressNotification->OnPrintProgress("Exporting scene hierarchy...");
	}

	const Aftk::C_SceneNode* pAftkRootNode = m_pAftkScene->GetRootSceneNodePtr();
	if (!pAftkRootNode)
	{
		AFTK_DO_BEGIN << "Aftk::C_Scene has no root node!" AFTK_DO_ERROR_RETURN_END_1;
	}
	if( pAftkRootNode->GetNumChildren() == 0 )
		return Aftk::C_Status::E_Status_Success;
	
	Aftk::C_Status status;

	const Aftk::C_SceneNode* pAftkSceneNode = pAftkRootNode->GetChildPtr();

	m_pXsiScene->Root()->SetName( "XsiRoot" );
	CSLSceneInfo* pXsiSceneInfo = m_pXsiScene->SceneInfo();
	pXsiSceneInfo->SetTimingType( CSLSceneInfo::SI_FRAMES ); //CSLSceneInfo::ETimingType::SI_FRAMES
	pXsiSceneInfo->SetStart( m_pAftkScene->GetStartTime() );
	pXsiSceneInfo->SetEnd( m_pAftkScene->GetEndTime() );
	pXsiSceneInfo->SetFrameRate( m_pAftkScene->GetFrameRate() );

	CSLAmbience* pXsiAmbience = m_pXsiScene->Ambience();
	pXsiAmbience->SetColor( CSIBCColorf( 0.2f, 0.2f, 0.2f ) );

	CSLCoordinateSystem* pXsiCoordinateSystem = m_pXsiScene->CoordinateSystem();
	pXsiCoordinateSystem->SetHandRotation( CSLCoordinateSystem::SI_RIGHT_HANDED );
	pXsiCoordinateSystem->SetUAxisOrientation( CSLCoordinateSystem::SI_RIGHT_U );
	pXsiCoordinateSystem->SetVAxisOrientation( CSLCoordinateSystem::SI_UP_V );
	pXsiCoordinateSystem->SetXAxisOrientation( CSLCoordinateSystem::SI_RIGHT );
	pXsiCoordinateSystem->SetYAxisOrientation( CSLCoordinateSystem::SI_UP );
	pXsiCoordinateSystem->SetZAxisOrientation( CSLCoordinateSystem::SI_OUT );

	CSLFileInfo* pXsiFileInfo = m_pXsiScene->FileInfo();
	pXsiFileInfo->SetUsername( "Aftk Xsi Exporter" );
	pXsiFileInfo->SetOriginator( "Bizarre Creations" );

	//C_SceneNodeExport::Export( this, m_pXsiScene->Root(), m_pAftkScene, pAftkRootNode );

	//Update progress...
	m_nProcessingStagesCompleted++;
	F32 progress = ((F32)m_nProcessingStagesCompleted / (F32)m_nProcessingStages) * 100.0f;
	if (m_pProgressNotification)
	{
		if (m_pProgressNotification->OnProgress(progress))
		{
			return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Aftk::C_Scene writing aborted.");
		}
	}

	//Loop thru the siblings of the root node and export their children.
	while (pAftkSceneNode)
	{
		status = ExportSceneHierarchyChildren(pAftkSceneNode, m_pXsiScene->Root() );
		//CHECK_ERROR_RETURN;

		pAftkSceneNode = pAftkSceneNode->GetNextPtr();
	}

	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_SceneExport::ExportSceneHierarchyChildren(
	const Aftk::C_SceneNode* pAftkSceneNode,
	CSLModel* pXsiParentNode)
{
	Aftk::C_Status status;
	CSLModel* pXsiModel;

	switch (pAftkSceneNode->GetType())
	{
		case Aftk::E_ObjType_SceneMesh:
		{
			pXsiModel = pXsiParentNode->AddMesh();
			if( !pXsiModel )
				return Aftk::C_Status( Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi mesh node" );

			status = C_SceneMeshExport::Export(
				this,
				pXsiModel,
				m_pAftkScene,
				pAftkSceneNode);
			break;
		}
		case Aftk::E_ObjType_SceneNode:
		default:
		{
			pXsiModel = pXsiParentNode->AddModel();
			if( !pXsiModel )
				return Aftk::C_Status( Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi scene node" );
		
			status = C_SceneNodeExport::Export(
				this,
				pXsiModel,
				m_pAftkScene,
				pAftkSceneNode);
			break;
		}
	}
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN

	C_AnimationExport::Export( this, pXsiModel, m_pAftkScene, pAftkSceneNode );


	//Update progress...
	m_nProcessingStagesCompleted++;
	F32 progress = ((F32)m_nProcessingStagesCompleted / (F32)m_nProcessingStages) * 100.0f;
	if (m_pProgressNotification)
	{
		if (m_pProgressNotification->OnProgress(progress))
		{
			return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Aftk::C_Scene writing aborted.");
		}
	}


	U32 numChildren = pAftkSceneNode->GetNumChildren();
	for (U32 i=0; i<numChildren; ++i)
	{
		status = ExportSceneHierarchyChildren(
			pAftkSceneNode->GetChildPtr(i), 
			pXsiModel);
			AFTK_CHECK_ERROR_RETURN;
	}

	return status;
}

Aftk::C_Status C_SceneExport::ExportSkinningData()
{
	Aftk::C_SceneNodeIterator it( (Aftk::C_Scene*) m_pAftkScene );

	if (m_pProgressNotification)
	{
		m_pProgressNotification->OnPrintProgress("Exporting skinning data...");
	}

	CSLEnvelopeList *pEnvList = m_pXsiScene->CreateEnvelopeList();
	if( !pEnvList )
		return Aftk::C_Status( Aftk::C_Status::E_Status_OutOfMemory, "Failed to create skinning envelope list" );

	
	// Export skinning envelopes
	while (it)
	{
		Aftk::C_SceneNode* pAftkSceneNode = it;
		switch (pAftkSceneNode->GetType())
		{
			case Aftk::E_ObjType_SceneMesh:
			{
				Aftk::C_SceneMeshNode* pAftkSceneMeshNode = (Aftk::C_SceneMeshNode*)pAftkSceneNode;
				Aftk::C_Status status;
				status = ExportSkinCluster( pAftkSceneMeshNode );
				AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;
				break;
			}
			default:
				break;
		}
		it++;
		//Update progress...
		m_nProcessingStagesCompleted++;
		F32 progress = ((F32)m_nProcessingStagesCompleted / (F32)m_nProcessingStages) * 100.0f;
		if (m_pProgressNotification)
		{
			if (m_pProgressNotification->OnProgress(progress))
			{
				return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Aftk::C_Scene writing aborted.");
			}
		}
	}
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneExport::ExportSkinCluster( const Aftk::C_SceneMeshNode* pAftkSceneNode )
{
	Aftk::C_Status status;

	CSLModel *pXsiSceneNode;

	status = GetXsiSceneNodeFromAftk( pAftkSceneNode, &pXsiSceneNode );
	AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;

	CSLMesh *pXsiMesh = (CSLMesh*) pXsiSceneNode->Primitive();
	CSLBaseShape *pBaseShape = pXsiMesh->Shape();

	CSLEnvelopeList *pXsiEnvelopeList = m_pXsiScene->EnvelopeList();

	U32 nInfluenceClusters = pAftkSceneNode->GetNumInfluenceClusters();
	for( U32 i=0; i<nInfluenceClusters; ++i )
	{
		const Aftk::C_InfluenceClusterNode *pAftkInfluenceCluster = pAftkSceneNode->GetInfluenceClusterPtr( i );
		if( pAftkInfluenceCluster->GetUsage() != AFTK_INFLUENCE_CLUSTER_USAGE_SKIN )
			continue;

		//copy skinning envelopes
		U32 nEnvelopes = pAftkInfluenceCluster->GetNumEnvelopes();
		if( nEnvelopes )
		{
			for( U32 j=0; j<nEnvelopes; ++j )
			{
				const Aftk::C_WeightEnvelope pAftkEnvelope = pAftkInfluenceCluster->GetEnvelope( j );

				CSLEnvelope *pXsiEnvelope = pXsiEnvelopeList->AddEnvelope();
				if( !pXsiEnvelope )
					return Aftk::C_Status( Aftk::C_Status::E_Status_OutOfMemory, "Failed to create new skinning envelope" );

				CSLModel* pXsiDeformer;
				status = GetXsiSceneNodeFromAftk( pAftkEnvelope.GetInfluenceNodePtr(), &pXsiDeformer );
				AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;

				pXsiEnvelope->SetEnvelope( pXsiSceneNode );
				pXsiEnvelope->SetDeformer( pXsiDeformer );

				U32 nWeights = pAftkEnvelope.GetNumWeights();
				if( nWeights > 0 )
				{
					//copy vertex weights
					CSLEnvelope::CSLVertexWeightArray *pXsiVertexWeights = pXsiEnvelope->GetVertexWeightList();
					pXsiVertexWeights->Resize( nWeights );

					for( U32 k=0; k<nWeights; ++k )
					{
						Aftk::C_Weight< F32 > aftkWeight;
						pAftkEnvelope.GetWeight( k, aftkWeight );

						(*pXsiVertexWeights)[k].m_fVertexIndex = (float) aftkWeight.GetIndex();
						(*pXsiVertexWeights)[k].m_fWeight = aftkWeight.GetWeight() * 100.0f;
					}
				}
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}