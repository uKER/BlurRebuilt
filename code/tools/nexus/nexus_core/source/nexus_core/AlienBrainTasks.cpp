#include "stdafx.h"
#ifdef NEXUS_ALIENBRAIN
#include <nexus_core/AlienBrainTasks.h>
#endif
#include <nexus_core/core.h>

#ifdef NEXUS_ALIENBRAIN

#include <shellapi.h>	// SHFileOperation for recursive file deletion 
#include <system/shell/shellexec.h>
#include <system/serialise/C_FileSystem.h>
namespace Nexus
{

AlienBrainTasks AlienBrainTasks::s_instance;

////////////////////////////////////////////////////////
// Nexus Specific Functions
////////////////////////////////////////////////////////
Tu::Status AlienBrainTasks::ABGetLatest(const char* pCommand, U32 nRetries)
{
	if (!AlienBrainTasks::Instance().Init())
	{
		g_pCore->PrintMessage("AlienBrainTasks::Instance().Init() failure.\n");
		return Tu::Status::FAILURE;
	}

	g_pCore->PrintMessage(std::string("ABGetLatest : ") + pCommand);
	
	if (!AlienBrainTasks::Instance().GetLatest(pCommand, nRetries))
	{
		g_pCore->PrintMessage("ABGetLatest failure.");
		return Tu::Status(Tu::Status::FAILURE, std::string("Error: File or Directory not found: ") + pCommand + "\n");
	}
	else
	{
		g_pCore->PrintMessage("ABGetLatest okay.");
	}

	return Tu::Status::SUCCESS;
}

int AlienBrainTasks::ABGetLatestLua(lua_State* pLuaState)
{
	std::string str = lua_tostring(pLuaState, 1);
	U32 nRetries = 5;

	//ABSyncLatest(<path>,<ABuname>, <ABpw>, <retries>)
	if (lua_gettop(pLuaState) > 3)
		nRetries = (U32) lua_tonumber(pLuaState, 4);

	lua_pop(pLuaState, 1);

	Tu::Status status = ABGetLatest(str.c_str(), nRetries);
	
	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

Tu::Status AlienBrainTasks::ABSyncLatest(const char* pCommand, U32 nRetries, bool forceOnFailure)
{
	AlienBrainTasks::Instance().Init();

	g_pCore->PrintMessage(std::string("ABSyncLatest: ") + pCommand + "\n");

	if (!AlienBrainTasks::Instance().SyncLatest(pCommand, nRetries, forceOnFailure))
	{
		return Tu::Status(Tu::Status::FAILURE, std::string("Error: File or Directory not found: ") + pCommand + "\n");
	}

	return Tu::Status::SUCCESS;
}

int AlienBrainTasks::ABSyncLatestLua(lua_State* pLuaState)
{
	std::string str = lua_tostring(pLuaState, 1);
	U32 nRetries = 5;
	bool forceOnFailure = true;

	//ABSyncLatest(<path>,<ABuname>, <ABpw>, <retries>, <forcegetlatest>)
	if (lua_gettop(pLuaState) > 3)
		nRetries = (U32) lua_tonumber(pLuaState, 4);

	if (lua_gettop(pLuaState) > 4)
		forceOnFailure = (lua_toboolean(pLuaState, 5)==1);

	lua_pop(pLuaState, 1);

	Tu::Status status = ABSyncLatest(str.c_str(), nRetries, forceOnFailure);
	//Return result
	return GlobalTasks::ProcessResult(pLuaState, status);
}

void AlienBrainTasks::Register(lua_State* pLuaState)
{
	//Register Lua functions
	lua_register( pLuaState, "ABGetLatest", &AlienBrainTasks::ABGetLatestLua);
	lua_register( pLuaState, "ABSyncLatest", &AlienBrainTasks::ABSyncLatestLua);
}

////////////////////////////////////////////////////////
// Alienbrain specific functions
////////////////////////////////////////////////////////

// initialize alienbrain
AlienBrainTasks::AlienBrainTasks()
{
}

bool AlienBrainTasks::Init()
{
	if (!m_pIntegrator)
	{
		m_pIntegrator = new CNxNSmartIntegrator();
		
		if (!m_pIntegrator)
			return false;
		
		if (!m_pIntegrator->InitInstance(false))
		{
			delete m_pIntegrator;
			m_pIntegrator = NULL;
			return false;
		}
	}

	return true;
}

void AlienBrainTasks::Term()
{
	if (m_pIntegrator)
	{
		m_pIntegrator->ExitInstance();
		delete m_pIntegrator;
		m_pIntegrator = NULL;
	}
}

bool AlienBrainTasks::SyncLatest(const char *psz, U32 nRetries, bool forceOnFailure)
{
	CNxNString nxnpath;

	if (!m_pIntegrator->MapManagedPath(psz, nxnpath))
	{
		return false; // can't sync latest on local file
	}
	
	CNxNFolder *pFolder = (CNxNFolder*)m_pIntegrator->GetManagedNode(nxnpath, CNxNFolder::GetType());
	if (!pFolder)
	{
		//we're only interested in folders for syncing
		return false;
	}

	//now, recurse every file in this folder, and see if it exists in the database. 
	//If not, delete it.

	//flip slashes for windows!
	char buffer[260];
	memset(buffer, 0, sizeof(char)*260);
	strncpy_s(buffer, psz, 260);
	char *cp = buffer;

	while (*cp)
	{
		if (*cp == '/')
		{
			*cp = '\\';
		}	
		cp++;	
	}

	//add trailing slash if not present

	size_t length = strlen(buffer);
	if (buffer[length-1] == '\\')
	{
		RecurseDeleteFilesNotInAB(CNxNString(buffer));
	}
	else
	{
		RecurseDeleteFilesNotInAB(CNxNString(buffer) + "\\");
	}

	//finally, do a get latest on the folder
	bool bSuccess = pFolder->GetLatest(false, false, false, false, false, "", true, false);

	for (U32 i=0; i<nRetries && !bSuccess; ++i)
	{
		std::stringstream message;
		message << "ABSyncLatest Failed... retrying" << i << "/" << nRetries << std::endl;
		g_pCore->PrintError(message.str());
		bSuccess = pFolder->GetLatest(false, false, false, false, false, "", true, false);
	}

	delete pFolder;

	if (forceOnFailure && !bSuccess)
	{
		g_pCore->PrintError("All ABSyncLatest retries have failed ... performing an ABGetLatest from scratch\n");

		/*
		SHFILEOPSTRUCT fileOp;
		fileOp.hwnd = NULL;
		fileOp.wFunc = FO_DELETE;
		fileOp.pFrom = buffer;		//**Should be double null terminated**
		fileOp.pTo = NULL;
		fileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI;
		fileOp.fAnyOperationsAborted = TRUE;
		fileOp.hNameMappings = NULL;
		fileOp.lpszProgressTitle = NULL;

		if (SHFileOperation(&fileOp) != 0)
			g_pCore->PrintWarning("Failed to properly delete folder prior to performing ABGetLatest\n");
		*/
		U32 exitCode = Sys::ShellExec::ExecuteCommandAndWait(
			"cmd",
			Util::C_String("/C rmdir /S /Q \"") + buffer + "\"",
			Sys::C_FileSystem::Instance().GetWorkingDirectory(),
			Sys::ShellExec::E_LaunchState_Minimise);

		if (exitCode != 0)
			g_pCore->PrintWarning("Failed to properly delete folder prior to performing ABGetLatest\n");

		bSuccess = GetLatest(psz, nRetries);

	}

	return bSuccess;

}

void AlienBrainTasks::RecurseDeleteFilesNotInAB(CNxNString folderName)
{
	//for debugging: ABSyncLatest("z:\\shitstorm\\assets\\levels\\manor2\\")
	WIN32_FIND_DATA fileData;
	
	HANDLE handle = FindFirstFile(folderName + "*.*", &fileData);
	if (handle == INVALID_HANDLE_VALUE)
		return;

	bool bGoing=true;
	while (bGoing)
	{
		CNxNString fileAndPath = folderName + fileData.cFileName;

		bool bDirectory = ((GetFileAttributes(fileAndPath) & FILE_ATTRIBUTE_DIRECTORY) != 0);
		if (bDirectory)
		{
			if(strcmp(".", fileData.cFileName) && strcmp("..", fileData.cFileName))
			{
				RecurseDeleteFilesNotInAB(fileAndPath + "\\");
			}
		}
		else
		{
			//can we map the file in AB? If not, make sure its not readonly and delete
			CNxNString nxnpath;
			m_pIntegrator->MapManagedPath(fileAndPath, nxnpath);
			CNxNFile *pFile = (CNxNFile*)m_pIntegrator->GetManagedNode(nxnpath, CNxNFile::GetType());

			if (!pFile)
			{
				//is it read only?

				DWORD attr = GetFileAttributes(fileAndPath);
				if (attr&FILE_ATTRIBUTE_READONLY)
				{
					SetFileAttributes(fileAndPath, attr&=~FILE_ATTRIBUTE_READONLY);
				}
				
				//delete it

				DeleteFile(fileAndPath);
			}
			else
			{
				delete pFile;
			}
		}
		(FindNextFile(handle, &fileData)==TRUE)?(bGoing = true):(bGoing = false);
	}
}

// get the latest version of a particular file
bool AlienBrainTasks::GetLatest(const char *psz, U32 nRetries)
{
	CNxNString nxnpath;

	if (!m_pIntegrator->MapManagedPath(psz, nxnpath))
	{
		return false; // don't need to get latest on local file
	}

	// First test if we're getting a file
	CNxNFile *pFile = (CNxNFile*)m_pIntegrator->GetManagedNode(nxnpath, CNxNFile::GetType());

	// If this isn't a file, test if its a folder
	if (!pFile)
	{
		CNxNFolder *pFolder = (CNxNFolder*)m_pIntegrator->GetManagedNode(nxnpath, CNxNFolder::GetType());
		if (!pFolder)
			return false;

		bool bSuccess = pFolder->GetLatest(false, false, false, false, false, "", true, false);
		for (U32 i=0; i<nRetries && !bSuccess; ++i)
		{
			std::stringstream message;
			message << "ABGetLatest Failed... retrying" << i << "/" << nRetries << std::endl;
			g_pCore->PrintMessage(message.str());
			bSuccess = pFolder->GetLatest(false, false, false, false, false, "", true, false);
		}
		delete pFolder;

		return bSuccess;
	}

	bool bSuccess = pFile->GetLatest(false, false, false, false, false, "", true, false);
	for (U32 i=0; i<nRetries && !bSuccess; ++i)
	{
		std::stringstream message;
		message << "ABGetGetLatest Failed... retrying" << i+1 << "/" << nRetries << std::endl;
		g_pCore->PrintMessage(message.str());
		bSuccess = pFile->GetLatest(false, false, false, false, false, "", true, false);
	}
	delete pFile;

	return bSuccess;
}

// release alienbrain
AlienBrainTasks::~AlienBrainTasks()
{
	Term();
}

}

#endif