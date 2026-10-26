#ifndef __MERCURY_CORE_CPUINFO_H__
#define __MERCURY_CORE_CPUINFO_H__

#ifdef _MSC_VER
#pragma once
#endif

//CPUInfo.h - helper class for getting cpu info, such as number of cpus, cache info etc.
//Probably only makes sense for PCs. We know the spec of consoles.

namespace MT
{
	class C_CpuInfo
	{
	public:

		C_CpuInfo();
		~C_CpuInfo();

		//Returns the number of logical cpus in the system, such as via hyperthreading. 
		//e.g.	a hyperthreaded P4 would have 1 physical and 2 logical cpus.
		//		a core2duo would have 2 physical and 2 logical cpus. 
		U32		GetNumLogicalCpus();
		
		//returns only the number of cpus which are currently active
		U32		GetNumActiveCpus();

		//simple helper, returns number of active logical cpus. 
		U32 GetNumCpus();

	private:

		U32 m_numLogicalCpus;
		U32 m_numActiveCpus;
	};
}//end namespace MT






#endif //__MERCURY_CORE_CPUINFO_H__
