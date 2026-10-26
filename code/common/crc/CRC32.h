#ifndef CRC_32_H
#define CRC_32_H

#include <sys/stat.h>
#include <iostream>
#ifndef YDL
#include <io.h>
#endif
#include "../tools_common/FileUtilities.h"

#define CRC32_BUFFER_SIZE 500000

class CRC32
{
public:
	bool CRCFromFile(const char* fileName, U32& crc)
	{
		crc = 0;
		if (!Tu::FileExists(fileName))
		{
			return false;
		}

		U32 lookupTable[256];

		GenerateLookupTable(lookupTable);

		U32 bytesToRead;
		if (!Tu::FileSize(fileName, bytesToRead))
		{
			return false;
		}
		if (bytesToRead == 0)
		{
			return false;
		}
		char* pBuffer = new char[CRC32_BUFFER_SIZE];
		U32 chunkSize = CRC32_BUFFER_SIZE;

		crc = 0xFFFFFFFF;

		std::ifstream file(fileName, std::ios::in | std::ios::binary);
		while (bytesToRead > 0)
		{
			if (bytesToRead < CRC32_BUFFER_SIZE)
			{
				chunkSize = bytesToRead;
			}
			file.read(pBuffer, chunkSize);
			if (!file)
			{
				delete[] pBuffer;
				return false;
			}

			for( U32 i = 0; i < chunkSize; i++ )
			{
				CRC( lookupTable, pBuffer[i], crc );
			}

			bytesToRead -= chunkSize;
		}

		file.close();
		delete[] pBuffer;
		return true;
	}

	U32 CRCFromFile(const std::string& fileName)
	{
		U32 crc;
		CRCFromFile(fileName.c_str(), crc);
		return crc;
	}

	U32 CRCFromMemory(const U8* pData, U32 size)
	{
		U32 lookupTable[256];
		GenerateLookupTable(lookupTable);
		U32 crc = 0xFFFFFFFF;
		for(U32 i = 0; i < size; i++)
			CRC(lookupTable, pData[i], crc);
		return crc;
	}

	bool CRCCheck(const std::string& fileName, U32 crc)
	{
		U32 fileCRC;
		CRCFromFile(fileName.c_str(), fileCRC);

		if (fileCRC == crc) 
			return true;
		return false;
	}

private:
	void GenerateLookupTable(U32* pLookupTable)
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

	inline void CRC(U32* pLookupTable, const char byte, U32& crc)
	{
		crc = (( crc ) >> 8 )^ pLookupTable[(( byte )^( crc )) & 0x000000FF ];
	}
};


#endif
