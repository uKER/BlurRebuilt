#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeUtility.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativeInfluenceClusterNode.h"
#include "NativeWeightEnvelope.h"
#include "NativeIObject.h"
#include "NativeNode.h"

using namespace Aftk;

C_Status C_NativeInfluenceClusterNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_InfluenceClusterNode* pInfluenceCluster,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders == true)
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_INFLUENCE_CLUSTER_NODE);

	//Write out the fragment of this node that comes from C_Node
	{
		//Setting false means that we don't use unnecessary block padding.
		//This is done simply to save disk space.
		status = C_NativeNode::Serialise(
			pDiskWriter, 
			pNativeFtk, 
			(C_InfluenceClusterNode*)pInfluenceCluster,
			true);
		if (!status) return status;

		//Write out the id of the node we influence.
		*pDiskWriter <<	pNativeFtk->GetIdFromPtr(pInfluenceCluster->m_pSceneNode);
	}
	{
		*pDiskWriter << (U32)pInfluenceCluster->GetNumEnvelopes();

		for (U32 i = 0; i < pInfluenceCluster->GetNumEnvelopes(); i++)
		{
			const C_WeightEnvelope* pWeightEnvelope = &pInfluenceCluster->GetEnvelope(i);

			status = C_NativeWeightEnvelope::Serialise(
				pDiskWriter,
				pNativeFtk,
				pWeightEnvelope,
				true);
			if (!status) return status;
		}

		//Write out the string describing the purpose of this influence cluster.
		*pDiskWriter << pInfluenceCluster->GetUsage();
	}
	if (useOuterBlockHeaders)
		pDiskWriter->EndBlock();
	return C_Status::E_Status_Success;
}

C_Status C_NativeInfluenceClusterNode::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_InfluenceClusterNode* pInfluenceCluster,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_INFLUENCE_CLUSTER_NODE))
			return C_Status(C_Status::E_Status_Failure, "Failed to find influence cluster.");
		pDiskReader->EnterBlock();
	}
	if (pDiskReader->GetVersion() >= 200)
	{
		//Read in the fragment of this node that comes from C_Node
		status = C_NativeNode::Deserialise(
			pDiskReader, 
			pNativeFtk, 
			(C_Node*)pInfluenceCluster, 
			true);
		if (!status) return status;

		#pragma warning(push)
		{
			#pragma warning(disable: 4312)
			S32 sceneNodeInteger;
			*pDiskReader >>	sceneNodeInteger;
			pInfluenceCluster->m_pSceneNode = reinterpret_cast<C_SceneNode*>(sceneNodeInteger);
		}
		#pragma warning(pop)
	}
	{
		U32 numWeightEnvelopes;
		*pDiskReader >> numWeightEnvelopes;
		pInfluenceCluster->SetEnvelopeInitialCapacity(numWeightEnvelopes);
		for (U32 i = 0; i < numWeightEnvelopes; i++)
		{
			pInfluenceCluster->AddEnvelope();
			C_WeightEnvelope* pWeightEnvelope = &pInfluenceCluster->GetEnvelope(i);

			status = C_NativeWeightEnvelope::Deserialise(
				pDiskReader,
				pNativeFtk,
				pWeightEnvelope,
				true);
			if (!status) return status;
		}

		if (pDiskReader->GetVersion() >= 139)
		{
			std::string temp;
			*pDiskReader >> temp;
			pInfluenceCluster->SetUsage(temp);
		}
	}
	if (useOuterBlockHeaders)
		pDiskReader->ExitBlock();

	return C_Status::E_Status_Success;
}

bool C_NativeInfluenceClusterNode::ResolveUnmanagedDgNodePointers(
	U32 version,
	C_NativeFtk* pNativeFtk, 
	C_InfluenceClusterNode* pInfluenceCluster)
{
#pragma warning(push)
#pragma warning(disable: 4311)

	//Resolve our connection to the node we influence.
	S32 sceneNodeInteger = 
		reinterpret_cast<S32>(pInfluenceCluster->m_pSceneNode);
	pInfluenceCluster->m_pSceneNode = 0;
	pInfluenceCluster->SetSceneNodePtr(
			(C_SceneNode*)pNativeFtk->GetPtrFromId(sceneNodeInteger));

	//Resolve skin cluster influence pointers
	for (U32 j = 0; j < pInfluenceCluster->GetNumEnvelopes(); j++)
	{
		C_WeightEnvelope* pWeightEnvelope = &pInfluenceCluster->GetEnvelope(j);
		if (!C_NativeWeightEnvelope::ResolveUnmanagedDgNodePointers(
			version,
			pNativeFtk,
			pWeightEnvelope))
			return false;
	}
#pragma warning(pop)
	return true;
}

bool C_NativeInfluenceClusterNode::ResolveUnmanagedDgNodePointers176(
	U32 version,
	C_NativeFtk* pNativeFtk, 
	C_InfluenceClusterNode* pInfluenceCluster)
{
#pragma warning(push)
#pragma warning(disable: 4311)

	//Resolve skin cluster influence pointers
	for (U32 j = 0; j < pInfluenceCluster->GetNumEnvelopes(); j++)
	{
		C_WeightEnvelope* pWeightEnvelope = &pInfluenceCluster->GetEnvelope(j);
		if (!C_NativeWeightEnvelope::ResolveUnmanagedDgNodePointers(
			version,
			pNativeFtk,
			pWeightEnvelope))
			return false;
	}
#pragma warning(pop)
	return true;
}
