#include "LogicalFsPch.h"

#include <System/Serialise/BinarySerialiseAutoEndian.h>
#include <System/Serialise/C_MemoryStream.h>

#include <Misc_Shared/Uid/FourCc.h>
#include <Misc_Shared/DataStructure/StringTable.h>

#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/PakFile.h>
#include <PakFs/Impl/PakFatEntry.h>
#include <PakFs/Impl/PakIoMediator.h>
#include <PakFs/Impl/PakSha1.h>
#include <PakFs/Impl/PakUtil.h>

#ifdef CFG_CONVERTER

namespace Pak
{

bool WritePreHeader(C_PakFile& pak, Sys::C_StreamPos& versionPos)
{
	//Write 4cc and version
	{
		U32 fourCc = PAK_FOURCC_U32;
		if (!pak.pMediator->Write(fourCc)) return false;
		//Mark as corrupt, we'll overwrite this later.
		U32 version = PAK_VERSION_CORRUPT;
		versionPos = pak.pMediator->GetWritePosition();
		if (!pak.pMediator->Write(version)) return false;
		if (!pak.pMediator->Write(pak.pMediator->GetSectorSize())) return false;
	}

	return true;
}

bool WriteHeader(C_PakFile& pak, Sys::C_StreamPos headerStartPos)
{
	U32 headerSize = sizeof(C_PakHeader);
	Sys::C_MemoryStream memStream(sizeof(C_PakHeader));
	memStream.SetEndianSwapFlag(pak.pMediator->GetEndianSwapFlag());
	Sys::BinarySerialiserAutoEndian ar(memStream);
	ar << pak.header;

	pak.pMediator->SetWritePosition(headerStartPos);
	if (!pak.pMediator->Write(memStream.GetBufferPtr(), headerSize)) return false;

	return true;
}

bool WriteStringTable(C_PakFile& pak)
{
	if (!pak.pMediator->Write(pak.stringTable.dataSize)) return false;
	PakCryptDecrypt(pak.stringTable.pData, pak.stringTable.dataSize);
	if (!pak.pMediator->Write(pak.stringTable.pData, pak.stringTable.dataSize)) return false;
	PakCryptDecrypt(pak.stringTable.pData, pak.stringTable.dataSize);
	Sys::C_StreamPos pos = pak.pMediator->GetWritePosition();
	U32 padding = pak.pMediator->GetSectorSize() - (pos % pak.pMediator->GetSectorSize());
	if (!WritePadding(*pak.pMediator, padding)) return false;

	return true;
}

bool WriteFat(C_PakFile& pak, Sys::C_StreamPos fatStartPos, U8* pSecurityHash)
{
	pak.pMediator->SetWritePosition(fatStartPos);

	U32 fatEntrySize = GetFatEntrySize((E_FatEntryType)pak.header.fatEntryType);
	if (fatEntrySize == 0)
		return false;
	U32 fatSize = fatEntrySize * pak.nFatEntries;

	//Write FAT
	if (pak.pMediator->GetEndianSwapFlag())
	{
		Sys::C_MemoryStream memStream(fatSize);
		memStream.SetEndianSwapFlag(true);
		Sys::BinarySerialiserAutoEndian ar(memStream);
		//Perform fixup for appropriate derived structure type.
		if (pak.header.fatEntryType == E_FatEntryType_Normal)
		{
			for (U32 i = 0; i < pak.nFatEntries; ++i)
			{
				C_PakFatEntry& fatEntry = ((C_PakFatEntry*)pak.pFatEntries)[i];
				ar << fatEntry;
			}
		}
		else if (pak.header.fatEntryType == E_FatEntryType_Secure)
		{
			for (U32 i = 0; i < pak.nFatEntries; ++i)
			{
				C_PakFatEntrySecure& fatEntry = ((C_PakFatEntrySecure*)pak.pFatEntries)[i];
				ar << fatEntry;
			}
		}
		else
			return false;

		//Calculate FAT hash.
		if (pSecurityHash)
		{
			Sha1::C_HashContext hashContext;
			if (!hashContext.Valid())	return false;
			if (!hashContext.UpdateHash(memStream.GetBufferPtr(), fatSize)) return false;
			if (!hashContext.GetHash(pSecurityHash)) return false;
		}
		if (!pak.pMediator->Write(memStream.GetBufferPtr(), fatSize)) return false;
	}
	else
	{
		//Calculate FAT hash.
		if (pSecurityHash)
		{
			Sha1::C_HashContext hashContext;
			if (!hashContext.Valid()) return false;
			if (!hashContext.UpdateHash(pak.pFatEntries, fatSize)) return false;
			if (!hashContext.GetHash(pSecurityHash)) return false;
		}
		if (!pak.pMediator->Write(pak.pFatEntries, fatSize)) return false;
	}

	return true;
}

}

#endif
