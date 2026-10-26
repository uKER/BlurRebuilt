#ifndef LOGICALFS_PAKFS_PAK_UTIL_H
#define LOGICALFS_PAKFS_PAK_UTIL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "PakDefinitions.h"

namespace Pak
{
	class C_PakIoMediator;

	void PakCryptDecrypt(U8* pData, U32 sizeBytes);

	bool WritePadding(C_PakIoMediator& mediator, U32 bytes);

	U32	GetFatEntrySize(E_FatEntryType fatEntryType);

	int FatEntryCompare(const void *lhs, const void *rhs);

	bool IsFilePosSectorAligned(Sys::I_File& file, U32 sectorSize);

	void ConvertToPakDirToken(
		Btl::String260& localised, 
		char dirToken, 
		const Sys::C_FilePath& filePath);

	bool GenerateSha1HashForFile(
		U8* pSha1Result, 
		Sys::I_File& file);

	class C_PakCrc32
	{
	private:
		static bool createdTable;
		static U32 lookupTable[256];
	public:
		C_PakCrc32()
		{
			if (!createdTable)
			{
				GenerateLookupTable(lookupTable);
				createdTable = true;
			}
		}

		U32 CRCFromMemory(const U8* pData, U32 size)
		{
			U32 crc = 0xFFFFFFFF;
			for(U32 i = 0; i < size; i++)
				CRC32(lookupTable, pData[i], crc);
			return crc;
		}

	private:
		INLINE static void CRC32(U32* pLookupTable, const char byte, U32& crc)
		{
			crc = (( crc ) >> 8 )^ pLookupTable[(( byte )^( crc )) & 0x000000FF ];
		}

		static void GenerateLookupTable(U32* pLookupTable)
		{
			//Binary reflection of the value 0x04C11DB7 used by WinZIP.
			U32 polynomial = 0xEDB88320;
			 
			U32 tableValue;
			for(S32 i = 0; i < 256; i++)
			{
				tableValue = i;
			      
				for(S32 j = 8; j > 0; j--)
				{
					if(tableValue & 1)
						tableValue = (tableValue >> 1) ^ polynomial;
					else
						tableValue >>= 1;
				}
				
				pLookupTable[i] = tableValue;
			}
		}
	};
}

#endif
