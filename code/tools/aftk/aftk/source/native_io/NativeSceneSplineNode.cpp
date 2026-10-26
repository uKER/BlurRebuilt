#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeSceneNode.h"
#include "NativeSceneSplineNode.h"
#include "NativeUtility.h"

using namespace Aftk;

C_Status C_NativeSceneSplineNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_SceneSplineNode* pSceneSplineNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_SPLINE_NODE);
	}
	//Write out the fragment of this node that comes from C_SceneNode
	{
		{
			C_SceneNode* pSceneNode = (C_SceneNode*)pSceneSplineNode;
			//Setting false means that we don't use unnecessary block padding.
			//This is done simply to save disk space.
			status = C_NativeSceneNode::Serialise(
				pDiskWriter, 
				pNativeFtk, 
				pSceneNode,
				true);
			if (!status) return status;
		}
		U32 form = (U32)pSceneSplineNode->GetSplineForm();
		*pDiskWriter << form;
		U32 splineType = (U32)pSceneSplineNode->GetSplineType();
		*pDiskWriter << splineType;
		*pDiskWriter << pSceneSplineNode->GetDegree();
		//Write out spline CVs
		*pDiskWriter << (U32)pSceneSplineNode->GetNumCVs();
		for (U32 i = 0; i < pSceneSplineNode->GetNumCVs(); i++)
		{
			const Position3* pPosition3 = &pSceneSplineNode->GetCV(i);
			*pDiskWriter << *pPosition3;
		}
		*pDiskWriter << (U32)pSceneSplineNode->GetNumKnots();
		for (U32 i = 0; i < pSceneSplineNode->GetNumKnots(); i++)
		{
			*pDiskWriter << (F32)pSceneSplineNode->GetKnot(i);
		}
		*pDiskWriter << (bool)pSceneSplineNode->IsRational();
	}
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}
	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneSplineNode::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_SceneSplineNode* pSceneSplineNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders)
	{
		#ifdef BE_EXTRA_CAREFUL
		if (pDiskReader->GetNextBlockInfo() != BLOCK_ID_SPLINE_NODE)
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find spline node block.");
		}
		#endif
		pDiskReader->EnterBlock();
	}
	{
		//Read in the fragment of this node that comes from C_SceneNode
		{
			C_SceneNode* pSceneNode = (C_SceneNode*)pSceneSplineNode;
			//Setting false means that we don't use unnecessary block padding.
			//This is done simply to save disk space.
			status = C_NativeSceneNode::Deserialise(pDiskReader, pNativeFtk, pSceneNode, true);
			if (!status) return status;
		}
		U32 form;
		*pDiskReader >> form;
		pSceneSplineNode->SetSplineForm((E_SplineForm)form);

		U32 splineType;
		*pDiskReader >> splineType;
		pSceneSplineNode->SetSplineType((E_SplineType)splineType);

		if (pDiskReader->GetVersion() >= 125)
		{
			U32 degree;
			*pDiskReader >> degree;
			pSceneSplineNode->SetDegree(degree);
		}

		//Read in spline CVs
		U32 numCVs;
		*pDiskReader >> numCVs;
		pSceneSplineNode->SetCVInitialCapacity(numCVs);
		for (U32 i = 0; i < numCVs; i++)
		{
			Position3 position;
			*pDiskReader >> position;
			pSceneSplineNode->AddCV(position);
		}
		if (pDiskReader->GetVersion() > 130)
		{
			U32 numKnots;
			*pDiskReader >> numKnots;
			pSceneSplineNode->SetKnotInitialCapacity(numKnots);
			for (U32 i = 0; i < numKnots; i++)
			{
				F32 knot;
				*pDiskReader >> knot;
				pSceneSplineNode->AddKnot(knot);
			}
		}
		if (pDiskReader->GetVersion() > 134)
		{
			*pDiskReader >> pSceneSplineNode->m_rational;
		}
	}
	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}
	return C_Status::E_Status_Success;
}


