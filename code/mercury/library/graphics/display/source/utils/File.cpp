#include "stdafx.h"
#include "Display/Utils/File.h"
#include <System/Serialise/I_FileSystem.h>

namespace Display
{

U64 GetFileTime(const Util::C_String& fileName)
{
	const Sys::C_FilePath file = Sys::GetDefaultFileSystem()->GetWorkingDirectory() + "/" + fileName;
	
	Sys::C_FileInfo fileInfo;
	if(Sys::GetDefaultFileSystem()->FileInfo(file, &fileInfo))
	{
		return fileInfo.m_FileTime;
	}
	else
	{
		return 0;
	}
}

bool GetFileData(const Util::C_String& fileName, U32& fileSize, U8** ppFileData)
{
	fileSize	= 0;
	*ppFileData	= 0;

	bool result = false;

	if (Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(fileName))
	{
		if ((fileSize = (U32)pFile->Length()) > 0)
		{
			if ((*ppFileData = new U8[fileSize]) != 0)
			{
				if (pFile->Read(*ppFileData, fileSize))
				{
					result = true;
				}
				else
				{
					LOGWARN(g_Log, "Failed to read file '%s'\n", fileName.Chars());
				}
			}
			else
			{
				LOGWARN(g_Log, "Out of memory reading '%s'\n", fileName.Chars());
			}
		}
		else
		{
			LOGWARN(g_Log, "File is empty '%s'\n", fileName.Chars());
		}

		delete pFile;
	}
	else
	{
		LOGWARN(g_Log, "Failed to open file '%s'\n", fileName.Chars());
	}

	if (!result)
	{
		delete [] *ppFileData;

		fileSize		= 0;
		*ppFileData		= 0;
	}

	return result;
}

}; // namespace Display
