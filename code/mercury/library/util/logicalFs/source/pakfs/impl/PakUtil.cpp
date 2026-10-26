#include "LogicalFsPch.h"

#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/PakUtil.h>
#include <PakFs/Impl/PakFatEntry.h>
#include <PakFs/Impl/PakIoMediator.h>
#include <PakFs/Impl/PakSha1.h>

#define PAK_CIPHER "xVXo40j3@$%\\%`"
#define PAK_CIPHER_LENGTH 14

namespace Pak
{

bool C_PakCrc32::createdTable = false;
U32 C_PakCrc32::lookupTable[256];

void PakCryptDecrypt(U8* pData, U32 sizeBytes)
{
	const char cipher[] = PAK_CIPHER;
	U32 cipherIdx = 0;
	for (U32 i = 0; i < sizeBytes; ++i)
	{
		++cipherIdx;
		cipherIdx = cipherIdx % PAK_CIPHER_LENGTH;
		pData[i] = cipher[cipherIdx] ^ pData[i];
	}
}

#ifdef CFG_CONVERTER
bool WritePadding(C_PakIoMediator& mediator, U32 bytes)
{
	const U32 kWriteBufferSize = 2048;
	U8 buf[kWriteBufferSize];
	memset(buf, 0, sizeof(buf));
	U32 remainder = bytes % kWriteBufferSize;
	if (!mediator.Write(buf, remainder))
		return false;
	U32 n = bytes / kWriteBufferSize;
	while(n)
	{
		if (!mediator.Write(buf, kWriteBufferSize))
			return false;
		--n;
	}

	return true;
}
#endif

U32	GetFatEntrySize(E_FatEntryType fatEntryType)
{
	switch(fatEntryType)
	{
		case E_FatEntryType_Normal:
			return sizeof(C_PakFatEntry);
		case E_FatEntryType_Secure:
			return sizeof(C_PakFatEntrySecure);
		default:
			break;
	};
	BIZ_ASSERT_FAIL("Unknown FAT entry size");
	return 0;
}

int FatEntryCompare(const void *lhs, const void *rhs)
{
	const C_PakFatEntry& a(*(const C_PakFatEntry*)lhs);
	const C_PakFatEntry& b(*(const C_PakFatEntry*)rhs);

	if (a < b)
		return -1;
	if (a > b)
		return 1;
	return 0;
}

bool IsFilePosSectorAligned(Sys::I_File& file, U32 sectorSize)
{
	//Check file alignment.
	U64 pos = file.GetPosition();
	if (pos % sectorSize != 0)
	{
		//Misaligned data, writer must ensure we end on a sector boundary.
		return false;
	}
	return true;
}

void ConvertToPakDirToken(
	Btl::String260& localised, 
	char dirToken, 
	const Sys::C_FilePath& filePath)
{
	BIZ_ASSERT(dirToken == '/' || dirToken == '\\');
	if (dirToken == Sys::GetLocalDirToken())
	{
		localised = filePath.String();
		return;
	}

	localised = filePath.String();
	for (S32 i = 0; i < localised.Length(); ++i)
	{
		if (localised[i] == Sys::GetLocalDirToken())
			localised[i] = dirToken;
	}
}

bool GenerateSha1HashForFile(
	U8* pSha1Result, 
	Sys::I_File& file)
{
	Sha1::C_HashContext hashContext;
	if (!hashContext.Valid())
		return false;

	U32 bytesRemaining = (U32)file.Length();
	U8 buffer[32*1024];
	U32 bytesTotal = 0;
	while(bytesRemaining)
	{
		U32 bytesToRead = bytesRemaining > sizeof(buffer) ? sizeof(buffer) : (U32)bytesRemaining;
		if (!file.Read(buffer, bytesToRead)) return false;

		if (!hashContext.UpdateHash(buffer, bytesToRead))
			return false;

		bytesRemaining -= bytesToRead;
		bytesTotal += bytesToRead;
	}

	//Store hash result.
	if (!hashContext.GetHash(pSha1Result))
		return false;

	return true;
}

}
