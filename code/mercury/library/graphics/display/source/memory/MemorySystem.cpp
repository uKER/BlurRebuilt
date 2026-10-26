#include "stdafx.h"
#include "Display/Memory/MemorySystem.h"
#include "System/Memory/MemoryTracker.h"
#include <System/Serialise/TextSerialise.h>
#include <algorithm>
#include <system/debug/log/debuglog.h>
#include <system/debug/log/subscribers/LogFileSubscriber.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

Debug::Bool g_debugForceOutOfSystemMemory("System.Memory.ForceOutOfSystemMemory", false);

namespace Display
{

OutOfMemoryCallBackFunction* C_MemoryManagerSystem::m_pOutOfMemoryCallBackFunction = NULL;
Sys::TextSerialiser* C_MemoryManagerSystem::m_pTextSerialiser = NULL;

C_MemoryManagerSystem::C_MemoryManagerSystem()
{
}

C_MemoryManagerSystem::~C_MemoryManagerSystem()
{
}

void C_MemoryManagerSystem::SetOutOfMemoryCallBackFunction(OutOfMemoryCallBackFunction* pFunc)
{
	m_pOutOfMemoryCallBackFunction = pFunc;
}

void C_MemoryManagerSystem::OutOfMemory()
{
	if(m_pOutOfMemoryCallBackFunction)
	{
		(*m_pOutOfMemoryCallBackFunction)();
	}
	else
	{
		C_MemoryManager::OutOfMemory(E_Memory_System);
	}
}

void C_MemoryManagerSystem::Update()
{
	if(g_debugForceOutOfSystemMemory)
	{
		OutOfMemory();
		g_debugForceOutOfSystemMemory = false;
	}
}

void C_MemoryManagerSystem::DumpUsage()
{
	Memory::Tracker::Instance().SetOutputCallBackFunction(Output);

	Output("\n:: System Memory Usage ::\n\n");
	Memory::Tracker::Instance().DumpNodeStats(false, false);

	Memory::Tracker::Instance().SetOutputCallBackFunction(NULL);
}

void C_MemoryManagerSystem::DumpUsage(const Util::C_String &filePath)
{
	const Util::C_String txtFilePath = filePath + ".txt";
	const Util::C_String csvFilePath = filePath + ".csv";

	Memory::Tracker::Instance().SetOutputCallBackFunction(Output);

	// output to txt file
	{
		Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(txtFilePath.Chars(), Sys::E_FileAccess_Write);
		if(pFile)
		{
			m_pTextSerialiser = new Sys::TextSerialiser(*pFile);

			Output("\n:: System Memory Usage ::\n\n");
			Memory::Tracker::Instance().DumpNodeStats(false, false);

			SAFE_DELETE_PTR(m_pTextSerialiser);
			SAFE_DELETE_PTR(pFile);
		}
		else
		{
			LOGERROR(g_Log, "Failed to open file '%s'\n", txtFilePath.Chars());
		}
	}

	// output to csv file
	{
		Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(csvFilePath.Chars(), Sys::E_FileAccess_Write);
		if(pFile)
		{
			m_pTextSerialiser = new Sys::TextSerialiser(*pFile);

			Memory::Tracker::Instance().DumpNodeStats(true, false);

			SAFE_DELETE_PTR(m_pTextSerialiser);
			SAFE_DELETE_PTR(pFile);
		}
		else
		{
			LOGERROR(g_Log, "Failed to open file '%s'\n", csvFilePath.Chars());
		}
	}
	
	Memory::Tracker::Instance().SetOutputCallBackFunction(NULL);
}

void C_MemoryManagerSystem::Output(char* output)
{
	if(m_pTextSerialiser)
	{
		const char* constOutput = output;
		(*m_pTextSerialiser) << constOutput;
	}
	else
	{
		#ifdef PLATFORM_PS3
		printf(output);
		#else
		OutputDebugString(output);
		#endif
	}
}

}; // namespace Display
