#include "LogicalFsPch.h"
#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/PakHelpers.h>

namespace LogicalFs
{
namespace PakHelpers
{

void FindPakFiles(Btl::Vector<Sys::C_FilePath>& pakFiles, Sys::I_FileSystem& fs, const Sys::C_FilePath& dir)
{
	Sys::C_DirectoryInfo dInfo;
	if (!fs.DirectoryInfo(dir, &dInfo))
		return;

	for(U32 i = 0; i < dInfo.NumFiles(); i++)
	{
		Sys::C_FileInfo& fInfo = dInfo.FileInfo(i);
		if(fInfo.m_FileType == Sys::E_FileType_Directory)
		{
			FindPakFiles(pakFiles, fs, fInfo.m_FileName);
		}
		else if(fInfo.m_FileType == Sys::E_FileType_File)
		{
			if (Misc::ExtractFileExtension(fInfo.m_FileName.Chars()) == "pak")
			{
				pakFiles.PushBack(fInfo.m_FileName);
			}
		}		            
	}
}

C_PakFs* OpenPak(Sys::I_FileSystem& fs, const Sys::C_FilePath& filePath, const Security::I_Security* pSecurity, const Sys::C_OpenParams& params)
{
	C_PakFs* pPakFs = new C_PakFs();
	if (!pPakFs->OpenPak(
		&fs,
		filePath,
		pSecurity,
		params))
		SAFE_DELETE_PTR(pPakFs);

	return pPakFs;
}

}
}
