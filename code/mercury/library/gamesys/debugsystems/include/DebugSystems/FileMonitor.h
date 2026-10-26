#ifndef GAMESYS_FILE_MONITOR_H
#define GAMESYS_FILE_MONITOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/serialise/FsPerfLogger.h>

struct IoCacheStats;

namespace DebugSystems
{
	class C_FileMonitor
	{
	public:
		C_FileMonitor();
		~C_FileMonitor();

		void Initialise();

		void Render();

	private:
		Btl::Vector<Sys::C_FsReport> m_reports;
		IoCacheStats* m_pCacheStats;
	};
}

#endif
