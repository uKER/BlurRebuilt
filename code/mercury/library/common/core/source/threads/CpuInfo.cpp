#include "CorePch.h"
#include <core/core.h>
#include <core/threads/CpuInfo.h>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#elif defined(PLATFORM_LINUX)
#include <unistd.h>
#endif

namespace MT
{
	C_CpuInfo::C_CpuInfo() : m_numLogicalCpus(1), m_numActiveCpus(1)
	{
#ifdef PLATFORM_WINDOWS

		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);

		m_numLogicalCpus = sysInfo.dwNumberOfProcessors;
		U32 cpuActiveMask = (U32)sysInfo.dwActiveProcessorMask;

		m_numActiveCpus=0;

		for (U32 i=0; i<32; i++)
		{
			if (cpuActiveMask & (1 << i))
			{
				m_numActiveCpus++;
			}
		}

#elif defined (PLATFORM_LINUX)
		m_numLogicalCpus = sysconf(_SC_NPROCESSORS_CONF);
		m_numActiveCpus = sysconf(_SC_NPROCESSORS_ONLN);
#else
		m_numLogicalCpus = 1;
		m_numActiveCpus = 1;
#endif

		if (m_numLogicalCpus < 1) m_numLogicalCpus = 1;
		if (m_numActiveCpus < 1) m_numActiveCpus = 1;
	}

	C_CpuInfo::~C_CpuInfo()
	{
	}

	//returns  the total number of logical cpus 
	U32	C_CpuInfo::GetNumLogicalCpus()
	{
		return m_numLogicalCpus;
	}
	
	//returns only the number of cpus which are currently active
	U32	C_CpuInfo::GetNumActiveCpus()
	{
		return m_numActiveCpus;
	}

	//simple helper, returns number of active logical cpus. 
	U32 C_CpuInfo::GetNumCpus()
	{
		return m_numActiveCpus;
	}

}
