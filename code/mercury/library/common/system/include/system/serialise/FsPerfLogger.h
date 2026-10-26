#ifndef SYSTEM_FS_PERF_LOGGER_H
#define SYSTEM_FS_PERF_LOGGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Utility/Singleton.h>
#include <Core/Threads/Mutex.h>
#include <System/Serialise/FilePath.h>
#include <System/Serialise/FilePathHashOps.h>
#include <Btl/Containers/OpenHashMap.h>
#include <Btl/Containers/OpenHashSet.h>
#include <Btl/Containers/Vector.h>
#include <Btl/Utility/hashops.h>
#include "FileSystemHelperTypes.h"

#if defined(CFG_GOLD) || defined(TOOL)
	#define FSPERF_SETMAINTHREADID(id)
	#define FSPERF_OPENED(fsname, filename, handle, access) 
	#define FSPERF_CLOSED(fsname, handle) 
	#define FSPERF_READ_PHYSICAL(fsname, handle, pos, readBytes, meta)
	#define FSPERF_WRITE_PHYSICAL(fsname, handle, writeBytes)
	#define FSPERF_READ_VIRTUAL(fsname, handle, pos, readBytes, meta)
	#define FSPERF_WRITE_VIRTUAL(fsname, handle, writeBytes)
	#define FSPERF_RENAME_FS(fsnameOld, fsNameNew)
#else
	#define FSPERF_SETMAINTHREADID(id) Sys::C_FsPerfLogger::Instance().SetMainThreadId(id);
	#define FSPERF_OPENED(fsname, filename, handle, access)	Sys::C_FsPerfLogger::Instance().OnFileOpened(fsname, filename, handle, access);
	#define FSPERF_CLOSED(fsname, handle)	Sys::C_FsPerfLogger::Instance().OnFileClosed(fsname, handle);
	#define FSPERF_READ_PHYSICAL(fsname, handle, pos, readBytes, meta) Sys::C_FsPerfLogger::Instance().OnFileRead(fsname, handle, pos, readBytes, true, meta);
	#define FSPERF_WRITE_PHYSICAL(fsname, handle, writeBytes) Sys::C_FsPerfLogger::Instance().OnFileWrite(fsname, handle, writeBytes, true);
	#define FSPERF_READ_VIRTUAL(fsname, handle, pos, readBytes, meta) Sys::C_FsPerfLogger::Instance().OnFileRead(fsname, handle, pos, readBytes, false, meta);
	#define FSPERF_WRITE_VIRTUAL(fsname, handle, writeBytes) Sys::C_FsPerfLogger::Instance().OnFileWrite(fsname, handle, writeBytes, false);
	#define FSPERF_RENAME_FS(fsNameOld, fsNameNew) Sys::C_FsPerfLogger::Instance().OnFsRename(fsNameOld, fsNameNew);
#endif

namespace Sys
{
	//Storage for individual file info.
	struct C_FsFileInfo
	{
		C_FsFileInfo() :
			access(E_FileAccess_Read),
			readBytes(0),
			writeBytes(0)
		{}

		C_FsFileInfo(const C_FsFileInfo& rhs)
		{
			*this = rhs;
		}
		const C_FsFileInfo& operator=(const C_FsFileInfo& rhs)
		{
			fsName = rhs.fsName;
			fileName = rhs.fileName;
			access = rhs.access;

			readBytes = rhs.readBytes;
			writeBytes = rhs.writeBytes;
			return *this;
		}

		Btl::String		fsName;
		Sys::C_FilePath fileName;
		E_FileAccess	access;

		U64 readBytes;
		U64 writeBytes;
	};

	//Storage for file-system info.
	struct C_FsStats
	{
		C_FsStats() :
			readBytes(0),
			writeBytes(0) {}
		~C_FsStats()
		{
		}
		C_FsStats(const C_FsStats& rhs)
		{
			*this = rhs;
		}

		const C_FsStats& operator=(const C_FsStats& rhs)
		{
			openFiles = rhs.openFiles;
			readBytes = rhs.readBytes;
			writeBytes = rhs.writeBytes;
			return *this;
		}

		Btl::Vector<U64> openFiles;
		
		U64 readBytes;
		U64 writeBytes;
	};

	//Query storage for an individual file system.
	struct C_FsReport
	{
		C_FsReport() {}
		C_FsReport(const C_FsReport& rhs)
		{
			*this = rhs;
		}

		const C_FsReport& operator=(const C_FsReport& rhs)
		{
			fsName = rhs.fsName;
			stats = rhs.stats;
			return *this;
		}

		Btl::String fsName;
		C_FsStats stats;
	};

	struct C_GlobalFsStats
	{
		C_GlobalFsStats() :
			numOpenFiles(0), 
			peakOpenFiles(0),
			readBytesPhysical(0),
			writeBytesPhysical(0), 
			readBytesVirtual(0),
			writeBytesVirtual(0) 
			{}
		C_GlobalFsStats(const C_GlobalFsStats& rhs)
		{
			*this = rhs;
		}
		const C_GlobalFsStats& operator=(const C_GlobalFsStats& rhs)
		{
			numOpenFiles = rhs.numOpenFiles;
			peakOpenFiles = rhs.peakOpenFiles;

			readBytesPhysical = rhs.readBytesPhysical;
			writeBytesPhysical = rhs.writeBytesPhysical;

			readBytesVirtual = rhs.readBytesVirtual;
			writeBytesVirtual = rhs.writeBytesVirtual;
			return *this;
		}

		U32 numOpenFiles;
		U32 peakOpenFiles;

		U64 readBytesPhysical;
		U64 writeBytesPhysical;
		U64 readBytesVirtual;
		U64 writeBytesVirtual;
	};

	class C_FsPerfLogger : public Ut::Singleton<C_FsPerfLogger, Ut::CreateStatic>
	{
	public:
		C_FsPerfLogger();
		virtual ~C_FsPerfLogger();

		void GetFileReports(const Btl::String& fsName, Btl::Vector<C_FsFileInfo>& reports) const;

		void GetFsReports(Btl::Vector<C_FsReport>& reports) const;

		void GetGlobalFsStats(C_GlobalFsStats& globalFsStats) const;

		void OnFileOpened(const Btl::String& fsName, const Sys::C_FilePath& fileName, U64 handle, E_FileAccess access);

		void OnFileClosed(const Btl::String& fsName, U64 handle);

		void OnFileRead(const Btl::String& fsName, U64 handle, U64 pos, U64 readBytes, bool physical, const char* meta = NULL);

		void OnFileWrite(const Btl::String& fsName, U64 handle, U64 writeBytes, bool physical);

		void OnFsRename(const Btl::String& fsNameOld, const Btl::String& fsNameNew);

		static void Shutdown();

		void DumpFileAccessOrder();

		void SetMainThreadId(U64 id);

	private:
		struct C_PerFsData
		{
			C_PerFsData() {}
			C_PerFsData(const C_PerFsData& rhs)
			{
				*this = rhs;
			}

			const C_PerFsData& operator=(const C_PerFsData& rhs)
			{
				fsStats = rhs.fsStats;
				uniqueFiles = rhs.uniqueFiles;
				fileAccessOrder = rhs.fileAccessOrder;
				return *this;
			}

			typedef Btl::OpenHashSet<Sys::C_FilePath, Sys::HashKeyOpFilePath > UniqueFileSet;

			C_FsStats						fsStats;
			//Unique, ordered file access tracking.
			UniqueFileSet					uniqueFiles;
			Btl::Vector<Sys::C_FilePath>	fileAccessOrder;
		};

		typedef Btl::OpenHashMap<Btl::String, C_PerFsData, Btl::HashKeyOpBtlString >			FsPerfHashMap;
		typedef Btl::OpenHashMap<U64, C_FsFileInfo>		FsFileInfoHashMap;

		//Hash table of stats per file system
		FsPerfHashMap		m_fsStatsMap;
		FsFileInfoHashMap	m_fsFileMap;
		

		C_GlobalFsStats		m_globalFsStats;
		U64					m_startTime;
		U64					m_mainThreadId;

		static bool m_sIsDead;

		mutable MT::Mutex m_fsStatsMutex;
	};
}

#endif
