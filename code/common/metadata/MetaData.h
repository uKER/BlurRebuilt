#ifndef META_DATA_H
#define META_DATA_H

#define METADATA_BUFFER_SIZE 500000

#include "../tools_common/fileutilities.h"
#ifdef WIN32
#include <windows.h>
#endif

class MetaData
{
public:
	static bool ReadMetaData(const std::string& fileName, std::string& metaData);
	static bool WriteMetaData(const std::string& fileName, const std::string& metaData);

private:
	static bool CopyFileFragment(
		std::ifstream& fileIn,
		std::ofstream& fileOut,
		U32 startPos,
		U32 bytesToCopy);
};

inline bool MetaData::ReadMetaData(const std::string& fileName, std::string& metaData)
{
	const char* pFileName = fileName.c_str();
	if (!Tu::FileExists(pFileName))
	{
//		"File does not exist.";
		return false;
	}

	U32 bytesToRead;
	if (!Tu::FileSize(pFileName, bytesToRead))
	{
//		"Could not query file size."
		return false;
	}
	if (bytesToRead == 0)
	{
//		"File is of zero bytes.";
		return false;
	}

	S32 filePos = Tu::FindInFile(pFileName, "META", 4);
	if (filePos == -1)
	{
//		"Could fine meta data.";
		return false;
	}

	//Read meta-data
	std::ifstream file(pFileName, std::ios::in | std::ios::binary);
	U32	size;
	file.seekg(filePos+4);
	file.read((char*)&size, 4);
	metaData.reserve(size);

	char* pStringBuffer = new char[size];
	file.read(pStringBuffer, size);

	//Copy buffer in to result string.
	metaData = pStringBuffer;
	delete[] pStringBuffer;

	return true;
}

inline bool MetaData::CopyFileFragment(
	std::ifstream& fileIn,
	std::ofstream& fileOut,
	U32 startPos,
	U32 bytesToCopy)
{
	fileIn.seekg(startPos);

	char* pBuffer = new char[METADATA_BUFFER_SIZE];
	U32 chunkSize = METADATA_BUFFER_SIZE;

	while (bytesToCopy > 0)
	{
		if (bytesToCopy < METADATA_BUFFER_SIZE)
		{
			chunkSize = bytesToCopy;
		}
		fileIn.read(pBuffer, chunkSize);
		if (!fileIn)
		{
			delete[] pBuffer;
			return false;
		}

		if (!fileOut)
		{
			delete[] pBuffer;
			return false;
		}
		fileOut.write(pBuffer, chunkSize);
		bytesToCopy -= chunkSize;
	}

	delete[] pBuffer;
	return true;
}

inline bool MetaData::WriteMetaData(const std::string& fileName, const std::string& metaData)
{
	const char* pFileName = fileName.c_str();
	if (!Tu::FileExists(pFileName))
	{
//		"File does not exist.";
		return false;
	}

	U32 bytesToRead;
	if (!Tu::FileSize(pFileName, bytesToRead))
	{
//		"Could not query file size.";
		return false;
	}
	if (bytesToRead == 0)
	{
//		"File is of zero bytes.";
		return false;
	}

	S32 metaDataPos = Tu::FindInFile(pFileName, "META", 4);
	if (metaDataPos == -1)
	{
//		"Could fine meta data.";
		return false;
	}

	std::string fileNameOut = fileName;
	fileNameOut += ".temp";

	std::ifstream fileIn(fileName.c_str(), std::ios::in | std::ios::binary);
	std::ofstream fileOut(fileNameOut.c_str(), std::ios::out | std::ios::binary);
	//Skip 'META'
	fileIn.seekg(metaDataPos+4);
	//Find the size of the old meta data string.
	U32	oldMetaDataSize;
	fileIn.read((char*)&oldMetaDataSize, 4);

	//Copy fragment of file prior to meta data
	if (!CopyFileFragment(fileIn, fileOut, 0, metaDataPos+4))
	{
		fileIn.close();
		fileOut.close();
//		"Could write file during meta data update.";
		return false;
	}
	
	//Copy meta data
	U32 metaDataLengthInBytes;
	{
		U32 stringLength = (U32)metaData.length() + 1;
		metaDataLengthInBytes = 4 + stringLength;
		//Store string length.
		fileOut.write((char*)&stringLength, 4);
		char* pStringBuffer = new char[stringLength];
		//Copy into output buffer.
		for (U32 i = 0; i < (U32)metaData.length(); i++)
		{
			pStringBuffer[i] = metaData[i];
		}
		//Null terminate string.
		pStringBuffer[metaData.length()] = '\0';
		fileOut.write(pStringBuffer, stringLength);
		delete[] pStringBuffer;
	}

	//Start of 'META' + 4 (length of 'META') + 4 (U32 size)
	U32 secondFragmentStart = metaDataPos+8+oldMetaDataSize;
	//Copy fragment of file after meta data
	if (!CopyFileFragment(fileIn, fileOut, secondFragmentStart, bytesToRead-secondFragmentStart))
	{
		fileIn.close();
		fileOut.close();
//		"Could write file during meta data update.";
		return false;
	}

	fileIn.close();
	fileOut.close();

#ifdef WIN32
	//Erase original file.
	if (!DeleteFile(fileName.c_str()))
	{
//		"Failed to delete old file during meta data update.";
		return false;
	}
	//Rename temp file.
	if (!MoveFile(fileNameOut.c_str(), fileName.c_str()))
	{
//		"Failed to move file during meta data update.";
		return false;
	}
#else
#pragma message("WARNING: TODO: program cross-platform version of this")
#endif

	return true;
}

#endif
