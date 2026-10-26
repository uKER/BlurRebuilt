#include "LogicalFsPch.h"
#include <System/Debug/Debug.h>

#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/PakFsManager.h>
#include <PakFs/Pak.h>

namespace LogicalFs
{

void VerifyPaksCb(void* pContext);
void VerifySpinCb(void* pContext);

Debug::Function g_verifyPaks("Util.LogicalFs.VerifyPaks", &LogicalFs::VerifyPaksCb, 0);
Debug::Function g_verifySpin("Util.LogicalFs.VerifySpin", &LogicalFs::VerifySpinCb, 0);
Debug::Bool g_enableSpin("Util.LogicalFs.SpinEnabled", true);

C_PakFsManager::C_PakFsManager()
{
}

C_PakFsManager::~C_PakFsManager()
{
	for (PakFsList::Iterator it = m_pakFsList.Begin(); it != m_pakFsList.End(); ++it)
	{
		delete *it;
	}
}

bool C_PakFsManager::Create()
{
	return true;
}

const C_PakFsManager::PakFsList& C_PakFsManager::GetPakFsList() const
{
	return m_pakFsList;
}

void C_PakFsManager::ReopenPaks()
{
	for (PakFsList::Iterator it = m_pakFsList.Begin(); it != m_pakFsList.End(); ++it)
	{
		LogicalFs::C_PakFs* pPakFs = *it;
		pPakFs->ReopenPakFile();
	}
}

void C_PakFsManager::ReopenStreamingPaks()
{
	//HACK!
	for (PakFsList::Iterator it = m_pakFsList.Begin(); it != m_pakFsList.End(); ++it)
	{
		LogicalFs::C_PakFs* pPakFs = *it;
		if (pPakFs->GetPakFilePath().Search("streaming") != -1)
			pPakFs->ReopenPakFile();
	}
}

void C_PakFsManager::Suspend()
{
	for (PakFsList::Iterator it = m_pakFsList.Begin(); it != m_pakFsList.End(); ++it)
	{
		LogicalFs::C_PakFs* pPakFs = *it;
		pPakFs->Suspend();
	}
}

void C_PakFsManager::Resume()
{
	for (PakFsList::Iterator it = m_pakFsList.Begin(); it != m_pakFsList.End(); ++it)
	{
		LogicalFs::C_PakFs* pPakFs = *it;
		pPakFs->Resume();
	}
}

void C_PakFsManager::StopReads()
{
	for (PakFsList::Iterator it = m_pakFsList.Begin(); it != m_pakFsList.End(); ++it)
	{
		LogicalFs::C_PakFs* pPakFs = *it;
		pPakFs->StopReads();
	}
}

void C_PakFsManager::OnPakFsCreated(C_PakFs* pPakFs)
{
	m_pakFsList.PushBack(pPakFs);
}

void C_PakFsManager::OnPakFsDestroy(C_PakFs* pPakFs)
{
	for (PakFsList::Iterator it = m_pakFsList.Begin(); it != m_pakFsList.End(); ++it)
	{
		if (*it == pPakFs)
		{
			m_pakFsList.Erase(it);
			break;
		}
	}
}

class SpinThread : public MT::Thread
{
public:
	SpinThread(U32 hwThread, U32 priority) : MT::Thread(), m_hwThread(hwThread), m_priority(priority) {}

	~SpinThread() { delete this; }

	virtual void ThreadMain()
	{

		S32 spinCount(0);
		while(g_enableSpin)
		{
			DebugLn("Spinning %i", spinCount);
			++spinCount;
		}
		DebugLn("Stopping spinning.");
	}

	U32 m_hwThread;
	U32 m_priority;
};

class VerifyThread : public MT::Thread
{
public:
	VerifyThread(U32 hwThread, U32 priority) : MT::Thread(), m_hwThread(hwThread), m_priority(priority) {}

	~VerifyThread() { delete this; }

	virtual void ThreadMain()
	{

		while(g_enableSpin)
		{
			const C_PakFsManager::PakFsList& pakList = C_PakFsManager::Instance().GetPakFsList();
			for (C_PakFsManager::PakFsList::ConstIterator it(pakList.Begin()); it != pakList.End(); ++it)
			{
				C_PakFs* pPakFs = *it;
				Pak::Verify2(*pPakFs->GetPak());
			}
		}
	}

	U32 m_hwThread;
	U32 m_priority;
};

void VerifyPaksCb(void* pContext)
{
	VerifyThread* pVt = new VerifyThread(0, 0);
	pVt->Start();
}

void VerifySpinCb(void* pContext)
{
	SpinThread* pVt = new SpinThread(0, 0);
	pVt->Start();
}

}
