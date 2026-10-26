#ifndef LOGICALFS_PAK_MANAGER_H
#define LOGICALFS_PAK_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Utility/Singleton.h>
#include <Core/Threads/Thread.h>
#include <Btl/Containers/List.h>
#include "PakFs.h"

namespace LogicalFs
{	
	class C_PakFsManager : public Ut::Singleton<C_PakFsManager, Ut::CreateStatic>
	{
	public:
		typedef Btl::List<C_PakFs*> PakFsList;

		C_PakFsManager();
		virtual ~C_PakFsManager();

		bool Create();

		const PakFsList& GetPakFsList() const;

		void ReopenPaks();

		void ReopenStreamingPaks();

		void Suspend();

		void Resume();
		
		void StopReads();

	private:
		void OnPakFsCreated(C_PakFs* pPakFs);
		void OnPakFsDestroy(C_PakFs* pPakFs);

		PakFsList m_pakFsList;

		friend C_PakFs::C_PakFs();
		friend C_PakFs::~C_PakFs();
};
}

#endif
