#include "SystemPch.h"

#include <core/threads/Misc.h>
#include <system/time/time.h>
#include <system/serialise/FsPerfLogger.h>
#include <system/debug/Variables.h>	// [Blur2PC] Debug::Bool

namespace Sys
{

static void DumpFileAccessOrder(void* unused)
{
	C_FsPerfLogger::Instance().DumpFileAccessOrder();
}

Debug::Bool g_fsPerfLogEvents("System.Io.FsPerfLogger.LogEvents", false, Debug::Persist());
Debug::Bool g_fsPerfLogAccessOrder("System.Io.FsPerfLogger.LogAccessOrder", 
#if defined(USER_KMACLEOD)
								   true,
#else
								   false, 
#endif
								   Debug::Persist());
Debug::String g_fsPerfBreakOnFile("System.Io.FsPerfLogger.BreakOnFile", "", Debug::Persist());
Debug::String g_fsPerfDumpAccessPrefix("System.Io.FsPerfLogger.DumpAccessPrefix", "", Debug::Persist());
Debug::Function g_fsPerfDumpAccessOrder("System.Io.FsPerfLogger.DumpAccessOrder", DumpFileAccessOrder, 0);

extern Debug::Logger &g_FileSystemGlobalsLog;
Debug::Logger &g_FileSystemAccessOrderLog = Debug::GetLogger("Mercury.FileSystemGlobals.FileAccessOrder");

bool C_FsPerfLogger::m_sIsDead = false;

F32 GetDurationSecsF(U64 delta)
{
	U32 durationMs = (U32)(delta / 1000);
	return durationMs / 1000.f;
}

void BreakOnFile(const char* filename)
{
	//Break on a particular file?
	const char* str = g_fsPerfBreakOnFile.Value();
	if (strlen(str) > 0)
	{
		if (!stricmp(filename, str)) {
			LOGDEBUG(g_FileSystemGlobalsLog, "BreakOnFile");
		}
	}
}

C_FsPerfLogger::C_FsPerfLogger()
{
	m_startTime = Time::GetPerformanceCounter();
	m_mainThreadId = 0;
}

C_FsPerfLogger::~C_FsPerfLogger()
{
	//BIZ_ASSERT(m_fsStatsMap.Size() == 0);
}

void C_FsPerfLogger::GetFileReports(const Btl::String& fsName, Btl::Vector<C_FsFileInfo>& reports) const
{
	if (m_sIsDead) return;

	m_fsStatsMutex.Wait();

	FsPerfHashMap::ConstIterator it = m_fsStatsMap.Find(fsName);
	if (it != m_fsStatsMap.End())
	{
		const Btl::Vector<U64>& v = it->GetValue().fsStats.openFiles;

		for (S32 i = 0; i < v.Size(); ++i)
		{
			U64 handle = v[i];
			FsFileInfoHashMap::ConstIterator fileIt = m_fsFileMap.Find(handle);
			if (fileIt != m_fsFileMap.End())
				reports.PushBack(fileIt->GetValue());
		}
	}

	m_fsStatsMutex.Release();
}

void C_FsPerfLogger::GetFsReports(Btl::Vector<C_FsReport>& reports) const
{
	if (m_sIsDead) return;

	m_fsStatsMutex.Wait();

	reports.Resize(m_fsStatsMap.Size());
	S32 i = 0;
	for (FsPerfHashMap::ConstIterator it = m_fsStatsMap.Begin(); it != m_fsStatsMap.End(); ++it, ++i)
	{
		C_FsReport& r = reports[i];
		r.fsName = it->GetKey();
		r.stats = it->GetValue().fsStats;
	}

	m_fsStatsMutex.Release();
}

void C_FsPerfLogger::GetGlobalFsStats(C_GlobalFsStats& globalFsStats) const
{
	if (m_sIsDead) return;

	m_fsStatsMutex.Wait();

	globalFsStats = m_globalFsStats;

	m_fsStatsMutex.Release();
}

void C_FsPerfLogger::OnFileOpened(const Btl::String& fsName, const Sys::C_FilePath& fileName, U64 handle, E_FileAccess access)
{
	if (m_sIsDead) return;

	m_fsStatsMutex.Wait();

	FsPerfHashMap::Iterator it = m_fsStatsMap.Find(fsName);
	C_PerFsData* perFsData(NULL);
	if (it == m_fsStatsMap.End())
	{
		FsPerfHashMap::InsertionReturnType r = m_fsStatsMap.Insert(fsName, C_PerFsData());
		if (r.iterator == m_fsStatsMap.End())
		{
			BIZ_ASSERT_FAIL("Failed to insert file perf entry!");
			m_fsStatsMutex.Release();
			return;
		}
		perFsData = &r.iterator->GetValue();
	}
	else
	{	
		perFsData = &it->GetValue();
	}

	if (m_fsFileMap.Find(handle) != m_fsFileMap.End())
	{
		BIZ_ASSERT_FAIL("Perf logger attempt to open file twice!");
		m_fsStatsMutex.Release();
		return;
	}

	perFsData->fsStats.openFiles.PushBack(handle);
	if (g_fsPerfLogAccessOrder)
	{
		if (perFsData->uniqueFiles.Find(fileName) == perFsData->uniqueFiles.End())
		{
			perFsData->uniqueFiles.Insert(fileName);
			perFsData->fileAccessOrder.PushBack(fileName);
		}
	}

	C_FsFileInfo fsFileInfo;
	fsFileInfo.fileName = fileName;
	fsFileInfo.access = access;
	m_fsFileMap.Insert(handle, fsFileInfo);

	++m_globalFsStats.numOpenFiles;
	if (m_globalFsStats.numOpenFiles > m_globalFsStats.peakOpenFiles)
		m_globalFsStats.peakOpenFiles = m_globalFsStats.numOpenFiles;

#ifndef CFG_GOLD
	if (g_fsPerfLogEvents){	
		LOGINFO(g_FileSystemGlobalsLog, "%.2fsecs%s FsOpen %s %s", 
			GetDurationSecsF(Time::GetPerformanceCounter() - m_startTime),
			MT::GetThreadId() == m_mainThreadId ? " MAIN THREAD!" : "",
			fsName.Chars(), fsFileInfo.fileName.Chars());
		BreakOnFile(fsFileInfo.fileName.Chars());
	}
#endif
	m_fsStatsMutex.Release();
}

void C_FsPerfLogger::OnFileClosed(const Btl::String& fsName, U64 handle)
{
	if (m_sIsDead) return;

	m_fsStatsMutex.Wait();

	//Find file system.
	FsPerfHashMap::Iterator it = m_fsStatsMap.Find(fsName);
	if (it != m_fsStatsMap.End())
	{
		//Find file.
		FsFileInfoHashMap::Iterator fileIt = m_fsFileMap.Find(handle);
		C_FsFileInfo* pFileInfo = 0;
		if (fileIt != m_fsFileMap.End())
			pFileInfo = &fileIt->GetValue();
		else
		{
			BIZ_ASSERT_FAIL("Failed to erase file hash entry");
			m_fsStatsMutex.Release();
			return;
		}

		#ifndef CFG_GOLD
		if (g_fsPerfLogEvents){	
			if (pFileInfo)
			{
				LOGINFO(g_FileSystemGlobalsLog, "%2.fsecs%s FsClose %s %s", 
					GetDurationSecsF(Time::GetPerformanceCounter() - m_startTime),
					MT::GetThreadId() == m_mainThreadId ? " MAIN THREAD!" : "",
					fsName.Chars(), pFileInfo->fileName.Chars());
				BreakOnFile(pFileInfo->fileName.Chars());
			}
		}
		#endif

		//Find file entry in list of open files.
		Btl::Vector<U64>& v = it->GetValue().fsStats.openFiles;
		for (S32 i = v.Size()-1; i >= 0; --i)
		{
			U64 currentHandle = v[i];
			if (currentHandle == handle)
			{
				//Free file info structure.
				v.EraseByIndex(i);
				--m_globalFsStats.numOpenFiles;

				//If we're logging file access, don't delete our access log per fs.
				if (v.Size() == 0 && !g_fsPerfLogAccessOrder)
					m_fsStatsMap.Erase(it);
				break;
			}
		}

		m_fsFileMap.Erase(fileIt);
	}

	m_fsStatsMutex.Release();
}

void C_FsPerfLogger::OnFileRead(const Btl::String& fsName, U64 handle, U64 pos, U64 readBytes, bool physical, const char* meta)
{
	if (readBytes == 0)
		return;
	if (m_sIsDead) return;

	m_fsStatsMutex.Wait();
	FsFileInfoHashMap::Iterator it = m_fsFileMap.Find(handle);
	if (it != m_fsFileMap.End())
	{
		C_FsFileInfo& fileInfo = it->GetValue();

		fileInfo.readBytes += readBytes;
		if (physical)
			m_globalFsStats.readBytesPhysical += readBytes;
		else
			m_globalFsStats.readBytesVirtual += readBytes;

		FsPerfHashMap::Iterator fsIt = m_fsStatsMap.Find(fsName);
		if (fsIt != m_fsStatsMap.End())
		{
			C_FsStats& fsStats = fsIt->GetValue().fsStats;
			fsStats.readBytes += readBytes;
		}
		else
		{
			BIZ_ASSERT_FAIL("Perf logger failed to find fs %s", fsName.Chars());
		}

		#ifndef CFG_GOLD
		if (g_fsPerfLogEvents)
		{	
		#ifdef COMPILER_VC
			const char* fmt = "%.2fsecs%s TID %I64u %s %s %s offset: %I64u read: %I64ubytes %s";
		#else
			const char* fmt = "%.2fsecs%s TID %llu %s %s %s offset: %llu read: %llubytes %s";
		#endif
			LOGINFO(g_FileSystemGlobalsLog, fmt, 
				GetDurationSecsF(Time::GetPerformanceCounter() - m_startTime),
				MT::GetThreadId() == m_mainThreadId ? " MAIN THREAD!" : "",
				MT::GetThreadId(),
				physical ? "FsPhysicalRead" : "FsVirtualRead", 
				fsName.Chars(), 
				fileInfo.fileName.Chars(), 
				pos,
				readBytes,
				meta ? meta : "");
			BreakOnFile(fileInfo.fileName.Chars());
		}
		#endif
	}
	else
	{
		BIZ_ASSERT_FAIL("Perf logger failed to find handle %s", fsName.Chars());
	}
	m_fsStatsMutex.Release();
}

void C_FsPerfLogger::OnFileWrite(const Btl::String& fsName, U64 handle, U64 writeBytes, bool physical)
{
	if (m_sIsDead) return;

	m_fsStatsMutex.Wait();
	FsFileInfoHashMap::Iterator it = m_fsFileMap.Find(handle);
	if (it != m_fsFileMap.End())
	{
		C_FsFileInfo& fileInfo = it->GetValue();
		fileInfo.writeBytes += writeBytes;
		if (physical)
			m_globalFsStats.writeBytesPhysical += writeBytes;
		else
			m_globalFsStats.writeBytesVirtual += writeBytes;

		FsPerfHashMap::Iterator fsIt = m_fsStatsMap.Find(fsName);
		if (fsIt != m_fsStatsMap.End())
		{
			C_FsStats& fsStats = fsIt->GetValue().fsStats;
			fsStats.writeBytes += writeBytes;
		}
		else
		{
			BIZ_ASSERT_FAIL("Perf logger failed to find fs %s", fsName.Chars());
		}

		#ifndef CFG_GOLD
		if (g_fsPerfLogEvents){	
			LOGINFO(g_FileSystemGlobalsLog, "%.2fsecs%s %s %s %s %ubytes",
				GetDurationSecsF(Time::GetPerformanceCounter() - m_startTime),
				MT::GetThreadId() == m_mainThreadId ? " MAIN THREAD!" : "",
				physical ? "FsPhysicalWrite" : "FsVirtualWrite", 
				fsName.Chars(), fileInfo.fileName.Chars(), (U32)writeBytes);
			BreakOnFile(fileInfo.fileName.Chars());
		}
		#endif
	}
	else
	{
		BIZ_ASSERT_FAIL("Perf logger failed to find handle %s", fsName.Chars());
	}
	m_fsStatsMutex.Release();
}

void C_FsPerfLogger::OnFsRename(const Btl::String& fsNameOld, const Btl::String& fsNameNew)
{
	m_fsStatsMutex.Wait();
	FsPerfHashMap::Iterator it = m_fsStatsMap.Find(fsNameOld);
	if (it != m_fsStatsMap.End())
	{
		//Copy...
		C_PerFsData perFsData = it->GetValue();
		m_fsStatsMap.Erase(it);

		FsPerfHashMap::InsertionReturnType r = m_fsStatsMap.Insert(fsNameNew, perFsData);
		if (r.iterator == m_fsStatsMap.End())
		{
			BIZ_ASSERT_FAIL("Failed to insert file perf entry!");
			m_fsStatsMutex.Release();
			return;
		}
	}

	m_fsStatsMutex.Release();
}

void C_FsPerfLogger::DumpFileAccessOrder()
{
	m_fsStatsMutex.Wait();

	Sys::C_FilePath prefix((const char*)g_fsPerfDumpAccessPrefix);
	for (FsPerfHashMap::Iterator it = m_fsStatsMap.Begin(); it != m_fsStatsMap.End(); ++it)
	{
		const Btl::Vector<Sys::C_FilePath>& files = it->GetValue().fileAccessOrder;
		LOGINFO(g_FileSystemAccessOrderLog, "Dumping file access order XML for %s", it->GetKey().Chars());

		LOGINFO(g_FileSystemAccessOrderLog, "<workitems>");
		for (S32 i = 0; i < files.Size(); ++i)
		{
			LOGINFO(g_FileSystemAccessOrderLog, "<item filepath=\"%s\"/>", (prefix + files[i]).Chars());
		}
		LOGINFO(g_FileSystemAccessOrderLog, "</workitems>");
	}

	m_fsStatsMutex.Release();
}

void C_FsPerfLogger::SetMainThreadId(U64 id)
{
	m_mainThreadId = id;
}

void C_FsPerfLogger::Shutdown()
{
	m_sIsDead = true;
}

}


