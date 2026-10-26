#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeUtility.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeIObject.h"
#include "NativeFtk.h"
#include "NativeTrack.h"
#include "NativeIPlug.h"
#include "NativeNullPlug.h"
#include "NativeStaticPlug.h"
#include "NativeDynamicPlug.h"

using namespace Aftk;

C_Status C_NativeTrack::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_Track* pTrack,
	bool useOuterBlockHeaders)
{
	if (useOuterBlockHeaders)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_TRACK);
	}
	//Write out the fragment of this node that comes from I_Object
	{
		C_Status status;
		//Setting false means that we don't use unnecessary block padding.
		//This is done simply to save disk space.
		status = C_NativeIObject::Serialise(
			pDiskWriter, 
			pNativeFtk, 
			pTrack);
		if (!status) return status;
	}

	*pDiskWriter << pTrack->m_enabled;

	*pDiskWriter << (U32)pTrack->m_clipNodes.Size();
	for (U32 i = 0; i < (U32)pTrack->m_clipNodes.Size(); i++)
	{
		C_NativeNullPlug::Serialise(
			pDiskWriter,
			pNativeFtk,
			pTrack->m_clipNodes[i]);
	}

	if (useOuterBlockHeaders)
	{
		pDiskWriter->EndBlock();
	}

	return C_Status::E_Status_Success;
}

C_Status C_NativeTrack::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_Track* pTrack,
	bool useOuterBlockHeaders)
{
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_TRACK))
			return C_Status(C_Status::E_Status_Failure, "Failed to find track block.");
		pDiskReader->EnterBlock();
	}
	if (pDiskReader->GetVersion() >= 201)
	//Read the fragment of this node that comes from I_Object
	{
		C_Status status;
		//Setting false means that we don't use unnecessary block padding.
		//This is done simply to save disk space.
		status = C_NativeIObject::Deserialise(
			pDiskReader, 
			pNativeFtk, 
			pTrack);
		if (!status) return status;
	}

	*pDiskReader >> pTrack->m_enabled;

	U32 numClips;
	*pDiskReader >> numClips;
	if (pDiskReader->GetVersion() < 201)
	{
		for (U32 i = 0; i < numClips; i++)
		{
			#pragma warning(push)
			{
				#pragma warning(disable: 4312)
				S32 clipPointerInteger;
				*pDiskReader >>	clipPointerInteger;

				//Resolve the clip source pointer
				C_ClipNode* pClip = (C_ClipNode*)pNativeFtk->GetPtrFromId(clipPointerInteger);
				AFTK_ASSERT(pClip);
				pTrack->AddClipNodePtr(pClip);
			}
			#pragma warning(pop)
		}
	}
	else
	{
		for (U32 i = 0; i < numClips; i++)
		{
			//Create null clip plug.
			pTrack->AddClipNodePtr(0);
			C_NativeNullPlug::Deserialise(
				pDiskReader,
				pNativeFtk,
				pTrack->m_clipNodes[i]);
		}
	}

	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}

	return C_Status::E_Status_Success;
}

bool C_NativeTrack::ResolveUnmanagedDgNodePointers(
	U32 version,
	C_NativeFtk* pNativeFtk, 
	C_Track* pTrack)
{
	/*
	if (version < 200)
		return true;

	//Resolve the clip source we're instancing.
#pragma warning(push)
#pragma warning(disable: 4311)

	for (U32 i = 0; i < (U32)pTrack->m_clipNodes.Size(); i++)
	{
		pTrack->m_clipNodes[i]
			= (C_ClipNode*)pNativeFtk->GetPtrFromId(reinterpret_cast<S32>(
				pTrack->m_clipNodes[i]));
		AFTK_ASSERT(pTrack->m_clipNodes[i]);
	}

#pragma warning(pop)
	*/
	return true;
}
