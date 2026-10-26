#include "stdafx.h"
#include "uberCopy/UberCopy.h"

using namespace UberCopy;


bool C_UberCopy::CopyItem(
	const Tools::FileCopy::C_FileCopyItem &copyItem,
	Util::E_Platform platform,
	const Util::C_String *console)
{
	switch (copyItem.m_FileCopyType)
	{
	case Tools::FileCopy::C_FileCopyItem::E_FileCopyItemType_Directory:
		return RecursivelyCopyDirectory(copyItem.m_FileCopySource, copyItem.m_FileCopyDestination, platform, console);
		break;
	case Tools::FileCopy::C_FileCopyItem::E_FileCopyItemType_File:
		return CopySingleFile(copyItem.m_FileCopySource, copyItem.m_FileCopyDestination, platform, console);
		break;
	default:
		LOGINFO(g_Log, "UberCopy: Unknown copying task type\n");
	}
	return false;
}


bool C_UberCopy::CopyUnknownItem(
	const Util::C_String &src,
	const Util::C_String &dst,
	Util::E_Platform platform,
	const Util::C_String *console)
{
	Sys::C_FileInfo fileInfo;
	Sys::C_FileSystem::Instance().FileInfo(src, &fileInfo);

	if (fileInfo.m_FileType == Sys::E_FileType_File)
		return CopySingleFile(src, dst, platform, console);
	else if (fileInfo.m_FileType == Sys::E_FileType_Directory)
		return RecursivelyCopyDirectory(src, dst, platform, console);
	else if (fileInfo.m_FileType == Sys::E_FileType_NotPresent)
		LOGINFO(g_Log, "The specified path does not exist %s\n", src.Chars());

	return false;
}


bool C_UberCopy::CopyList(
	const Tools::FileCopy::C_FileCopyList &copyList,
	bool skipPlatformOnFailure,
	U32 platforms)
{
	bool result = true;
	for (U32 i=0; i<Util::E_Platform_Count; ++i)
	{
		if (!(Tools::FileCopy::GetBitForPlatform((Util::E_Platform)i) & platforms))
			continue;

		const Tools::FileCopy::FileCopyItemArray platformCopyList
			= copyList.GetFileCopyItemArray((Util::E_Platform)i);

		const Tools::FileCopy::C_FileCopyOption *pConsoleList
			= copyList.FindFileCopyOptionByName((Util::E_Platform)i, "console");

		for (U32 j=0; j < (U32)platformCopyList.Size(); ++j)
		{
			if (pConsoleList)
			{
				Util::C_String consoleNameList = pConsoleList->m_Value;
				S32 separatorIndex = consoleNameList.Search(',');
				
				while (separatorIndex > 0)
				{
					Util::C_String console = consoleNameList(0,separatorIndex);
					result &= CopyItem(platformCopyList[j], (Util::E_Platform)i, &console);
					consoleNameList = consoleNameList(separatorIndex+1,consoleNameList.Length());
					separatorIndex = consoleNameList.Search(',');
				}
				result &= CopyItem(platformCopyList[j], (Util::E_Platform)i, &consoleNameList);
			}
			else
				result &= CopyItem(platformCopyList[j], (Util::E_Platform)i);

			if (!result)
			{
				if (skipPlatformOnFailure)
					break;
				else
					return result;
			}
		}
	}
	return result;
}


bool C_UberCopy::CopyXmlList(
	const Tools::FileCopy::C_FileCopyListXml &xmlCopyList,
	bool skipPlatformOnFailure,
	U32 platforms)
{
	Tools::FileCopy::C_FileCopyList copylist;

	if(!xmlCopyList.CopyToFileCopyList(copylist))
		return false;

	return CopyList(copylist, skipPlatformOnFailure, platforms);
}


bool C_UberCopy::RecursivelyCopyDirectory(
	const Util::C_String &src,
	const Util::C_String &dst,
	Util::E_Platform platform,
	const Util::C_String *console)
{
	Sys::C_FileInfo fileInfo;
	Sys::C_FileSystem &fileSystem = Sys::C_FileSystem::Instance();

	fileSystem.FileInfo(src, &fileInfo);

	if (fileInfo.m_FileType == Sys::E_FileType_NotPresent)
	{
		LOGINFO(g_Log, "The specified path does not exist %s\n", src.Chars());
		return false;
	}
	else if (fileInfo.m_FileType == Sys::E_FileType_File)
	{
		LOGERROR(g_Log, "The specified path is not a directory %s\n", src.Chars());
		return false;
	}

	Sys::C_DirectoryInfo directoryInfo;
	fileSystem.DirectoryInfo(src + "\\", &directoryInfo);

	bool result = true;

	for (U32 i=0; i<directoryInfo.NumFiles(); ++i)
	{
		Sys::C_FileInfo fileInfo = directoryInfo.FileInfo(i);
		Util::C_String path = dst + "\\" + Sys::ExtractFilename(fileInfo.m_FileName).Chars();

		switch(fileInfo.m_FileType)
		{
		case Sys::E_FileType_Directory:
			result &= RecursivelyCopyDirectory(fileInfo.m_FileName.Chars(), path, platform, console);
			break;

		case Sys::E_FileType_File:
			result &= CopySingleFile(fileInfo.m_FileName.Chars(), path, platform, console);
			break;

		default:
			break;
		}
	}

	return result;
}

bool C_UberCopy::CopySingleFile(
	const Util::C_String &src,
	const Util::C_String &dst,
	Util::E_Platform platform,
	const Util::C_String *console)
{
	Sys::C_FileInfo fileInfo;
	Sys::C_FileSystem &fileSystem = Sys::C_FileSystem::Instance();

	fileSystem.FileInfo(src, &fileInfo);
	if (fileInfo.m_FileType != Sys::E_FileType_File)
	{
		LOGINFO(g_Log, "%s is not a file\n", src.Chars());
		return false;
	}

	switch(platform)
	{
	case Util::E_Platform_PC:
		LOGINFO(g_Log, "Copying to PC: %s to %s\n", src.Chars(), dst.Chars());
		return Tools::FileCopy::FileCopyUtils::CopyFile(src, dst);
		break;

	case Util::E_Platform_PS3:
		LOGINFO(g_Log, "Copying to PC(PS3): %s to %s\n", src.Chars(), dst.Chars());
		return Tools::FileCopy::FileCopyUtils::CopyFile(src, dst);
		break;

	case Util::E_Platform_360:
		LOGINFO(g_Log, "Copying to 360: %s to %s\n", src.Chars(), dst.Chars());
		return Tools::FileCopy::FileCopyUtils360::Instance().CopyFile(src, dst,console);
		break;

	default:
		LOGINFO(g_Log, "Unknown destination platform: %s to %s\n", src.Chars(), dst.Chars());
		break;
	}
	return false;
}