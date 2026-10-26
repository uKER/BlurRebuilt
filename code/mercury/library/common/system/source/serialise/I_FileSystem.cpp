#include "SystemPch.h"

#include <system/serialise.h>
#include <system/serialise/I_FileSystem.h>
#include <system/misc/stringutil.h>

bool Sys::I_FileSystem::CreateDirectory( const C_FilePath& _filePath ) 
{
	C_FilePath filePath = ConvertToCurrentDirectory(GetWorkingDirectory(), _filePath);

	if (FileExists(filePath))
		return true;

	//Create output folder.
	Btl::Vector<C_FilePath> directories;
	filePath.TokeniseByDir(directories);
	if (directories.Size() == 0)
		return false;

	C_FilePath path;
	S32 startOffset;
	if (filePath[0] == GetLocalDirToken())
	{
		startOffset = 0;
	}
	else 
	{
		path = directories[0];
		startOffset = 1;
	}
#ifdef PLATFORM_PC
	//Account for the fact that we might be creating a folder on a network share.
	if (_filePath.Search(C_FilePath("\\\\")) != -1)
	{
		if (directories.Size() < 2)
			return false;

		path = C_FilePath("\\\\") + directories[0] +  C_FilePath("\\") + directories[1];
		startOffset = 2;
	}
#endif
	for (S32 i = startOffset; i < directories.Size(); i++)
	{
		path += C_FilePath("\\") + directories[i];

		if (FileExists(path))
			continue;
		if (!CreateSingleDirectory(path))
			return false;
	}

	return true;
}
