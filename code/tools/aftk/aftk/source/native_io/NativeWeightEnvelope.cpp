#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeUtility.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativeWeightEnvelope.h"
#include "NativeWeight.h"

using namespace Aftk;

C_Status C_NativeWeightEnvelope::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const ::C_WeightEnvelope* pWeightEnvelope,
	bool useOuterBlockHeaders)
{
	C_Status status;

	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_WEIGHT_ENVELOPE);
	}
	{
		*pDiskWriter << pNativeFtk->GetIdFromPtr(pWeightEnvelope->GetInfluenceNodePtr());

		*pDiskWriter << (U32)pWeightEnvelope->GetDataType();

		*pDiskWriter << (U32)pWeightEnvelope->GetNumWeights();
		for (U32 i = 0; i < pWeightEnvelope->GetNumWeights(); i++)
		{
			switch (pWeightEnvelope->GetDataType())
			{
			case E_GenericAttributeType_F32:
				{
					C_Weight<F32> weight;
					pWeightEnvelope->GetWeight(i, weight);

					status = C_NativeWeight<F32>::Serialise(
						pDiskWriter,
						pNativeFtk,
						&weight);
					if (!status) return status;
				}
				break;

				default:
					LOGERROR(g_aftkMainLogger, "Unknown weight type.\n");
					AFTK_ASSERT(false);
					return C_Status::E_Status_Failure;
			}
		}
	}
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}

	return C_Status::E_Status_Success;
}

C_Status C_NativeWeightEnvelope::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_WeightEnvelope* pWeightEnvelope,
	bool useOuterBlockHeaders)
{
	C_Status status;

	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_WEIGHT_ENVELOPE))
		{
			return C_Status(C_Status::E_Status_Failure, "Could not find weight envelope block.");
		}
		pDiskReader->EnterBlock();
	}
	{
		#pragma warning(push)
		{
			#pragma warning(disable: 4312)
			S32 influencePointerInteger;
			*pDiskReader >>	influencePointerInteger;
			pWeightEnvelope->m_pInfluenceNode = reinterpret_cast<Aftk::C_SceneNode*>(influencePointerInteger);
		}
		#pragma warning(pop)

		E_GenericAttributeType weightTypeEnum;
		if (pDiskReader->GetVersion() >= 139)
		{
			U32 temp;
			*pDiskReader >> temp;
			weightTypeEnum = (E_GenericAttributeType)temp;
		}
		else
		{
			//Legacy vertex weights are all of type F32
			weightTypeEnum = E_GenericAttributeType_F32;
		}

		U32 numWeights;
		*pDiskReader >> numWeights;
		pWeightEnvelope->SetWeightInitialCapacity(numWeights);
		for (U32 i = 0; i < numWeights; i++)
		{
			switch (weightTypeEnum)
			{
				case E_GenericAttributeType_F32:
				{
					C_Weight<F32> weight;
					
					status = C_NativeWeight<F32>::Deserialise(
						pDiskReader,
						pNativeFtk,
						&weight);
					if (!status) return status;

					pWeightEnvelope->AddWeight(weight);
				}
				break;

				default:
					LOGERROR(g_aftkMainLogger, "Unknown weight type.\n");
					AFTK_ASSERT(false);
					return C_Status::E_Status_Failure;
			}
		}
	}
	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}
	
	return C_Status::E_Status_Success;
}

bool C_NativeWeightEnvelope::ResolveUnmanagedDgNodePointers(
	U32 version,
	C_NativeFtk* pNativeFtk,
	C_WeightEnvelope* pWeightEnvelope)
{
	#pragma warning(push)
	#pragma warning(disable: 4311)
	pWeightEnvelope->m_pInfluenceNode = 
		(C_SceneNode*)pNativeFtk->GetPtrFromId(reinterpret_cast<S32>(pWeightEnvelope->m_pInfluenceNode));
	#pragma warning(pop)

	return true;
}

