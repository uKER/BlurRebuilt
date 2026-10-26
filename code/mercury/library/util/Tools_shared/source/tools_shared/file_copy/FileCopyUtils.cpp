#include "stdafx.h"

#include <MMSystem.h>

#include <core/utility/Singleton.h>

#include <system/serialise/C_FileSystem.h>
#include <system/time/Time.h>
#include <system/shell/ShellExec.h>

#include <tools_shared/PlatformHelpers.h>
#include <tools_shared/Utility/MacroExpander.h>
#include <tools_shared/file_copy/FileCopyUtils.h>
#include <tools_shared/file_copy/FileCopyPlatforms.h>

#if defined(FILE_COPY_XDEVKIT)
 #include <atlbase.h>		//for CComBSTR
 #include <atlcomtime.h>	//for COleDateTime
 #include <XDevkit.h>

#define HRESULT_DEVKIT_CHECK(retval,message, ...) if (hr!=S_XDEVKIT_NOERR && hr!=S_OK) { LOGWARN(g_log, message, __VA_ARGS__); return retval; }

#elif defined(FILE_COPY_XBDM)
  #include <Xbdm.h>
  #pragma message("Using xbdm ... copying files will only work for default console")
#endif

#if defined(FILE_COPY_XDEVKIT) && defined(FILE_COPY_XBDM)
  #error("Using both xdevkit & xbdm for file copying?!? o_O");
#endif

#if !defined(FILE_COPY_XDEVKIT) && !defined(FILE_COPY_XBDM)
 #pragma message("Not using xbdm or xdevkit ... copying files will require xbcp to work!")
#endif


using namespace Tools::FileCopy;

S32 GetUNCSharePathOffset(const Util::C_String& filePath)
{
	//ie. "\\servername\sharefolder\"
	Util::C_String formattedFilePath = Util::FormatFileString(filePath);
	S32 position = 0;
	if (filePath(0,2) == "\\\\")
	{
		position = formattedFilePath.Search('/', 3);
		position = formattedFilePath.Search('/', position+1);
	}
	return position;
}


static void FileTimeToAprTime(U64 *result, const FILETIME *input)
{
    /* Convert FILETIME one 64 bit number so we can work with it. */
    *result = input->dwHighDateTime;
    *result = (*result) << 32;
    *result |= input->dwLowDateTime;
    *result /= 10;    /* Convert from 100 nano-sec periods to micro-seconds. */
	*result -= SYS_DELTA_EPOCH_IN_USEC;  /* Convert from Windows epoch to Unix epoch */
    return;
}


Util::C_String FileCopyUtils::GetExecutableDirectory()
{
	char appPath[MAX_PATH] = "";
	GetModuleFileName(0, appPath, sizeof(appPath) - 1);
	return Util::ExtractDirectory(appPath);
};


Util::C_String FileCopyUtils::GetExecutableName()
{
	char appPath[MAX_PATH] = "";
	GetModuleFileName(0, appPath, sizeof(appPath) - 1);
	return Util::RemoveFileExtension(  Util::ExtractFilename(appPath) );
};


Util::C_String FileCopyUtils::GetCurrentDirectory()
{
	char appPath[MAX_PATH] = "";
	::GetCurrentDirectory(sizeof(appPath) - 1, appPath );
	return Util::C_String(appPath) + "\\";
};


bool FileCopyUtils::MakeWritable(const Util::C_String& fileName)
{
	Sys::C_FileInfo fileInfo;
    
	if(Sys::C_FileSystem::Instance().FileInfo(fileName, &fileInfo))
	{
		switch( fileInfo.m_FileType )
		{
			case Sys::E_FileType_File:
			case Sys::E_FileType_Directory:
				// check if read only
				if(fileInfo.m_FileAccess != (Sys::E_FileAccess_Read | Sys::E_FileAccess_Write))
				{
					// make read-write
					return Sys::C_FileSystem::Instance().FileAccess( fileName, Sys::E_FileAccess_Read | Sys::E_FileAccess_Write);
				}
			break;

			case Sys::E_FileType_NotPresent:
			break;
		}
	}
	return false;
};


bool FileCopyUtils::CreateOutputPath(const Util::C_String& filePath)
{
	Util::C_String formattedFilePath;
	S32 pos=0;
	{
		S32 uncOffset = GetUNCSharePathOffset(filePath);
		if (uncOffset == 0)
		{
			formattedFilePath = Util::FormatFileString(filePath);
			S32 driveOffset = formattedFilePath.Search(":/");
			if (driveOffset>0)
				pos = driveOffset+2;
		}
		else
		{
			if(uncOffset+1 >= filePath.Length())
				return true;
			else
			{
				formattedFilePath = filePath(0,uncOffset+1) +
					Util::FormatFileString(filePath(uncOffset+1, filePath.Length()));
				pos = uncOffset+1;
			}
		}
	}

	while(true)
	{
		if ((pos<0) || (pos>= formattedFilePath.Length()-1))
			break;

		pos++;
		pos = formattedFilePath.Search('/', pos);

		Util::C_String partialPath = formattedFilePath(0, pos);

		Sys::C_FileInfo fileInfo;
		Sys::C_FileSystem::Instance().FileInfo(partialPath, &fileInfo);
                    
		if( fileInfo.m_FileType == Sys::E_FileType_Directory )
		{
			// check if read only
			if(fileInfo.m_FileAccess != (Sys::E_FileAccess_Read | Sys::E_FileAccess_Write))
			{
				// make read/write-able
				Sys::C_FileSystem::Instance().FileAccess( partialPath, Sys::E_FileAccess_Read | Sys::E_FileAccess_Write);
			}
		}
		else if( fileInfo.m_FileType == Sys::E_FileType_File )
		{
			// a file exists with the same name as the directory we are creating
			return false;
		}
		else if( fileInfo.m_FileType == Sys::E_FileType_NotPresent )
		{
			// create directory
			if( !Sys::C_FileSystem::Instance().CreateDirectory(partialPath))
			{
				return false;
			}
		}
	}
	return true;
};


bool FileCopyUtils::CopyFile( const Util::C_String& src, const Util::C_String& dst, int numRetries, int waitTimeMs )
{
	Util::C_String srcDir = Util::ExtractDirectory(src), dstDir = Util::ExtractDirectory(dst);
	Util::C_String srcFile = Util::ExtractFilename(src), dstFile = Util::ExtractFilename(dst);

	if (srcDir.Lower() != dstDir.Lower() || srcFile.Lower() != dstFile.Lower())
	{
		if(!CreateOutputPath(dstDir))
		{
			LOGWARN(g_log, "Could not create directory\n");
			return false;
		}

		BOOL exitCode = 0;

		for (int i=0; i<numRetries; i++)
		{
			MakeWritable(dst);
			exitCode = ::CopyFile(src.Chars(), dst.Chars(), false);

			if (exitCode)
				break;
			else
			{
				int lastError = GetLastError();
				Sleep (waitTimeMs);
			}
		}

		if (!exitCode) {
			LOGWARN(g_log, "Could not copy %s to %s\n", src.Chars(), dst.Chars());
			return false;
		}
	} else {
		LOGWARN(g_log, "Could not copy %s to %s: Source and destination file are the same!\n", src.Chars(), dst.Chars());
		return false;
	}
	return true;
};


E_TimeCompare FileCopyUtils::ShouldCopyFiles(const Util::C_String &src, const Util::C_String &dst)
{
	Sys::C_FileInfo srcInfo;
	Sys::C_FileInfo dstInfo;

	Sys::C_FileSystem::Instance().FileInfo(src, &srcInfo);
	Sys::C_FileSystem::Instance().FileInfo(dst, &dstInfo);

	if ((srcInfo.m_FileType==Sys::E_FileType_NotPresent) ||
		(dstInfo.m_FileType==Sys::E_FileType_NotPresent))
		return E_TimeCompare_Unknown;

	if (srcInfo.m_FileTime<dstInfo.m_FileTime)
		return E_TimeCompare_DestinationNewer;

	if (srcInfo.m_FileTime>dstInfo.m_FileTime)
		return E_TimeCompare_SourceNewer;

	return E_TimeCompare_Same;
}







FileCopyUtils360::FileCopyUtils360()
{
#if defined(FILE_COPY_XDEVKIT)

	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
		LOGERROR(g_log, "Error initialising COM\n");

	hr = CoCreateInstance(
			 __uuidof(XboxManager),
			 NULL,
			 CLSCTX_INPROC_SERVER,
			 __uuidof(IXboxManager),
			 &(void*&)m_pXboxManager );

	if (FAILED(hr))
		LOGERROR(g_log, "Error getting XboxManager interface\n");

/*
	IXboxConsoles *pXboxConsoles;
	hr = m_pXboxManager->get_Consoles(&pXboxConsoles);
	if (FAILED(hr))
	{
		LOGERROR(g_log, "Error getting console list\n");
		return;
	}

	IEnumVARIANT *variantEnum;
	hr = pXboxConsoles->get__NewEnum(&variantEnum);
	if (FAILED(hr))
	{
		LOGERROR(g_log, "Failed to get console list iterator\n");
		return;
	}

	LONG nConsoles = 0;
	pXboxConsoles->get_Count(&nConsoles);

	for(S32 i=0; i<nConsoles; ++i)
	{
		char buffer[256];
		VARIANT v;
		VariantInit(&v);
		hr = variantEnum->Next(1, &v, NULL);

		wcstombs(buffer, v.bstrVal,256);
		m_Consoles.PushBack(Util::C_String(buffer));
	}
*/
#endif
}


FileCopyUtils360::~FileCopyUtils360()
{
#if defined(FILE_COPY_XDEVKIT)
	CoUninitialize();
#endif
}


const Util::C_String FileCopyUtils360::FormatFileString(const Util::C_String src)
{
	Util::C_String dest = Util::FormatFileString(src);
	return dest.Replace("/", "\\");
};

#if defined(FILE_COPY_XDEVKIT)
IXboxConsole* FileCopyUtils360::GetConsoleInterface(const Util::C_String *console)
{
	CComBSTR consoleName;
	char consoleName_char[256];

	if (console)
		consoleName = console->Chars();
	else
	{
		assert(m_pXboxManager);

		HRESULT hr = m_pXboxManager->get_DefaultConsole(&consoleName.m_str);
		HRESULT_DEVKIT_CHECK(0,"Could not get default console name\n");
	}

	wcstombs(consoleName_char,consoleName.m_str,256);

	if(console && m_ConsoleName == *console)
	{
		// Already opened console interface
		return m_pXboxConsole;
	}
	else
	{
		IXboxConsole *pXboxConsole;
		LOGINFO(g_log, "Connecting to %s ...\n",consoleName_char);
		HRESULT hr = m_pXboxManager->OpenConsole(consoleName.m_str, &pXboxConsole);

		HRESULT_DEVKIT_CHECK(0,"Could not open console interface\n");

		hr = pXboxConsole->FindConsole(0,200);
		HRESULT_DEVKIT_CHECK(0,"Could not connect to console\n");

		m_pXboxConsole = pXboxConsole;
		m_ConsoleName = consoleName_char;
		return m_pXboxConsole;
	}
}
#endif

bool FileCopyUtils360::CreateOutputPath( const Util::C_String& filePath, const Util::C_String *console)
{
#if defined(FILE_COPY_XDEVKIT)

	IXboxConsole *pXboxConsole = GetConsoleInterface(console);
	if (!pXboxConsole)
		return false;

	CComBSTR path;
	Util::C_String formattedFilePath;
	S32 pos=0;
	{
		formattedFilePath = FormatFileString(filePath);
		S32 driveOffset = formattedFilePath.Search(":\\");
		if (driveOffset>0)
				pos = driveOffset+2;
	}

	while(true)
	{
		if ((pos<0) || (pos>= formattedFilePath.Length()-1))
			break;

		pos++;
		pos = formattedFilePath.Search('\\', pos);

		Util::C_String partialPath = formattedFilePath(0, pos);
		CComBSTR partialPathBSTR = partialPath.Chars();

		IXboxFile *pXboxFile;
		HRESULT hr = pXboxConsole->GetFileObject(partialPathBSTR.m_str, &pXboxFile);
		if (hr != S_XDEVKIT_NOERR)
		{
			if(hr == E_XDEVKIT_NOSUCHFILE)
			{
				hr = pXboxConsole->MakeDirectory(partialPathBSTR.m_str);
				HRESULT_DEVKIT_CHECK(false,"Could not create directory %s - error code %0x\n", partialPath.Chars(), hr);

			}
			else
				HRESULT_DEVKIT_CHECK(false,"Could not get attributes for %s - error code %0x\n", partialPath.Chars(), hr);
		}
		else
		{
			VARIANT_BOOL directory;

			hr = pXboxFile->get_IsDirectory(&directory);

			HRESULT_DEVKIT_CHECK(false,"Could not get attributes for %s - error code %0x\n", partialPath.Chars(), hr);

			if (directory)
			{
				VARIANT_BOOL readOnly;
				hr = pXboxFile->get_IsReadOnly(&readOnly);

				HRESULT_DEVKIT_CHECK(false,"Could not get attributes for %s - error code %0x\n", partialPath.Chars(), hr);
				if (readOnly)
				{
					hr = pXboxFile->put_IsReadOnly(0);
					HRESULT_DEVKIT_CHECK(false,"Could not set attribute for %s - error code %0x\n", partialPath.Chars(), hr);
				}
			}
			else
			{
				HRESULT_DEVKIT_CHECK(false,"Could not create directory %s - file already exists%0x\n", partialPath.Chars());
				return false;
			}
		}
	}
    return true;

#elif defined(FILE_COPY_XBDM)

	Util::C_String formattedFilePath;
	S32 pos=0;
	{
		formattedFilePath = FormatFileString(filePath);
		S32 driveOffset = formattedFilePath.Search(":\\");
		if (driveOffset>0)
				pos = driveOffset+2;
	}

	while(true)
	{
		if ((pos<0) || (pos>= formattedFilePath.Length()-1))
			break;

		pos++;
		pos = formattedFilePath.Search('\\', pos);

		Util::C_String partialPath = formattedFilePath(0, pos);

		DM_FILE_ATTRIBUTES dmFileAttributes;
		HRESULT result = DmGetFileAttributes( partialPath.Chars(), &dmFileAttributes );

		if(result != XBDM_NOERR)
		{
			if(result == XBDM_NOSUCHFILE)
			{
				result = DmMkdir( partialPath.Chars() );
				if(result != XBDM_NOERR)
				{
					LOGWARN(g_log, "Could not create directory %s - error code %0x\n", partialPath.Chars(), result);
					return false;
				}
			}
			else
			{
				LOGWARN(g_log, "Could not get attributes for %s - error code %0x\n", partialPath.Chars(), result);
				return false;
			}
		}
		else
		{
			if( (dmFileAttributes.Attributes & FILE_ATTRIBUTE_DIRECTORY) )
			{
				// check if read only
				if( (dmFileAttributes.Attributes & FILE_ATTRIBUTE_READONLY) )
				{
					LOGDEBUG(g_log, "File Is Read Only");
					// make read-write
					dmFileAttributes.Attributes &= ~FILE_ATTRIBUTE_READONLY;
					result = DmSetFileAttributes(partialPath.Chars(), &dmFileAttributes);
					if(result != XBDM_NOERR)
					{
						LOGWARN(g_log, "Could not set attributes for %s - error code %0x\n", partialPath.Chars(), result);
					}
				}
					
				// continue						
			}
			else if( !(dmFileAttributes.Attributes & FILE_ATTRIBUTE_DIRECTORY) )
			{
				// a file exists with the same name as the directory we are creating
				LOGWARN(g_log, "Could not create directory %s - file already exists%0x\n", partialPath.Chars());
				return false;
			}
		}
	}
    return true;

#else

	//shouldn't need to as xbcp /T creates directories for you
	return true;

#endif

};





bool FileCopyUtils360::CopyFile( const Util::C_String& src, const Util::C_String& dst, const Util::C_String *console)
{
#ifndef WRAP_32
#if defined(FILE_COPY_XDEVKIT)

	Util::C_String dstFile = FormatFileString(dst);

	//LOGINFO(g_log, "360 Copying : %s to %s\n", src.Chars(), dst.Chars());
	Util::C_String dstDir = Util::ExtractDirectory(dstFile);
	if(!CreateOutputPath(dstDir, console))
		return false;

	CComBSTR localpath = src.Chars();
	CComBSTR remotepath = dst.Chars();

	IXboxConsole *pXboxConsole = GetConsoleInterface(console);
	if (!pXboxConsole)
		return false;

	HRESULT hr = pXboxConsole->SendFile(localpath, remotepath);

	HRESULT_DEVKIT_CHECK(false,"Could not copy %s to %s\n", src.Chars(), dst.Chars());
	
	return true;

#elif defined(FILE_COPY_XBDM)

	Util::C_String dstFile = FormatFileString(dst);

	//LOGINFO(g_log, "360 Copying : %s to %s\n", src.Chars(), dst.Chars());
	Util::C_String dstDir = Util::ExtractDirectory(dstFile);
	if (!CreateOutputPath(dstDir))
		return false;

	HRESULT result;
	result = DmSendFile( src.Chars(), dstFile.Chars() );
	if(result != XBDM_NOERR)
	{
		LOGINFO(g_log, "Could not copy %s to %s\n", src.Chars(), dst.Chars());
		return false;
	}
	return true;

#else

	//NOTE: using /x when calling xbcp will override the default kit...
	Util::C_String params("/R /Q /Y /D /T ");
	if (console)
		params = params + "/X:" + *console + " \"" + src + "\" \"" + dst + "\"";
	else
		params = params + "\"" + src + "\" \"" + dst + "\"";

	S32 result = Sys::ShellExec::ExecuteCommandAndWait(
		(Sys::C_FileSystem::Instance().GetWorkingDirectory() + "\\xbcp.exe").Chars(),
		params.Chars(),
		Sys::C_FileSystem::Instance().GetWorkingDirectory().Chars(),
		Sys::ShellExec::E_LaunchState_Hide,
		&Tools::g_log,
		NULL,
		"",
		60000);// 10 minutes (milliseconds)

	if (result != 0)
	{
		LOGINFO(g_log, "Error copying %s to %s  xbcp error code %0x", src.Chars(), dst.Chars(), result);
		return false;
	}

	return true;
#endif
#endif
};



// Only copy if file on devkit is older
E_TimeCompare FileCopyUtils360::ShouldCopyFiles(const Util::C_String &src, const Util::C_String &dst, const Util::C_String *console)
{
#ifdef FILE_COPY_XDEVKIT

	Sys::C_FileInfo srcInfo;
	Sys::C_FileSystem::Instance().FileInfo(src, &srcInfo);
	if (srcInfo.m_FileType==Sys::E_FileType_NotPresent)
		return E_TimeCompare_Unknown;

	IXboxConsole *pXboxConsole = GetConsoleInterface(console);
	if (!pXboxConsole)
		return E_TimeCompare_Unknown;

	CComBSTR xboxDstPath = dst.Chars();

	IXboxFile *pXboxFile;
	HRESULT hr = pXboxConsole->GetFileObject(xboxDstPath.m_str, &pXboxFile);
	if (hr != S_OK && hr != S_XDEVKIT_NOERR)
		return E_TimeCompare_Unknown;

	VARIANT v;
	VariantInit(&v);
	hr = pXboxFile->get_ChangeTime(&v);

	if (hr != E_XDEVKIT_NOSUCHPATH)
	{
		COleDateTime srcTime(__time64_t(srcInfo.m_FileTime/1000000L));
		COleDateTime dstTime(v);
		
		if (srcTime < dstTime)
			return E_TimeCompare_DestinationNewer;

		if (srcTime > dstTime)
			return E_TimeCompare_SourceNewer;

		return E_TimeCompare_Same;
	}
	return E_TimeCompare_SourceNewer; 

#elif defined(FILE_COPY_XENON)

	DM_FILE_ATTRIBUTES dmFileAttributes;
	Sys::C_FileInfo srcInfo;

	Sys::C_FileSystem::Instance().FileInfo(src, &srcInfo);

	if (srcInfo.m_FileType==Sys::E_FileType_NotPresent)
		return E_TimeCompare_Unknown;

	HRESULT hr = DmGetFileAttributes(dst.Chars(), &dmFileAttributes);

	if (hr == XBDM_UNDEFINED)
		return E_TimeCompare_Unknown;

	if (hr != XBDM_NOSUCHFILE)
	{
		Time::C_Time dstTime;
		FileTimeToAprTime(&dstTime, &dmFileAttributes.ChangeTime);
		if (srcInfo.m_FileTime<dstTime)
			return E_TimeCompare_DestinationNewer;

		if (srcInfo.m_FileTime>dstTime)
			return E_TimeCompare_SourceNewer;

		return E_TimeCompare_Same;
	}
	return E_TimeCompare_SourceNewer;

#else

	return E_TimeCompare_Unknown;

#endif
}