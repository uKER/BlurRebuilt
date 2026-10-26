#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeSceneNode.h"
#include "NativeUtility.h"
#include "NativeIPlug.h"
#include "NativeNullPlug.h"
#include "NativeStaticPlug.h"
#include "NativeDynamicPlug.h"

using namespace Aftk;

C_Status C_NativeSceneNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_SceneNode* pSceneNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_SCENE_NODE);
	}
	{
		//Write out the fragment of this node that comes from C_Node
		{
			C_Node* pNode = (C_Node*)pSceneNode;
			//Setting false means that we don't use unnecessary block padding.
			//This is done simply to save disk space.
			status = C_NativeNode::Serialise(
				pDiskWriter, 
				pNativeFtk, 
				pNode,
				true);
			if (!status) return status;
		}

		//Serialise relationship pointers
		*pDiskWriter <<			pNativeFtk->GetIdFromPtr(pSceneNode->m_pParent);
		*pDiskWriter <<			pNativeFtk->GetIdFromPtr(pSceneNode->m_pChild);
		*pDiskWriter <<			pNativeFtk->GetIdFromPtr(pSceneNode->m_pNext);
		*pDiskWriter <<			pNativeFtk->GetIdFromPtr(pSceneNode->m_pPrevious);

		//Write out our transform proporties.
		{
			status = C_NativeNullPlug::Serialise(
				pDiskWriter,
				pNativeFtk,
				&pSceneNode->m_translatePlug);
			if (!status) return status;
			{
				status = C_NativeStaticPlug<F32>::Serialise(
					pDiskWriter,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_translatePlug.GetChildPtr(0));
				if (!status) return status;

				status = C_NativeStaticPlug<F32>::Serialise(
					pDiskWriter,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_translatePlug.GetChildPtr(1));
				if (!status) return status;

				status = C_NativeStaticPlug<F32>::Serialise(
					pDiskWriter,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_translatePlug.GetChildPtr(2));
				if (!status) return status;
			}

			status = C_NativeNullPlug::Serialise(
				pDiskWriter,
				pNativeFtk,
				&pSceneNode->m_rotatePlug);
			if (!status) return status;
			{
				status = C_NativeStaticPlug<F32>::Serialise(
					pDiskWriter,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_rotatePlug.GetChildPtr(0));
				if (!status) return status;

				status = C_NativeStaticPlug<F32>::Serialise(
					pDiskWriter,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_rotatePlug.GetChildPtr(1));
				if (!status) return status;

				status = C_NativeStaticPlug<F32>::Serialise(
					pDiskWriter,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_rotatePlug.GetChildPtr(2));
				if (!status) return status;
			}

			status = C_NativeNullPlug::Serialise(
				pDiskWriter,
				pNativeFtk,
				&pSceneNode->m_scalePlug);
			if (!status) return status;
			{
				status = C_NativeStaticPlug<F32>::Serialise(
					pDiskWriter,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_scalePlug.GetChildPtr(0));
				if (!status) return status;

				status = C_NativeStaticPlug<F32>::Serialise(
					pDiskWriter,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_scalePlug.GetChildPtr(1));
				if (!status) return status;

				status = C_NativeStaticPlug<F32>::Serialise(
					pDiskWriter,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_scalePlug.GetChildPtr(2));
				if (!status) return status;
			}

			status = C_NativeNullPlug::Serialise(
				pDiskWriter,
				pNativeFtk,
				&pSceneNode->m_shearPlug);
			if (!status) return status;
			{
				status = C_NativeStaticPlug<F32>::Serialise(
					pDiskWriter,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_shearPlug.GetChildPtr(0));
				if (!status) return status;

				status = C_NativeStaticPlug<F32>::Serialise(
					pDiskWriter,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_shearPlug.GetChildPtr(1));
				if (!status) return status;

				status = C_NativeStaticPlug<F32>::Serialise(
					pDiskWriter,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_shearPlug.GetChildPtr(2));
				if (!status) return status;
			}

			status = C_NativeNullPlug::Serialise(
				pDiskWriter,
				pNativeFtk,
				&pSceneNode->m_worldMatrixProxyPlug);
			if (!status) return status;

			status = C_NativeStaticPlug<bool>::Serialise(
				pDiskWriter,
				pNativeFtk,
				&pSceneNode->m_visibilityPlug);
			if (!status) return status;

			*pDiskWriter << (U32)pSceneNode->m_rotationInterpolation;

			//num plugs
			U32 numPlugs = (U32)pSceneNode->m_dynamicPlugs.Size();
			//Store the number of plugs
			*pDiskWriter << numPlugs;
			//material attributes
			for (U32 i = 0; i < numPlugs; i++) 
			{
				const C_DynamicPlug* pPlug = (C_DynamicPlug*)pSceneNode->m_dynamicPlugs[i];

				status = C_NativeDynamicPlug::Serialise(
					pDiskWriter, 
					pNativeFtk,
					pPlug);
				if (!status) return status;
			}

			*pDiskWriter << (U32)pSceneNode->m_rotationOrder;
			*pDiskWriter << pSceneNode->m_scalePivot;
			*pDiskWriter << pSceneNode->m_scalePivotTranslation;
			*pDiskWriter << pSceneNode->m_rotatePivot;
			*pDiskWriter << pSceneNode->m_rotatePivotTranslation;

			*pDiskWriter <<	pSceneNode->m_basePose;
		}

		*pDiskWriter <<			pSceneNode->m_uniqueId;

		//Write out blind data.
		{
			if (pSceneNode->m_pBlindDataResource == 0)
			{
				*pDiskWriter << S32(-1);
			}
			else
			{
				//A container for a blind data resource
				const C_SharedResource<C_BlindDataNode>* pBlindDataResource 
					= pSceneNode->m_pBlindDataResource;
				//store the instance of the blind data resource, not its shared resource pointer

				Ut::UtMap<const C_BlindDataResource*, U32>::Iterator bdIt;
				bdIt = pNativeFtk->m_pBlindDataResToIthIndexMapping->Find(pBlindDataResource);
				if (bdIt == pNativeFtk->m_pBlindDataResToIthIndexMapping->End())
				{
					LOGERROR(g_aftkMainLogger, "Could not find blind data resource during serialise for node [%s] uid [%i] type [%i].\n",
						pSceneNode->GetName(), pSceneNode->GetUniqueId(), pSceneNode->GetType());

					for (U32 i = 0; i < pBlindDataResource->GetContent().GetNumAttributes(); i++)
					{
						LOGERROR(g_aftkMainLogger, "Attr: %i [%s]\n",
							i,
							pBlindDataResource->GetContent().GetAttribute(i).GetName().c_str());
					}

					const C_BlindDataResource* pExistingRes = 
						(const_cast<Aftk::C_BlindDataResourceManager&>(pSceneNode->GetScenePtr()->m_blindDataNodeManager)).Exists(pBlindDataResource->GetContent());
					if (pExistingRes)
					{
						LOGERROR(g_aftkMainLogger, "Existing resource found for %i as %i.", pBlindDataResource, pExistingRes);
					}
					else
					{
						LOGERROR(g_aftkMainLogger, "No existing resource found.");
					}
					const Ut::UtVector<Ut::DynamicVector<C_BlindDataResource*> >& arr = 
						pSceneNode->GetScenePtr()->m_blindDataNodeManager.m_array;
					bool found = false;
					for (U32 i = 0; i < (U32)arr.Size(); i++)
					{
						if (arr[i] == pBlindDataResource)
						{
							LOGERROR(g_aftkMainLogger, "Found res at index %i as compared to %i", i, pBlindDataResource->GetIndex());
							found = true;
						}
					}
					if (!found)
					{
						LOGERROR(g_aftkMainLogger, "Not founc");
					}
					if (arr.Size() != pSceneNode->GetScenePtr()->m_blindDataNodeManager.m_hashTable.Size())
					{
						LOGERROR(g_aftkMainLogger, "Hash-array mismatch");
					}

					AFTK_ASSERT(bdIt != pNativeFtk->m_pBlindDataResToIthIndexMapping->End());
					return C_Status::E_Status_Failure;
				}
				const Aftk::C_Scene* pScene = pSceneNode->GetScenePtr();

				S32 ithIndex = bdIt->GetValue();
				*pDiskWriter << ithIndex;
			}
		}
	}

	if (useOuterBlockHeaders == true)
		pDiskWriter->EndBlock();
	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneNode::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_SceneNode* pSceneNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders == true)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_SCENE_NODE))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find scene node block.");
		}
		pDiskReader->EnterBlock();
	}
	{
		//Read in the fragment of this node that comes from C_Node
		{
			//Setting false means that we don't use unnecessary block padding.
			//This is done simply to save disk space.
			status = C_NativeNode::Deserialise(
				pDiskReader, 
				pNativeFtk, 
				(C_Node*)pSceneNode, 
				true);
			if (!status) return status;
		}

		pSceneNode->m_hierarchyValid = false;

		if (pDiskReader->GetVersion() < 200)
		{
			//Deserialise this node's id, in other words its integer mapped 'this' pointer.
			S32 id;
			*pDiskReader >> id;
			//We know this node's true this pointer, and so we can add this to our mapping table.
			//This allows us to rebuild the relationship pointers after loading the scene.
			pNativeFtk->AddToIdPtrMapping(id, pSceneNode);
		}

		//Deserialise relationship pointers

		//Disable warnings about casting from an integer to a pointer. This is fine for
		//our purposes
		#pragma warning(push)
		{
			#pragma warning(disable: 4312)
			S32 parentPointerInteger;
			*pDiskReader >>			parentPointerInteger;
			pSceneNode->m_pParent = reinterpret_cast<C_SceneNode*>(parentPointerInteger);
			
			S32 childPointerInteger;
			*pDiskReader >>			childPointerInteger;
			pSceneNode->m_pChild = reinterpret_cast<C_SceneNode*>(childPointerInteger);
			
			S32 nextPointerInteger;
			*pDiskReader >>			nextPointerInteger;
			pSceneNode->m_pNext = reinterpret_cast<C_SceneNode*>(nextPointerInteger);
			
			S32 previousPointerInteger;
			*pDiskReader >>			previousPointerInteger;
			pSceneNode->m_pPrevious = reinterpret_cast<C_SceneNode*>(previousPointerInteger);
		}
		#pragma warning(pop)

		if (pDiskReader->GetVersion() < 200)
		{
			Matrix44 matrixIn;
			*pDiskReader >> matrixIn;
			pSceneNode->SetLocalMatrix(matrixIn);

			//In debug mode perform additional validation that our matrix decomposition has worked
			//as expected.
			#ifdef BUILD_CONFIG_DEBUG
			Vector3F32 translate;
			Vector3F32 scale;
			Vector3F32 shear;
			Vector3F32 rotate;

			pSceneNode->GetTranslation(translate, false);
			pSceneNode->GetScale(scale, false);
			pSceneNode->GetShear(shear, false);
			pSceneNode->GetRotation(rotate, false);

			Matrix44 test;
			BuildMatrix(
				test,
				pSceneNode->m_scalePivot,
				scale,
				shear,
				pSceneNode->m_scalePivotTranslation,
				pSceneNode->m_rotatePivot,
				pSceneNode->m_rotateOrientation,
				rotate, 
				pSceneNode->m_rotatePivotTranslation,
				translate,
				pSceneNode->m_rotationOrder);

			AFTK_ASSERT(Aftk::EqualityTestFMatrix(test, matrixIn));
			#endif
		}
		else
		{
			//Read transform proporties
			status = C_NativeNullPlug::Deserialise(
				pDiskReader,
				pNativeFtk,
				&pSceneNode->m_translatePlug);
			if (!status) return status;
			{
				status = C_NativeStaticPlug<F32>::Deserialise(
					pDiskReader,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_translatePlug.GetChildPtr(0));
				if (!status) return status;

				status = C_NativeStaticPlug<F32>::Deserialise(
					pDiskReader,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_translatePlug.GetChildPtr(1));
				if (!status) return status;

				status = C_NativeStaticPlug<F32>::Deserialise(
					pDiskReader,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_translatePlug.GetChildPtr(2));
				if (!status) return status;
			}

			status = C_NativeNullPlug::Deserialise(
				pDiskReader,
				pNativeFtk,
				&pSceneNode->m_rotatePlug);
			if (!status) return status;
			{
				status = C_NativeStaticPlug<F32>::Deserialise(
					pDiskReader,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_rotatePlug.GetChildPtr(0));
				if (!status) return status;

				status = C_NativeStaticPlug<F32>::Deserialise(
					pDiskReader,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_rotatePlug.GetChildPtr(1));
				if (!status) return status;

				status = C_NativeStaticPlug<F32>::Deserialise(
					pDiskReader,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_rotatePlug.GetChildPtr(2));
				if (!status) return status;
			}

			status = C_NativeNullPlug::Deserialise(
				pDiskReader,
				pNativeFtk,
				&pSceneNode->m_scalePlug);
			if (!status) return status;
			{
				status = C_NativeStaticPlug<F32>::Deserialise(
					pDiskReader,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_scalePlug.GetChildPtr(0));
				if (!status) return status;

				status = C_NativeStaticPlug<F32>::Deserialise(
					pDiskReader,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_scalePlug.GetChildPtr(1));
				if (!status) return status;

				status = C_NativeStaticPlug<F32>::Deserialise(
					pDiskReader,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_scalePlug.GetChildPtr(2));
				if (!status) return status;
			}

			status = C_NativeNullPlug::Deserialise(
				pDiskReader,
				pNativeFtk,
				&pSceneNode->m_shearPlug);
			if (!status) return status;
			{
				status = C_NativeStaticPlug<F32>::Deserialise(
					pDiskReader,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_shearPlug.GetChildPtr(0));
				if (!status) return status;

				status = C_NativeStaticPlug<F32>::Deserialise(
					pDiskReader,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_shearPlug.GetChildPtr(1));
				if (!status) return status;

				status = C_NativeStaticPlug<F32>::Deserialise(
					pDiskReader,
					pNativeFtk,
					(C_StaticPlug<F32>*)pSceneNode->m_shearPlug.GetChildPtr(2));
				if (!status) return status;
			}

			if (pDiskReader->GetVersion() >= 204)
			{
				status = C_NativeNullPlug::Deserialise(
					pDiskReader,
					pNativeFtk,
					&pSceneNode->m_worldMatrixProxyPlug);
			}

			if (pDiskReader->GetVersion() >= 210)
			{
				status = C_NativeStaticPlug<bool>::Deserialise(
					pDiskReader,
					pNativeFtk,
					&pSceneNode->m_visibilityPlug);

				U32 tmp;
				*pDiskReader >> tmp;
				pSceneNode->m_rotationInterpolation = (E_RotationInterpolation)tmp;
			}

			U32 numPlugs;
			*pDiskReader >> numPlugs;
			pSceneNode->m_dynamicPlugs.Reserve(numPlugs);
			for (U32 i = 0; i < numPlugs; i++)
			{
				C_DynamicPlug* pPlug = new C_DynamicPlug();
				if (!pPlug) return C_Status::E_Status_OutOfMemory;
				pSceneNode->AddPlugPtr(pPlug);

				status = C_NativeDynamicPlug::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pPlug);
				if (!status) return status;
			}

			U32 rotationOrder;
			*pDiskReader >> rotationOrder;
			pSceneNode->m_rotationOrder = ((E_RotationOrder)rotationOrder);

			*pDiskReader >> pSceneNode->m_scalePivot;
			*pDiskReader >> pSceneNode->m_scalePivotTranslation;
			*pDiskReader >> pSceneNode->m_rotatePivot;
			*pDiskReader >> pSceneNode->m_rotatePivotTranslation;
		}

		//Read in base pose if needed
		if (pDiskReader->GetVersion() > 140)
		{
			Matrix44 temp;
			*pDiskReader >> temp;
			pSceneNode->SetBasePose(temp);
		}

		*pDiskReader >> pSceneNode->m_uniqueId;

		if (pDiskReader->GetVersion() < 147)
		{	
			S32 temp;
			*pDiskReader >> temp;
		}

		// Read in this node's layer index. Prior to 171 each node could only be a member of a single layer.
		if (pDiskReader->GetVersion() < 171)
		{
			S32 layerIndex;
			*pDiskReader >> layerIndex;

			//If valid, find the corresponding layer.
			if (layerIndex != -1)
			{
				C_Scene* pScene = pSceneNode->GetScenePtr();
				C_LayerNode* pLayer = pScene->GetLayerPtr(layerIndex);
				pLayer->AddMember(pSceneNode);
			}
		}
		else if (pDiskReader->GetVersion() < 200)
		{
			U32 nLayers;
			*pDiskReader >> nLayers;
			for (U32 iLayer = 0; iLayer < nLayers; iLayer++)
			{
				S32 layerIndex;
				*pDiskReader >> layerIndex;

				//If valid, find the corresponding layer.
				if (layerIndex != -1)
				{
					C_Scene* pScene = pSceneNode->GetScenePtr();
					C_LayerNode* pLayer = pScene->GetLayerPtr(layerIndex);
					pLayer->AddMember(pSceneNode);
				}
			}
		}

		{
			//Fetch the instance of the blind data resource, not its shared resource pointer
			S32 blindDataIndex;
			*pDiskReader >> blindDataIndex;
			
			if (blindDataIndex == -1)
			{
				pSceneNode->SetBlindDataResourcePtr(0);
			}
			else
			{
				//Fetch the blind data corresponding to this instance
				const C_SharedResource<C_BlindDataNode>* pBlindDataResource = 
					pNativeFtk->m_blindDataMappingArray[blindDataIndex];
				if (pBlindDataResource == 0)
				{
					//We couldn't fetch the blind data instance. Perhaps the file is corrupt. Abort.
					return C_Status(C_Status::E_Status_Failure, "Failed to fetch blind data instance.");
				}

				// Store our newly acquired blind data resource pointer.
				pSceneNode->SetBlindDataResourcePtr(pBlindDataResource);
			}
		}

	}
	if (useOuterBlockHeaders == true)
		pDiskReader->ExitBlock();

	return C_Status::E_Status_Success;
}

bool C_NativeSceneNode::ResolveUnmanagedDgNodePointers(
	U32 version,
	C_NativeFtk* pNativeFtk,
	C_SceneNode* pSceneNode)
{
#pragma warning(push)
#pragma warning(disable: 4311)
	//Converts integer based pointers into correct scene node pointers.

	// This is one of the few places I'm going to willingly bypass the AFTK's interface as
	// The get/set pointer options of the AFTK do actually perform background work.
	pSceneNode->m_pNext 
		= (C_SceneNode*)pNativeFtk->GetPtrFromId(reinterpret_cast<S32>(pSceneNode->m_pNext));
	pSceneNode->m_pPrevious 
		= (C_SceneNode*)pNativeFtk->GetPtrFromId(reinterpret_cast<S32>(pSceneNode->m_pPrevious));
	pSceneNode->m_pParent 
		= (C_SceneNode*)pNativeFtk->GetPtrFromId(reinterpret_cast<S32>(pSceneNode->m_pParent));
	pSceneNode->m_pChild 
		= (C_SceneNode*)pNativeFtk->GetPtrFromId(reinterpret_cast<S32>(pSceneNode->m_pChild));
#pragma warning(pop)
	pSceneNode->m_hierarchyValid = true;
	return true;
}
