#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeUtility.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeAnimationCurve.h"
#include "NativeBaseKey.h"
#include "NativeHermiteKey.h"
#include "NativeIPlug.h"
#include "NativeNullPlug.h"

using namespace Aftk;

C_Status C_NativeAnimationCurveNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_AnimationCurveNode* pAnimationCurve,
	bool useOuterBlockHeaders)
{
	if (useOuterBlockHeaders)
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_ANIMATION_CURVE_NODE);

	C_Status status;
	//Write out the fragment of this node that comes from C_Node
	{
		C_Node* pNode = (C_Node*)pAnimationCurve;
		//Setting false means that we don't use unnecessary block padding.
		//This is done simply to save disk space.
		status = C_NativeNode::Serialise(
			pDiskWriter, 
			pNativeFtk, 
			pNode, 
			true);
		if (!status) return status;
	}

	status = C_NativeNullPlug::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pAnimationCurve->m_inputPlug);
	if (!status) return status;

	*pDiskWriter << (U32)pAnimationCurve->m_curveType;
	*pDiskWriter << (U32)pAnimationCurve->m_preInfinity;
	*pDiskWriter << (U32)pAnimationCurve->m_postInfinity;
	*pDiskWriter << pAnimationCurve->m_isTimeInput;

	//Write out our key data
	U32 numKeys = pAnimationCurve->GetNumKeys();
	*pDiskWriter << numKeys;

	if (numKeys > 0)
	{
		switch(pAnimationCurve->GetCurveType())
		{
		case E_AnimCurveType_Linear:
			for (U32 i = 0; i < numKeys; i++)
			{
				C_LinearKey key;
				pAnimationCurve->GetKey(key, i);
				C_NativeBaseKey::Serialise(
					pDiskWriter,
					key);
			}
			break;
		case E_AnimCurveType_Hermite:
			for (U32 i = 0; i < numKeys; i++)
			{
				C_HermiteKey key;
				pAnimationCurve->GetKey(key, i);
				C_NativeHermiteKey::Serialise(
					pDiskWriter,
					key);
			}
			break;
		default:
			//Unknown curve type.
			LOGERROR(g_aftkMainLogger, "Unknown curve type.\n",
						pAnimationCurve->GetName());
			return C_Status::E_Status_Failure;
		};
	}

	if (useOuterBlockHeaders)
		pDiskWriter->EndBlock();

	return C_Status::E_Status_Success;
}

C_Status C_NativeAnimationCurveNode::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_AnimationCurveNode* pAnimationCurve,
	bool useOuterBlockHeaders)
{
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_ANIMATION_CURVE_NODE))
			return C_Status(C_Status::E_Status_Failure, "Failed to find animation curve block.");
		pDiskReader->EnterBlock();
	}
	C_Status status;
	//Read in the fragment of this node that comes from C_Node
	{
		C_Node* pNode = (C_Node*)pAnimationCurve;
		//Setting false means that we don't use unnecessary block padding.
		//This is done simply to save disk space.
		status = C_NativeNode::Deserialise(
			pDiskReader, 
			pNativeFtk, 
			pNode, 
			true);
		if (!status) return status;
	}

	status = C_NativeNullPlug::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pAnimationCurve->m_inputPlug);
	if (!status) return status;

	E_AnimCurveType curveType;
	{
		U32 temp;
		*pDiskReader >> temp;
		curveType = (E_AnimCurveType)temp;
	}

	*pDiskReader >> (U32&)pAnimationCurve->m_preInfinity;
	*pDiskReader >> (U32&)pAnimationCurve->m_postInfinity;
	*pDiskReader >> pAnimationCurve->m_isTimeInput;

	//Note: Datatype is set internally during add key function.
	U32 numKeys;
	*pDiskReader >> numKeys;

	switch(curveType)
	{
	case E_AnimCurveType_Linear:
		for (U32 i = 0; i < numKeys; i++)
		{
			C_LinearKey key;
			C_NativeBaseKey::Deserialise(
				pDiskReader,
				key);
			pAnimationCurve->AddKey(key);
		}
		break;
	case E_AnimCurveType_Hermite:
		for (U32 i = 0; i < numKeys; i++)
		{
			C_HermiteKey key;
			C_NativeHermiteKey::Deserialise(
				pDiskReader,
				pNativeFtk,
				key);
			pAnimationCurve->AddKey(key);
		}
		break;
	default:
		break;
	}

	if (useOuterBlockHeaders)
		pDiskReader->ExitBlock();
	return C_Status::E_Status_Success;
}

C_Status C_NativeAnimationCurveNode::DeserialiseLegacy176(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_AnimationCurveNode* pAnimationCurve,
	Aftk::E_AnimAttribute& eUsage,
	bool useOuterBlockHeaders)
{
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_ANIMATION_CURVE_NODE))
			return C_Status(C_Status::E_Status_Failure, "Failed to find animation curve block.");
		pDiskReader->EnterBlock();
	}
	C_Status status;
	//Read in the fragment of this node that comes from C_Node
	{
		//Setting false means that we don't use unnecessary block padding.
		//This is done simply to save disk space.
		status = C_NativeNode::Deserialise(
			pDiskReader, 
			pNativeFtk, 
			pAnimationCurve, 
			true);
		if (!status) return status;
	}

	U32 preInfinity;
	*pDiskReader >> preInfinity;

	U32 postInfinity;
	*pDiskReader >> postInfinity;

	U32 usage;
	*pDiskReader >> usage;
	//Prior to v2.00 anim curve enums were different.
	if (usage != 0)
	{
		usage--;
		eUsage = (E_AnimAttribute)usage;
	}
	else
		eUsage = E_AnimAttribute_Unknown;

	if (pDiskReader->GetVersion() < 171)
	{
		U32 dataType;
		*pDiskReader >> dataType;
		E_GenericAttributeType dataTypeEnum = (E_GenericAttributeType)dataType;
	}
	
	//Note: Datatype is set internally during add key function.
	U32 numKeys;
	*pDiskReader >> numKeys;
	for (U32 i = 0; i < numKeys; i++)
	{
		C_LinearKey key;
		C_NativeBaseKey::Deserialise(
			pDiskReader,
			key);

		if (
			(eUsage == Aftk::E_AnimAttribute_RotateX) ||
			(eUsage == Aftk::E_AnimAttribute_RotateY) ||
			(eUsage == Aftk::E_AnimAttribute_RotateZ))
		{
			key.GetValue() = Maths::DegToRad(key.GetValue());
		}
		pAnimationCurve->AddKey(key);
	}

	if (useOuterBlockHeaders)
		pDiskReader->ExitBlock();
	return C_Status::E_Status_Success;
}
