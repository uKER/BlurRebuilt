#include "stdafx.h"

#include "XsiIoSceneImport.h"
#include "XsiIoMaterialImport.h"
#include "XsiIoSceneNodeImport.h"
#include "XsiIoSceneMeshImport.h"
#include "XsiIoSceneIkNodeImport.h"
#include "XsiIoAnimationImport.h"
#include "XsiIoAnimationCurveImport.h"

using namespace XsiIo;

Aftk::C_Status C_SceneImport::Import(
	CSLScene* pXsiScene, 
	Aftk::C_Scene *pAftkScene,
	const C_ImportOptions* pOptions,
	Aftk::I_Progress* pProgress)
{
	m_pXsiScene = pXsiScene;
	m_pAftkScene = pAftkScene;
	m_pOptions = pOptions;

	Aftk::C_Status status;

	m_numProcessingStagesCompleted = 0;
	m_numProcessingStages = CountXsiSceneNodes(pXsiScene->Root());
	if (pXsiScene->GetMaterialLibrary())
	{
		m_numProcessingStages += pXsiScene->GetMaterialLibrary()->GetMaterialCount();
	}

	pAftkScene->Clear();

	m_materialMap.Clear();
	m_xsiAftkSceneNodeMapping.Clear();
	m_aftkXsiSceneNodeMapping.Clear();

	//Set scene transform
	if (pAftkScene->GetRootSceneNodePtr())
	{
		Aftk::Matrix44 sceneTransform;
		SetIdentity(&sceneTransform);
		sceneTransform[2] *= -1.f;
		pAftkScene->GetRootSceneNodePtr()->SetLocalMatrix(
			sceneTransform * pAftkScene->GetRootSceneNodePtr()->GetLocalMatrix(false));
	}

	//Convert materials
	status = ConvertMaterials(
		pXsiScene,
		pAftkScene, 
		pProgress);
	AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_CLEAN_UP_RETURN_END

	//Convert nodes
	Aftk::C_SceneNode* pAftkRootNode = ConvertNode(
		pXsiScene->Root(), 
		pProgress,
		status);
	AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_CLEAN_UP_RETURN_END

	//Set Root node
	pAftkScene->SetRootSceneNodePtr(pAftkRootNode);

	//Import skinning information
	status = ImportSkinClusters(pAftkScene);
	AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_CLEAN_UP_RETURN_END;

	pAftkScene->Optimise();

	//Set UV linking info
	//Loop through all meshes
	Aftk::C_SceneNodeIterator it(pAftkScene);
	while (it) 
	{
		Aftk::C_SceneNode* pNode = it;
		if (pNode->HasType(Aftk::E_ObjType_SceneMesh)) {

			Aftk::C_SceneMeshNode* pMesh = (Aftk::C_SceneMeshNode*) pNode;

			// Loop through all mesh materials
			for (U16 iMaterial = 0; iMaterial < pMesh->GetNumPolygonMaterialMappings(); iMaterial++) {

				const Aftk::C_MaterialResource* pMatRes = pMesh->GetPolygonMaterialMappingPtr(iMaterial);
				const Aftk::C_MaterialNode* pMaterial = &pMatRes->GetContent();

				// Loop through all shader image nodes on material
				for (U32 jImage = 0; jImage < pMaterial->GetNumShaderImageNodes(); jImage++) {

					const Aftk::C_ShaderImageNode* pImage = pMaterial->GetShaderImageNodePtr(jImage);

					// Get connection between image node and whatever shader uses it
					const Aftk::C_ShaderNode* pShader = 
						GetShaderDestinationConnection(pImage);
					if (!pShader)
						continue;
						
					// Find the "tspace_id" plug on the shader node connected to the image node.
					const Aftk::I_Plug* pPlug = pShader->GetPlugPtr(std::string("tspace_id"));
					if (!pPlug)
						continue;

					// The "tspace_id" plug stores the name of the UV set to use.
					// Find the index of the UV set by name and store this mapping.	
					if (pPlug->GetDataType() == Aftk::E_GenericAttributeType_String) {
						std::string uvSetName;
						pPlug->GetValue(uvSetName);
						S32 index = pMesh->GetTextureCoordinateSetIndex(uvSetName);
						pMesh->AddUVSetIndex(pMatRes, pImage, index);
					}
				}
			}
		}
		it++;
	}

	return Aftk::C_Status::E_Status_Success;
}

//Each image node can only be connected to one shader node at most - so return the first shader node
//we encounter.
const Aftk::C_ShaderNode* C_SceneImport::GetShaderDestinationConnection(
	const Aftk::C_ShaderImageNode* pImage) 
{
	const Ut::UtVector<Ut::DynamicVector<Aftk::I_Plug*> >& connectedPlugs = 
		pImage->GetConnectedPlugs();
	for (U32 i = 0; i < (U32)connectedPlugs.Size(); i++)
	{
		Aftk::I_Object* pObj = connectedPlugs[i]->GetSourceConnectionPtr();
		if (pObj && pObj->HasType(Aftk::E_ObjType_Shader))
			return (Aftk::C_ShaderNode*)pObj;
	}

	return 0;
}

const Aftk::C_SharedResource<Aftk::C_MaterialNode>* C_SceneImport::GetAftkMaterialFromXsi(
	CSLBaseMaterial* pXsiMaterial,
	Aftk::C_Status& status)
{
	if (pXsiMaterial == 0)
	{
		status.SetErrorCode(Aftk::C_Status::E_Status_Success);
		return 0;
	}

	MaterialMap::Iterator it;
	it = m_materialMap.Find(pXsiMaterial);
	if (it == m_materialMap.End())
	{
		status.SetErrorCode(Aftk::C_Status::E_Status_Failure);
		status << "Could not find material node " << pXsiMaterial->GetName();
		return 0;
	}

	status = Aftk::C_Status::E_Status_Success;
	return it->GetValue();
}

Aftk::C_Status C_SceneImport::ConvertMaterials(
	CSLScene* pXsiScene, 
	Aftk::C_Scene *pAftkScene,
	Aftk::I_Progress* pProgressCallback)
{
	int nMaterials = pXsiScene->GetMaterialLibrary()->GetMaterialCount();
	CSLBaseMaterial** materialList = pXsiScene->GetMaterialLibrary()->GetMaterialList();

	for (int i = 0; i < nMaterials; i++)
	{
		Aftk::C_MaterialNode aftkMaterialNode;

		Aftk::C_Status status;
		status = C_MaterialImport::Import(
			this,
			materialList[i],
			&aftkMaterialNode);
		AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;;

		const Aftk::C_SharedResource<Aftk::C_MaterialNode>* material = 
			pAftkScene->AddMaterialNode(aftkMaterialNode, true);
		m_materialMap[materialList[i]] = material;

		{
			//Mike: Remap material ptrs
		}

		//Update progress...
		m_numProcessingStagesCompleted++;
		F32 progress = ((F32)m_numProcessingStagesCompleted / (F32)m_numProcessingStages) * 100.0f;
		if (pProgressCallback)
		{
			if (pProgressCallback->OnProgress(progress))
			{
				return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Aftk::C_Scene reading aborted.");
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_SceneNode* C_SceneImport::ConvertNode(
	CSLModel* pXsiNode, 
	Aftk::I_Progress* pProgress,
	Aftk::C_Status& status)
{
	Aftk::C_SceneNode* pAftkSceneNode;

	CSLTemplate::ETemplateType type = pXsiNode->GetPrimitiveType();

	switch (type) 
	{
		case CSLTemplate::SI_MESH:
		{
			pAftkSceneNode = new Aftk::C_SceneMeshNode(m_pAftkScene);
			if (!pAftkSceneNode)
			{
				status.Set(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create new mesh node.", 1);
				return 0;
			}
			status = C_SceneMeshNodeImport::Import(
				this,
				pXsiNode, 
				pAftkSceneNode);
			if (!status)
			{
				//Returning the error reported by the importer.
				return 0;
			}

			break;
		}
		case CSLTemplate::SI_IK_ROOT:
		case CSLTemplate::SI_IK_JOINT:
		case CSLTemplate::SI_IK_EFFECTOR:
		{
			pAftkSceneNode = new Aftk::C_SceneJointNode(m_pAftkScene);
			if (!pAftkSceneNode)
			{
				status.Set(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create new joint node", 1);
				return 0;
			}
			status = C_SceneIkNodeImport::Import(
				this,
				pXsiNode,
				pAftkSceneNode);
			if (!status)
			{
				return 0;
			}
			break;
		}
		default:
		{
			pAftkSceneNode = new Aftk::C_SceneNode(m_pAftkScene);
			if (!pAftkSceneNode)
			{
				status.Set(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create new scene ndoe.", 1);
				return 0;
			}

			status = C_SceneNodeImport::Import(
				this,
				pXsiNode,
				pAftkSceneNode);
			if (!status)
			{
				//Returning the error reported by the importer.
				return 0;
			}

			break;
		}
	}

	if (pAftkSceneNode)
	{
		status = AnimationImport::Import(
			this,
			pXsiNode,
			pAftkSceneNode);
		if (!status)
		{
			//Returning the error reported by the importer.
			return 0;
		}
	}

	//Update progress...
	m_numProcessingStagesCompleted++;
	F32 progress = ((F32)m_numProcessingStagesCompleted / (F32)m_numProcessingStages) * 100.0f;
	if (pProgress)
	{
		if (pProgress->OnProgress(progress))
		{
			status = Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Aftk::C_Scene reading aborted.");
			return 0;
		}
	}

	//Establish hierachy relationships
	if (pXsiNode->GetChildrenCount() > 0)
	{
		CSLModel** childList = pXsiNode->GetChildrenList();

		Aftk::C_SceneNode* pAftkSceneNodeChild = 
			ConvertNode(
			childList[0], 
			pProgress,
			status);
		if (!status)
		{
			//Return the error reported by the child exporter.
			return 0;
		}

		pAftkSceneNode->SetChildPtr(pAftkSceneNodeChild);

		for (int i = 1; i < pXsiNode->GetChildrenCount(); i++ )
		{
			Aftk::C_SceneNode* pAftkSceneNodeNext = ConvertNode(
				childList[i], 
				pProgress,
				status);
			if (!status)
			{
				//Return the error reported by the child exporter.
				return 0;
			}

			//pAftkSceneNodeChild->InsertNextPtr(pAftkSceneNodeNext);
			pAftkSceneNode->AddChildPtr(pAftkSceneNodeNext);
			pAftkSceneNodeChild = pAftkSceneNodeNext;
		}
	}

	return pAftkSceneNode;
}

U32 C_SceneImport::CountXsiSceneNodes(CSLModel* pXsiNode)
{
	U32 count = 1;

	for (S32 i = 0; i < pXsiNode->GetChildrenCount(); i++ )
	{
		count += CountXsiSceneNodes(pXsiNode->GetChildrenList()[i]);
	}

	return count;
}

Aftk::C_Status C_SceneImport::ImportSkinClusters(Aftk::C_Scene* pAftkScene)
{
	Aftk::C_SceneNodeIterator it(pAftkScene);
	while (it)
	{
		Aftk::C_SceneNode* pAftkSceneNode = it;
		switch (pAftkSceneNode->GetType())
		{
			case Aftk::E_ObjType_SceneMesh:
			{
				Aftk::C_SceneMeshNode* pAftkSceneMeshNode = (Aftk::C_SceneMeshNode*)pAftkSceneNode;
				//If needed, import skin cluster information.
				Aftk::C_Status status;
				status = ImportSkinCluster(pAftkSceneMeshNode);
				AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;
			}
			break;

			default:
				break;
		}

		it++;
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneImport::ImportSkinCluster(Aftk::C_SceneNode* pAftkSceneNode)
{
	Aftk::C_Status status;

	Aftk::C_Scene* pAftkScene = pAftkSceneNode->GetScenePtr();

	CSLModel* pXsiNode;
	status = GetXsiFromAftkSceneNode(pAftkSceneNode, &pXsiNode);
	AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;

	CSLMesh* pXsiMesh = (CSLMesh*)pXsiNode->Primitive();
	CSLBaseShape* pBaseShape = pXsiMesh->Shape();
	
	U32 numEnvelopes = pXsiNode->GetEnvelopeCount();
	CSLEnvelope** ppEnvelopes = pXsiNode->GetEnvelopeList();
	if (numEnvelopes > 0)
	{
		//Skinning envelopes are unique to meshes within XSI. If we have envelopes, then
		//we must first create a skin cluster.
		Aftk::C_InfluenceClusterNode* pAftkInfluenceCluster = new Aftk::C_InfluenceClusterNode(pAftkScene);

		pAftkInfluenceCluster->SetUsage(AFTK_INFLUENCE_CLUSTER_USAGE_SKIN);

		//Associate mesh with skin cluster.
		pAftkInfluenceCluster->SetSceneNodePtr(pAftkSceneNode);

		pAftkInfluenceCluster->SetEnvelopeInitialCapacity(numEnvelopes);

		//Copy skinning envelopes
		for (U32 iEnvelope = 0; iEnvelope < numEnvelopes; iEnvelope++)
		{
			CSLEnvelope* pXsiEnvelope = ppEnvelopes[iEnvelope];
			CSLModel* pXsiInfluenceNode = pXsiEnvelope->GetDeformer();
			int				noWeights	= pXsiEnvelope->GetVertexWeightCount();
			SLVertexWeight*	weights		= pXsiEnvelope->GetVertexWeightListPtr();
			
/*
			Chris C recommended that empty envelopes be left in.
			if (noWeights <= 0)
			{
				// Ignore envelopes with no weights - different parts may not use all bones.
				continue;
			}
*/

			Aftk::C_SceneNode* pAftkInfluenceNode;
			status = GetAftkSceneNodeFromXsi(pXsiInfluenceNode, &pAftkInfluenceNode);
			AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;

			//Create new envelope
			U32 aftkEnvelopeIndex = pAftkInfluenceCluster->AddEnvelope();
			Aftk::C_WeightEnvelope* pAftkEnvelope = &pAftkInfluenceCluster->GetEnvelope(aftkEnvelopeIndex);
			//Set influence object (aka bone)
			pAftkEnvelope->SetInfluenceNodePtr(pAftkInfluenceNode);

			//Copy vertex weighting information.
			pAftkEnvelope->SetWeightInitialCapacity(noWeights);
			for(U32 jWeight = 0; jWeight < (U32)noWeights; jWeight++)
			{
				S32	vertexNo	= (S32)weights[jWeight].m_fVertexIndex;
				F32	weight		= weights[jWeight].m_fWeight;
				//In XSI, vertex weights are expressed as a percentage. In the AFTK, we express
				//weights as values between 0 and 1 typically.
				weight *= 0.01f;

				if (weight > AFTK_EPSILON)
				{
					Aftk::C_Weight<F32> aftkWeight;
					aftkWeight.SetIndex(vertexNo);
					aftkWeight.SetWeight(weight);

					pAftkEnvelope->AddWeight(aftkWeight);
				}
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneImport::GetAftkSceneNodeFromXsi(
	CSLModel* pXsiNode,
	Aftk::C_SceneNode** ppAftkSceneNode)
{
	Ut::UtMap<CSLModel*, Aftk::C_SceneNode*>::Iterator it;
	it = m_xsiAftkSceneNodeMapping.Find(pXsiNode);
	if (it == m_xsiAftkSceneNodeMapping.End())
	{
		return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Could not map Xsi->Aftk scene node.");
	}

	*ppAftkSceneNode = it->GetValue();
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneImport::GetXsiFromAftkSceneNode(
	const Aftk::C_SceneNode* pSceneNode,
	CSLModel** ppXsiNode)
{
	Ut::UtMap<const Aftk::C_SceneNode*, CSLModel*>::Iterator it;
	it = m_aftkXsiSceneNodeMapping.Find(pSceneNode);
	if (it == m_aftkXsiSceneNodeMapping.End())
	{
		return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Could not map Aftk->Xsi scene node.");
	}

	*ppXsiNode = it->GetValue();
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneImport::AddToXsiAftkSceneNodeMapping(
	CSLModel* pXsiNode,
	Aftk::C_SceneNode* pAftkSceneNode)
{
	m_xsiAftkSceneNodeMapping[pXsiNode] = pAftkSceneNode;
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneImport::AddToAftkXsiSceneNodeMapping(
	const Aftk::C_SceneNode* pAftkSceneNode,
	CSLModel* pXsiNode)
{
	m_aftkXsiSceneNodeMapping[pAftkSceneNode] = pXsiNode;
	return Aftk::C_Status::E_Status_Success;
}

CSLScene* C_SceneImport::GetXsiScenePtr()
{
	return m_pXsiScene;
}

Aftk::C_Scene* C_SceneImport::GetAftkScenePtr()
{
	return m_pAftkScene;
}

const C_ImportOptions* C_SceneImport::GetOptionsPtr() const
{
	return m_pOptions;
}

void C_SceneImport::CleanUpAfterFailure()
{
}
