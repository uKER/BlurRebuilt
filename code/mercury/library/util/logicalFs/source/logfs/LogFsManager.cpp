#include "LogicalFsPch.h"

#include <Core/Containers/String.h>
#include <System/Debug/Debug.h>
#include <System/Debug/Log/Format.h>
#include <LogFs/LogFs.h>
#include <LogFs/LogFsManager.h>

namespace LogicalFs
{

Debug::Bool g_logFsEnable("Util.LogicalFs.LogFs.Enable", false, Debug::Persist());
Debug::Bool g_logToDisk("Util.LogicalFs.LogFs.LogToDisk", false, Debug::Persist());

void C_LogFsManager::Initialise(const Sys::C_FilePath& outputPath)
{
	if (m_initialised)
		return;

	m_outputPath = outputPath;

	m_pLfSubscriber = NULL;
	m_pAccessLfSubscriber = NULL;

	m_openingLogFile = false;

	//Flip state to ensure we invoke a state change.
	m_logToDisk = !g_logToDisk;
	HandleLogToDiskState();

	if (g_logFsEnable)
	{
		if (m_preInitAccessArray.Size() > 0)
		{
			LOGINFO(m_logger, "<Pre LogFs init activity>");
			for (S32 i = 0; i < m_preInitMsgArray.Size(); ++i)
				_dbgMessageX (m_logger, m_preInitMsgArray[i].level, m_preInitMsgArray[i].msg.Chars());
			m_preInitMsgArray.Destroy();
			LOGINFO(m_logger, "</Pre LogFs init activity>");
		}

		if (m_preInitAccessArray.Size() > 0)
		{
			for (S32 i = 0; i < m_preInitAccessArray.Size(); ++i)
				LOGINFO(m_accessLogger, m_preInitAccessArray[i].msg.Chars());
			m_preInitAccessArray.Destroy();
		}
	}

	m_initialised = true;
}

void C_LogFsManager::OnLogFsCreated(Sys::I_FileSystem* pLogFs, const char* name)
{
	m_mutex.Wait();
	m_regFsList.PushBack(new C_RegFs(name, pLogFs));
	m_mutex.Release();
}

void C_LogFsManager::OnLogFsDestroy(Sys::I_FileSystem* pLogFs)
{
	m_mutex.Wait();
	for (RegFsList::Iterator it = m_regFsList.Begin(); it != m_regFsList.End(); ++it)
	{
		if ((*it)->pFileSystem == pLogFs)
		{
			delete *it;
			m_regFsList.Erase(it);
			break;
		}
	}
	m_mutex.Release();
}

C_LogFsManager::C_LogFsManager() :
	m_logger(Debug::GetLogger("mercury.logicalfs.logfs")),
	m_accessLogger(Debug::GetLogger("mercury.logicalfs.logfs.access")),
	m_pLfSubscriber(NULL),
	m_pAccessLfSubscriber(NULL),
	m_initialised(false)
{
	m_accessLogger.SetPropagateMessages(false);
	m_accessLogger.SetLevel(DebugLevel::Debug);
}

const C_LogFsManager::RegFsList& C_LogFsManager::GetRegFsList() const
{
	return m_regFsList;
}

void C_LogFsManager::PushEvent(const char* name)
{
	if (!m_mutex.Wait())
		return;

	EventStack* pStack = AcquireThreadContextStack(MT::GetThreadId());
	if (!pStack) return;

	if (pStack->Size() < pStack->Capacity())
		pStack->PushBack(name);

	m_mutex.Release();
}

void C_LogFsManager::PopEvent()
{
	if (!m_mutex.Wait())
		return;

	EventStack* pStack;
	PerThreadEventStack::Iterator it = m_perThreadEventStack.Find(MT::GetThreadId());
	if (it != m_perThreadEventStack.End())
	{
		pStack = &it->GetValue();
		if (pStack->Empty())
			m_perThreadEventStack.Erase(it);
	}
	m_mutex.Release();
}

void C_LogFsManager::LogMessage(const char* fsName, DebugLevel::Level level, char* format, ...)
{
	if (m_openingLogFile)
		//No choice but to lose this message.
		return;

	m_mutex.Wait();
	if (g_logFsEnable)
	{
		HandleLogToDiskState();

		const char* eventName = "";
		{
			EventStack* pStack;
			PerThreadEventStack::Iterator it = m_perThreadEventStack.Find(MT::GetThreadId());
			if (it != m_perThreadEventStack.End())
			{
				pStack = &it->GetValue();
				if (pStack->Size() > 0)
					eventName = pStack->Back().Chars();
			}
		}	
		char buffer1[1024];
		U64 tId = MT::GetThreadId();
		if (strlen(eventName) == 0)
			sprintf_s(buffer1, lengthof(buffer1), 
				#ifdef COMPILER_VC
				"TID: %I64u FS: \"%s\" ", 
				#else
				"TID: %llu FS: \"%s\" ",
				#endif
				tId, fsName);
		else
			sprintf_s(buffer1, lengthof(buffer1), 
				#ifdef COMPILER_VC
				"TID: %I64u FS: \"%s\" CTX: \"%s\" ", 
				#else
				"TID: %llu FS: \"%s\" CTX: \"%s\" ",
				#endif
				tId, fsName, eventName);
		char buffer2[1024];
		va_list arglist;
		va_start(arglist, format);
		vsprintf_s(buffer2, lengthof(buffer2), format, arglist);
		va_end(arglist);

		strcat_s(buffer1, lengthof(buffer1), buffer2);

		if (!m_initialised)
		{
			if (m_preInitMsgArray.Size() < LOGFS_PREINIT_MSG_MAX)
			{
				m_preInitMsgArray.PushBack();
				m_preInitMsgArray.Back().level = level;
				m_preInitMsgArray.Back().msg = buffer1;
			}
		}
		else
		{
			_dbgMessageX (m_logger, level, buffer1);
		}
	}
	m_mutex.Release();
}

void C_LogFsManager::LogFileAccess(const char* name, const Sys::C_FilePath& filePath)
{
	if (m_openingLogFile)
		//No choice but to lose this access.
		return;

	m_mutex.Wait();
	if (g_logFsEnable && g_logToDisk)
	{
		char buffer[1024];
		sprintf_s(buffer, lengthof(buffer), "\"%s\" \"%s\"", name, filePath.Chars());

		if (!m_initialised)
		{
			if (m_preInitAccessArray.Size() < LOGFS_PREINIT_MSG_MAX)
			{
				m_preInitAccessArray.PushBack();
				m_preInitAccessArray.Back().level = DebugLevel::Info;
				m_preInitAccessArray.Back().msg = buffer;
			}
		}
		else
		{
			LOGINFO(m_accessLogger, buffer);
		}
	}
	m_mutex.Release();
}

C_LogFsManager::EventStack* C_LogFsManager::AcquireThreadContextStack(MT::ThreadId id)
{
	PerThreadEventStack::Iterator it = m_perThreadEventStack.Find(id);
	if (it != m_perThreadEventStack.End())
		return &it->GetValue();

	PerThreadEventStack::InsertionReturnType r = m_perThreadEventStack.Insert(id);
	if (r.inserted == false)
	{
		BIZ_ASSERT_FAIL("Failed to acquire per thread context stack!");
		return 0;
	}

	return &r.iterator->GetValue();
}

void C_LogFsManager::HandleLogToDiskState()
{
	m_mutex.Wait();
	
	if (g_logToDisk != m_logToDisk)
	{
		if (m_pLfSubscriber)
		{
			m_logger.RemoveSubscriber(m_pLfSubscriber);
			SAFE_DELETE_PTR(m_pLfSubscriber);
		}
		if (m_pLfSubscriber)
		{
			m_accessLogger.RemoveSubscriber(m_pAccessLfSubscriber);
			SAFE_DELETE_PTR(m_pAccessLfSubscriber);
		}
		if (g_logToDisk)
		{
			m_openingLogFile = true;
			m_logger.SetPropagateMessages(false);
			m_logger.SetLevel(DebugLevel::Debug);

			m_pLfSubscriber = new Debug::LogFileSubscriber((m_outputPath + "/logfs.log").Chars());
			{
				Util::C_String fmt("%t %5p %c\n");
				m_pLfSubscriber->Format().SetFormatString(fmt);
			}
			m_logger.AddSubscriber(m_pLfSubscriber);

			m_pAccessLfSubscriber = new Debug::LogFileSubscriber((m_outputPath + "/logfs_access.log").Chars());
			{
				Util::C_String fmt("%c\n");
				m_pAccessLfSubscriber->Format().SetFormatString(fmt);
			}
			m_accessLogger.AddSubscriber(m_pAccessLfSubscriber);
			
			m_openingLogFile = false;
		}
		else
		{
			m_logger.SetPropagateMessages(true);
			m_logger.SetLevel(DebugLevel::Debug);
		}
		m_logToDisk = g_logToDisk;
	}

	m_mutex.Release();
}


}
