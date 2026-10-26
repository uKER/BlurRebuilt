#ifndef TU_FILE_UTILITIES_H
#define TU_FILE_UTILITIES_H

#include <sys/stat.h>
#include <iostream>
#ifndef YDL
#include <io.h>
#endif

#define CHAR4_TO_U32(value) ((U32)(((value>>24)&0xff)|((value>>8)&0xff00)|((value<<8)&0xff0000)|((value<<24)&0xff000000)))

#ifdef GCC
#include <unistd.h>
#include <sys/types.h>
#define ACCESS(A,B) access(A,B)
#else
#define ACCESS(A,B) _access(A,B)
#endif

namespace Tu
{

inline static bool FileExists(const char* fileName)
{
	// Let's see if we can open the output file.
	int test = ACCESS(fileName, 0);
	if (test == 0)
		return true;
	else
		return false;
}

inline static bool FileExists(const std::string& fileName)
{
	// Let's see if we can open the output file.
	int test = ACCESS(fileName.c_str(), 0);
	if (test == 0)
		return true;
	else
		return false;
}


inline bool static FileSize(const char* fileName, U32 &size)
{
	struct stat results;

	if (stat(fileName, &results) == 0)
	{
		// The size of the file in bytes is in
		// results.st_size
		size = results.st_size;
		return true;
	}

	return false;
}

#ifndef GCC
// JAN: TODO: port this function to GCC
inline bool FileDate(const char* fileName, U32& lowDate, U32& highDate)
{
	WIN32_FILE_ATTRIBUTE_DATA fad;
	if (GetFileAttributesEx(fileName, GetFileExInfoStandard, &fad))
	{
		lowDate = *(U32*)&fad.ftLastWriteTime.dwLowDateTime;
		highDate = *(U32*)&fad.ftLastWriteTime.dwHighDateTime;
		return true;
	}
	return false;
}
#endif

inline S32 FindInFile(const char* fileName, const char* searchTarget, U32 searchTargetSize)
{
	if (!FileExists(fileName))
	{
		return -1;
	}

	U32 bytesToRead;
	if (!FileSize(fileName, bytesToRead))
	{
		return -1;
	}
	if (bytesToRead == 0)
	{
		return -1;
	}

	bool* pSearchTargetFlags = new bool[searchTargetSize];
	for (U32 i = 0; i < searchTargetSize; i++)
		pSearchTargetFlags[i] = false;

	std::ifstream file(fileName, std::ios::in | std::ios::binary);

	bool found = false;
	U32 highestCharFound = 0;
	U32 filePos = 0;
	//Search chunk for meta data
	for (filePos = 0; filePos < bytesToRead; filePos++)
	{
		char buffer;
		file.read(&buffer, 1);
		if (!file)
		{
			//Abort
			break;
		}

		for (U32 i = highestCharFound; i < searchTargetSize; i++)
		{
			if (searchTarget[i] == buffer)
			{
				pSearchTargetFlags[i] = true;
				highestCharFound++;
				break;
			}
			else if (!pSearchTargetFlags[i])
			{
				//We didn't find what we were looking for, restart our search for the target string.
				for (U32 i = 0; i < highestCharFound; i++)
					pSearchTargetFlags[i] = false;
				highestCharFound = 0;
				break;
			}
		}

		if (highestCharFound == searchTargetSize)
		{
			found = true;
			break;
		}
	}
	delete[] pSearchTargetFlags;

	file.close();
	if (found)
	{
		return filePos-searchTargetSize+1;
	}
	else
	{
		return -1;
	}
}

}

#endif
