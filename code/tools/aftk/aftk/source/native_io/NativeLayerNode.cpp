#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeUtility.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeLayerNode.h"
#include "NativeGenericAttribute.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeFtk.h"

using namespace Aftk;

C_Status C_NativeLayerNode::Serialise(
	C_NativeDiskWriter* pDiskWriter,
	C_NativeFtk* pNativeFtk, 
	const C_LayerNode* pLayer, 
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders)
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_LAYER_NODE);

	//Write out the fragment of this node that comes from C_Node
	{
		//Setting false means that we don't use unnecessary block padding.
		//This is done simply to save disk space.
		status = C_NativeNode::Serialise(
			pDiskWriter, 
			pNativeFtk, 
			(C_Node*)pLayer,
			true);
		if (!status) return status;
	}
	{
		*pDiskWriter << (U32)pLayer->m_memberVector.Size();
		for (U32 iMember = 0; iMember < (U32)pLayer->m_memberVector.Size(); iMember++)
			*pDiskWriter <<	pNativeFtk->GetIdFromPtr(pLayer->m_memberVector[iMember]);
	}

	if (useOuterBlockHeaders)
		pDiskWriter->EndBlock();
	return C_Status::E_Status_Success;
}

C_Status C_NativeLayerNode::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_LayerNode* pLayer,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_LAYER_NODE))
			return C_Status(C_Status::E_Status_Failure, "Failed to find layer block.");
		pDiskReader->EnterBlock();
	}
	{
		if (pDiskReader->GetVersion() < 200)
		{
			//Read name
			std::string temp;
			*pDiskReader >> temp;
			pLayer->SetName(temp);
		}
		else
		{
			//Read in the fragment of this node that comes from C_Node
			status = C_NativeNode::Deserialise(
				pDiskReader, 
				pNativeFtk, 
				(C_Node*)pLayer, 
				true);
			if (!status) return status;
		}

		if (pDiskReader->GetVersion() >= 200)
		{
			//Read in layer pointer integers. These will be transformed in our pointer 
			//resolution step.
			U32 nMembers;
			*pDiskReader >> nMembers;
			pLayer->m_memberVector.Resize(nMembers);
			for (U32 iMember = 0; iMember < nMembers; iMember++)
			{
#pragma warning(push)
#pragma warning(disable: 4312)
				S32 memberPointerInteger;
				*pDiskReader >>	memberPointerInteger;
				pLayer->m_memberVector[iMember] = reinterpret_cast<C_SceneNode*>(memberPointerInteger);
#pragma warning(pop)
			}
		}
		
		//Read attribute list
		if (pDiskReader->GetVersion() < 200)
		{
			pDiskReader->EnterBlock();
			{
				//num attributes
				U32 numAttributes;
				*pDiskReader >> numAttributes;
				if (numAttributes > 0)
					return C_Status(C_Status::E_Status_Failure, "Unsupported attribute count");
			}
			pDiskReader->ExitBlock();
		}
	}
	if (useOuterBlockHeaders)
		pDiskReader->ExitBlock();

	return C_Status::E_Status_Success;
}

bool C_NativeLayerNode::ResolveUnmanagedDgNodePointers(
	U32 version,
	C_NativeFtk* pNativeFtk, 
	C_LayerNode* pLayer)
{
	if (version < 200)
		return true;

#pragma warning(push)
#pragma warning(disable: 4311)

	//Resolve member pointers
	//Copy our hijacked integer store.
	Ut::UtVector< Ut::DynamicVector<C_SceneNode*> > tempLayerVector;
	tempLayerVector = pLayer->m_memberVector;
	pLayer->m_memberVector.Clear();
	for (U32 iMember = 0; iMember < (U32)tempLayerVector.Size(); iMember++)
	{
		C_SceneNode* pSceneNode = 
			(C_SceneNode*)pNativeFtk->GetPtrFromId(reinterpret_cast<S32>(tempLayerVector[iMember]));
		pLayer->AddMember(pSceneNode);
	}
#pragma warning(pop)
	return true;
}
