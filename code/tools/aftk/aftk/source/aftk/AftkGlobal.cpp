#include <stdafx.h>

// include FTKs
#include "Ftk.h"
#include <system/debug/log/debuglog.h>

#include "../native_io/NativeFtk.h"

//XSI FTK not available for 64-bit systems.
//Maya API not available either.
#ifdef AFTK_XSI
#include "../xsi_io/XsiIo.h"
#endif
#ifdef AFTK_MAYA_IO
#include "../maya_io/MayaIo.h"
#endif
#ifdef AFTK_FBX
#include "../fbx_io/FbxIo.h"
#endif

namespace Aftk
{
	static const U32 AFTK_VERSION = 200;
}

using namespace Aftk;

bool C_Global::m_created = false;

bool C_Global::Create()
{
	S_DEBUG(g_aftkMainLogger, "Aftk::C_Global::Create() invoked.\n");
	bool status = C_MemoryManager::Create();
	if (!status)
		return status;
	m_created = true;
	return true;
}

bool C_Global::Created()
{
	return m_created;
}

void C_Global::Destroy()
{
	S_DEBUG(g_aftkMainLogger, "Aftk::C_Global::Destroy() invoked.\n");
	C_MemoryManager::Destroy();
	m_created = false;
}


U32 C_Global::GetVersion()
{
	return AFTK_VERSION;
}

std::string C_Global::GetBuildStamp()
{
	const char* version = "Version: ABCD.EF.GH (HH:MM)";
	return std::string(version);
}

std::string C_Global::GetBuildPc()
{
	const char* buildPC = "BuildPC:                                                                                                                                                                                                                                                               ";
	return std::string(buildPC);
}

std::string C_Global::GetSvnRevision()
{
	const char* svnRevision = "SVN Rev:                                                                                                                                                                                                                                                               ";
	std::string svnRevisionStr(svnRevision);
	std::string::size_type pos = svnRevisionStr.find("\n");
	if (pos != std::string::npos)
	{
		svnRevisionStr = svnRevisionStr.substr(0, pos);	
	}
	
	return svnRevisionStr;
}

C_Status C_Global::GetHighestSupportedFileVersion(E_FileType format, U32& version)
{
	switch (format)
	{
		case E_FileType_A3d:
		{
			version = C_NativeFtk::GetHighestSupportedFileVersion();
			return C_Status::E_Status_Success;
		}
		#ifdef AFTK_XSI
		case E_FileType_Xsi:
		{
			version = XsiIo::C_Main::GetHighestSupportedFileVersion();
			return C_Status::E_Status_Success;
		}
		#endif
		#ifdef AFTK_MAYA_IO
		case E_FileType_Mb:
		case E_FileType_Ma:
		{
			version = MayaIo::C_Main::GetHighestSupportedFileVersion();
			return C_Status::E_Status_Success;
		}
		#else
			return C_Status(C_Status::E_Status_NotImplemented, "This version of the  was not built with Maya support.", 1);
		#endif
		default:
		{
			C_Status status;
			status.Set(C_Status::E_Status_Failure, "Could not determine highest supported file version.", 1);
			return C_Status::E_Status_Success;
		}
	}

	return C_Status::E_Status_Success;
}

C_Status C_Global::GetLowestSupportedFileVersion(E_FileType format, U32& version)
{
	switch (format)
	{
		case E_FileType_A3d:
		{
			version = C_NativeFtk::GetLowestSupportedFileVersion();
			return C_Status::E_Status_Success;
		}
		#ifdef AFTK_XSI
		case E_FileType_Xsi:
		{
			version = XsiIo::C_Main::GetLowestSupportedFileVersion();
			return C_Status::E_Status_Success;
		}
		#endif
		default:
		{
			C_Status status;
			status.Set(C_Status::E_Status_Failure, "Could not determine lowest supported file version.", 1);
			return status;
		}
	}

	return C_Status::E_Status_Success;
}

C_Status C_Global::GetFileType(const std::string& fileName, E_FileType& format)
{
	C_Status status;
	// Look at the file extension to determine what type of file we are to load
	std::string extension;
	U32 length = (U32)fileName.length();
	if (length < 5)
	{
		status.Set(C_Status::E_Status_Failure, "Could not determine file type to read from its extension.", 1);
		return status;
	}

	extension = fileName.substr(length-3, 3);
	std::transform(extension.begin(), extension.end(), extension.begin(), tolower); 

	if (extension == std::string("a3d")) 
	{
		format = E_FileType_A3d;
		return C_Status::E_Status_Success;

	}
	else if (extension == std::string("xsi")) 
	{
		format = E_FileType_Xsi;
		return C_Status::E_Status_Success;
	}
	else if (extension == std::string(".mb")) 
	{
		format = E_FileType_Mb;
		return C_Status::E_Status_Success;
	}
	else if (extension == std::string(".ma")) 
	{
		format = E_FileType_Ma;
		return C_Status::E_Status_Success;
	}
	else if (extension == std::string("fbx")) 
	{
		format = E_FileType_Fbx;
		return C_Status::E_Status_Success;
	}

	format = E_FileType_Unknown;
	return C_Status::E_Status_Success;
}

C_Status C_Global::GetFileVersion(const std::string& fileName, U32& version)
{
	C_Status status;

	E_FileType format;
	status = GetFileType(fileName, format);
	if (!status)
	{
		return status;
	}

	switch (format)
	{
		case E_FileType_A3d:
		{
			C_NativeFtk nativeFtk;
			return nativeFtk.GetFileVersion(
				fileName, 
				version);
		}
		#ifdef AFTK_XSI
		case E_FileType_Xsi:
		{
			XsiIo::C_Main xsiIoMain;
			return xsiIoMain.GetFileVersion(
				fileName, 
				version);
		}
		#endif
		#ifdef AFTK_MAYA_IO
		case E_FileType_Mb:
		case E_FileType_Ma:
		{
			MayaIo::C_Main mayaIoMain;
			return mayaIoMain.GetFileVersion(
				fileName, 
				version);
		}
		#else
			return C_Status(C_Status::E_Status_NotImplemented, "This version of the  was not built with Maya support.");
		#endif
		default:
		{
			status.Set(C_Status::E_Status_Failure, "Could not determine file version.", 1);
		}
	}

	return C_Status::E_Status_Failure;
}

void C_Global::OutputText(E_OutputChannel channel, const std::string& text)
{
	switch (channel)
	{
		case E_OutputChannel_Debug:
			S_DEBUG(g_aftkMainLogger, text);
			break;
		case E_OutputChannel_Info:
			S_INFO(g_aftkMainLogger, text);
			break;
		case E_OutputChannel_Warning:
			S_WARN(g_aftkMainLogger, text);
			break;
		case E_OutputChannel_Error:
			S_ERROR(g_aftkMainLogger, text);
			break;
		case E_OutputChannel_Fatal:
			S_FATAL(g_aftkMainLogger, text);
			break;
		default:
			break;
	}
}

std::string C_Global::GetFileExtension(E_FileType format)
{
	switch (format)
	{
		case E_FileType_BestGuess:
		{
			return std::string("");
		}
		case E_FileType_Xsi:
		{
			return std::string("xsi");
		}
		case E_FileType_A3d:
		{
			return std::string("a3d");
		}
		case E_FileType_Mb:
		{
			return std::string("mb");
		}
		case E_FileType_Ma:
		{
			return std::string("ma");
		}
		default:
		{
			return std::string("");
		}
	}
}

StringArray C_Global::GetReadSupportedFileDescriptions() 
{
	StringArray stringArray;
	stringArray.PushBack(std::string("Abstract 3D"));
	#ifdef AFTK_XSI
	stringArray.PushBack(std::string("Softimage XSI"));
	#endif
	#ifdef AFTK_MAYA_IO
	stringArray.PushBack(std::string("Maya Binary"));
	stringArray.PushBack(std::string("Maya ASCII"));
	#endif
	#ifdef AFTK_FBX
	stringArray.PushBack(std::string("FBX"));
	#endif

	return stringArray;
}

StringArray C_Global::GetWriteSupportedFileDescriptions() 
{
	StringArray stringArray;

	stringArray.PushBack(std::string("Abstract 3D"));
	#ifdef AFTK_XSI
	stringArray.PushBack(std::string("Softimage XSI"));
	#endif

	return stringArray;
}


StringArray C_Global::GetReadSupportedFileExtensions() 
{
	StringArray stringArray;

	stringArray.PushBack(std::string("a3d"));

	#ifdef AFTK_XSI
	stringArray.PushBack(std::string("xsi"));
	#endif

	#ifdef AFTK_MAYA_IO
	stringArray.PushBack(std::string("mb"));
	stringArray.PushBack(std::string("ma"));
	#endif

	#ifdef AFTK_FBX
	stringArray.PushBack(std::string("fbx"));
	#endif

	return stringArray;
}

StringArray C_Global::GetWriteSupportedFileExtensions() 
{
	StringArray stringArray;
	stringArray.PushBack(std::string("a3d"));

	return stringArray;
}

