
#include "stdafx.h"

#include <converter_shared/helpers/ConvertSmallInstancesToMesh.h>
#include <AftkUt/AftkUtNodeFlagParser.h>

#include <converter_shared/helpers/AftkHelpers.h>

const bool C_ConvertSmallInstancesToMesh::ConvertSmallInstancesToMesh( Aftk::C_Scene* pAftkScene )
{
	Ut::Vector<Aftk::C_SceneMeshNode*> meshPtrList;

	for( Aftk::C_SceneNodeIterator iter(pAftkScene); iter; ++iter)
	{
		Aftk::C_SceneNode* pSceneNode = iter;
		if( pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
		{
			const int NUM_INSTANCE_CUT_OFF = 400;
			const int NUM_POLYGON_CUT_OFF_1 = 40;
			const int NUM_POLYGON_CUT_OFF_2 = 30;

			const bool isCollisionNode = (AftkHelpers::GetCollisionType( pSceneNode ) != AftkHelpers::E_CollisionType_None);

			Aftk::C_SceneMeshNode* pSceneMeshNode = (Aftk::C_SceneMeshNode*)pSceneNode;

			const bool generatesResonableAmountOfMesh = (pSceneMeshNode->GetNumInstancesOfThisNode() < NUM_INSTANCE_CUT_OFF 
						&& pSceneMeshNode->GetNumPolygons(0) < NUM_POLYGON_CUT_OFF_1); // expand if it won't generate too much mesh

			const bool veryLowPolygonCount = pSceneMeshNode->GetNumPolygons(0) < NUM_POLYGON_CUT_OFF_2; // if very low polygon count always expand				

			if( (generatesResonableAmountOfMesh || veryLowPolygonCount) && !isCollisionNode) // this is what the logic should be
			//if( (generatesResonableAmountOfMesh || veryLowPolygonCount) || isCollisionNode)
			{
				meshPtrList.PushBack( pSceneMeshNode );
			}
		}
	}
	
	bool success = true;
	
	const S32 meshCount = meshPtrList.Size();
	for(S32 meshNo=0; meshNo<meshCount; meshNo++)
	{
		success = ConvertSmallInstanceToMesh( meshPtrList[meshNo] ) && success;
	}

	return success;
}

Aftk::C_SceneMeshNode* ConvertInstanceToMesh(Aftk::C_SceneInstanceNode* pInstanceNode)
{
	Aftk::C_Scene* pAftkScene = pInstanceNode->GetScenePtr();
	Aftk::C_SceneMeshNode* pMeshNode = (Aftk::C_SceneMeshNode*)pInstanceNode->GetSourceNodePtr();
	Aftk::C_SceneMeshNode* pNewNode = (Aftk::C_SceneMeshNode*)pMeshNode->Clone( pAftkScene );

	assert( pMeshNode->GetType() == Aftk::E_ObjType_SceneMesh );

	Util::C_String instanceName = pInstanceNode->GetName().c_str();
	pNewNode->SetLocalMatrix( pInstanceNode->GetLocalMatrix() );

	// remove new node from all layers
	while( pNewNode->GetNumLayers() )
	{
		Aftk::C_LayerNode* pLayerNode = pNewNode->GetLayerPtr( 0 );
		pLayerNode->DeleteMember( pNewNode );
	}

	// add new node to instace layers
	const U32 layerCount = pInstanceNode->GetNumLayers();
	for( U32 layerNo=0; layerNo<layerCount; layerNo++)
	{
		Aftk::C_LayerNode* pLayerNode = pInstanceNode->GetLayerPtr( layerNo );
		pLayerNode->AddMember(pNewNode);
	}

	pInstanceNode->GetParentPtr()->AddChildPtr( pNewNode );
	if (pInstanceNode->GetParentPtr()->HasType(Aftk::E_ObjType_SceneLodGroup))
	{
		Aftk::C_SceneLodGroupNode* pLodGroup = (Aftk::C_SceneLodGroupNode*)pInstanceNode->GetParentPtr();
		S32 lodIdx = pLodGroup->GetLodLevelIndex(pInstanceNode);
		if (lodIdx != -1)
		{
			Aftk::C_GroupLodLevelInfo& lodInfo = pLodGroup->GetLodLevelInfo(lodIdx);
			lodInfo.SetSceneNodePtr(pNewNode);
		}
	}

	// TODO: Properly reparent child nodes
	pAftkScene->DeleteSceneNodeAndReparentChildren( pInstanceNode, true );

	pNewNode->SetName( instanceName.Chars() );

	return pNewNode;
}

enum E_LightingType
{
	E_LightingType_SH = 0,
	E_LightingType_Vertex,
	E_LightingType_Lightmap,
	E_LighitngType_None
};

E_LightingType GetLightingType(Aftk::C_SceneInstanceNode* pInstanceNode)
{
	Aftk::C_SceneNode* pSourceNode = pInstanceNode->GetSourceNodePtr();

	AftkUt::C_NodeFlagParser instanceNodeFlags(*pInstanceNode);
	AftkUt::C_NodeFlagParser sourceNodeFlags(*pSourceNode);
	
	bool result = false;
    if (instanceNodeFlags.GetParameterValue("UseSHLighting", result))
    {
		if (result)
			return E_LightingType_SH; // it was set to true
    }

	// get the vertex lighting value
    if (instanceNodeFlags.GetParameterValue("UseVertexLighting", result))
    {
        if (result)
			return E_LightingType_Vertex; // it was set to true
    }

	// get the lightmap lighting value
    if (instanceNodeFlags.GetParameterValue("UseLightmapLighting", result))
    {
		if (result)
			return E_LightingType_Lightmap; // it was set to true
    }

    // get the lightmap lighting value
    if (instanceNodeFlags.GetParameterValue("DoNotLight", result))
	{
		if (result)
			return E_LighitngType_None; // it was set to true
	}

	
    if (sourceNodeFlags.GetParameterValue("UseSHLighting", result))
    {
		if (result)
			return E_LightingType_SH; // it was set to true
    }

	// get the vertex lighting value
    if (sourceNodeFlags.GetParameterValue("UseVertexLighting", result))
    {
        if (result)
			return E_LightingType_Vertex; // it was set to true
    }

	// get the lightmap lighting value
    if (sourceNodeFlags.GetParameterValue("UseLightmapLighting", result))
    {
		if (result)
			return E_LightingType_Lightmap; // it was set to true
    }

    // get the lightmap lighting value
    if (sourceNodeFlags.GetParameterValue("DoNotLight", result))
	{
		if (result)
			return E_LighitngType_None; // it was set to true
	}

	return E_LightingType_Vertex;
}


const bool C_ConvertSmallInstancesToMesh::ConvertSmallInstanceToMesh( Aftk::C_SceneMeshNode* pMeshNode )
{
	bool success = true;

	if (!pMeshNode)
		return false;

	U32 counter = 0;

	while((pMeshNode->GetNumInstancesOfThisNode() != 0) && (counter < pMeshNode->GetNumInstancesOfThisNode()))
	{
		Aftk::C_SceneInstanceNode* pInstanceNode = (Aftk::C_SceneInstanceNode*)pMeshNode->GetInstanceOfThisNode( counter );

		E_LightingType lightingType = GetLightingType(pInstanceNode);

		if (E_LightingType_SH != lightingType)
		{
			Aftk::C_SceneMeshNode* pNewNode = ConvertInstanceToMesh( pInstanceNode );
			AftkUt::C_NodeFlagParser newNodeFlags(*pNewNode);
			newNodeFlags.SetParameterValue("UseVertexLighting", true);
			newNodeFlags.SetParameterValue("UseSHLighting", false);
		}
		else
		{
			LOGDEBUG( Tools::g_log, "C_ConvertSmallInstancesToMesh: Not expanding %s as it uses spherical harmonic lighting",
				pInstanceNode->GetName().c_str() );
			
			counter++;
		}
	}	

	return success;
}

//void C_MeshFlattenSmallInstances::FlattenSmallInstances( Aftk::C_SceneMeshNode* pMeshNode )
//{
//	Aftk::C_Scene* pAftkScene = pMeshNode->GetScenePtr();
//
//	while(pMeshNode->GetNumInstancesOfThisNode() != 0)
//	{
//		Aftk::C_SceneInstanceNode* pInstanceNode = (Aftk::C_SceneInstanceNode*)pMeshNode->GetInstanceOfThisNode( 0 );
//
//		Aftk::C_SceneMeshNode* pNewNode = (Aftk::C_SceneMeshNode*)pMeshNode->Clone( pAftkScene );
//
//		pNewNode->SetLocalMatrix( pInstanceNode->GetLocalMatrix() );
//
//		pInstanceNode->GetParentPtr()->AddChildPtr( pNewNode );
//
//		pAftkScene->DeleteSceneNodeAndReparentChildren( pInstanceNode, true );
//	}	
//}
//
//const bool C_MeshFlattenSmallInstances::FlattenSmallInstances( Aftk::C_Scene* pAftkScene, const U32 minPolyCount )
//{
//	Ut::Vector<Aftk::C_SceneMeshNode*> meshPtrList;
//
//	for( Aftk::C_SceneNodeIterator iter(pAftkScene); iter; ++iter)
//	{
//		Aftk::C_SceneNode* pSceneNode = iter;
//		if( pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
//		{
//			Aftk::C_SceneMeshNode* pSceneMeshNode = (Aftk::C_SceneMeshNode*)pSceneNode;
//			if( pSceneMeshNode->GetNumInstancesOfThisNode() )
//			{
//				const U32 lodLevel = 0;
//				if( pSceneMeshNode->GetNumPolygons( lodLevel ) < minPolyCount )
//				{
//					meshPtrList.PushBack( pSceneMeshNode );
//				}
//			}
//		}
//	}
//	
//	const S32 meshCount = meshPtrList.Size();
//	for(S32 meshNo=0; meshNo<meshCount; meshNo++)
//	{
//		FlattenSmallInstances( meshPtrList[meshNo] );
//	}
//
//	return true;
//}

/*
///////////////////////////////////////////////////////////////////////////
// GMIG version
///////////////////////////////////////////////////////////////////////////

void ConvertSmallInstancesToMesh::operator () (Aftk::C_SceneInstanceNode& instance)
{
	Aftk::C_SceneNode* pSource = instance.GetSourceNodePtr();
	if (!pSource || !pSource->HasType(Aftk::E_ObjType_ISceneMesh))
		return;

	Process(instance, (Aftk::I_SceneMeshNode&)*pSource);
}

void ConvertSmallInstancesToMesh::operator () (Aftk::C_SceneMeshBonedInstanceNode& instance)
{
	Aftk::C_SceneNode* pSource = instance.GetSourceNodePtr();
	if (!pSource || !pSource->HasType(Aftk::E_ObjType_ISceneMesh))
		return;

	Process(instance, (Aftk::I_SceneMeshNode&)*pSource);
}

void ConvertSmallInstancesToMesh::operator () (Aftk::C_SceneNode& node)
{
	if (!node.HasType(Aftk::E_ObjType_ISceneMesh))
		return;

	Process(node, node);
}

void ConvertSmallInstancesToMesh::Process(Aftk::C_SceneNode& instance, const Aftk::C_SceneNode& source)
{
	const int NUM_INSTANCE_CUT_OFF = 400;
	const int NUM_POLYGON_CUT_OFF_1 = 20;
	const int NUM_POLYGON_CUT_OFF_2 = 4;

	Aftk::C_SceneMeshRemappableNode& meshInterface = (Aftk::C_SceneMeshRemappableNode&)source;

	if ((source.GetNumInstancesOfThisNode() < NUM_INSTANCE_CUT_OFF 
				&& meshInterface.GetNumPolygons(0) < NUM_POLYGON_CUT_OFF_1) // expand if it won't generate too much mesh
		|| meshInterface.GetNumPolygons(0) < NUM_POLYGON_CUT_OFF_2 // if very low polygon count always expand
		|| AftkUtility::GetLod(instance) > 0 // always expand low lods
		)
	{
		Aftk::C_SceneMeshNode* pCopy = 0;

		if (instance.GetType() == Aftk::E_ObjType_SceneMeshBonedInstance)
		{
			if (!AftkUt::C_ActorParser::IsActor(instance)) // don't convert actors to mesh.
			{
				Aftk::C_SceneMeshBonedInstanceNode& bonedInstance = 
					(Aftk::C_SceneMeshBonedInstanceNode&)instance;
				bonedInstance.SetEvaluateNormalInverseTranspose(false);
				pCopy = new Aftk::C_SceneMeshNode(bonedInstance);
				bonedInstance.SetSourceNodePtr(0);

				// barrier low-lods get vertex-lit
				AftkUt::C_ActorParser actor(*pCopy);
				actor.SetParameterValue("UseVertexLighting", AftkUt::C_ActorParser::E_ParamType_Bool, "1");
			}
		}
		else if (instance.GetType() == Aftk::E_ObjType_SceneInstance)
		{
			if (!AftkUt::C_ActorParser::IsActor(instance)) // don't convert actors to mesh.
			{
				const Aftk::C_SceneMeshNode& mesh = 
					(Aftk::C_SceneMeshNode&)source;
				pCopy = new Aftk::C_SceneMeshNode(mesh);

				Aftk::C_SceneInstanceNode& instanceNode = 
					(Aftk::C_SceneInstanceNode&)instance;
				instanceNode.SetSourceNodePtr(0);

				// all other instances get vertex-lit also
				AftkUt::C_ActorParser actor(*pCopy);
				actor.SetParameterValue("UseVertexLighting", AftkUt::C_ActorParser::E_ParamType_Bool, "1");
			}
		}
		else if (source.GetNumInstancesOfThisNode() > 0) // must be the base instance
		{
			LOGDEBUG(g_Log, "InstanceLOD: '%s' has %d instances and contains %d polygons (%d extra polygons)",
				instance.GetName().c_str(),
				source.GetNumInstancesOfThisNode(),
				meshInterface.GetNumPolygons(0),
				source.GetNumInstancesOfThisNode()*meshInterface.GetNumPolygons(0));
		}
		else if (AftkUtility::GetLod(instance) > 0)
		{
			// all other instances get vertex-lit also
			AftkUt::C_ActorParser actor(instance);
			actor.SetParameterValue("UseVertexLighting", AftkUt::C_ActorParser::E_ParamType_Bool, "1");
		}

		if (pCopy)
		{
			// don't copy base actor
			AftkUt::C_ActorParser::DeleteActorString(*pCopy);

			Aftk::Matrix44 instanceWorldMatrix = instance.GetWorldMatrix(false);

			pCopy->Replace(&instance);
			pCopy->SetWorldMatrix(instanceWorldMatrix, true);
		}
	}
	else if (AftkUtility::GetLod(instance) > 0)
	{
		// all other instances get vertex-lit also
		AftkUt::C_ActorParser actor(instance);
		actor.SetParameterValue("UseVertexLighting", AftkUt::C_ActorParser::E_ParamType_Bool, "1");
	}
}

//////////////////////////////////////////////////////////////////////////
*/
