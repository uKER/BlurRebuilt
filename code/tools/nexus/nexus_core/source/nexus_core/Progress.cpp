#include "stdafx.h"

namespace Nexus
{

Tu::Status Progress::m_taskResult = Tu::Status::FAILURE;

Progress::Progress()
{
	m_taskResult = Tu::Status::SUCCESS;
	m_pTaskCallbackFunc = 0;
	m_started = false;
}

Progress::~Progress()
{
}

void Progress::Begin()
{
	m_cancelled			= false;
	m_progress			= 0;
	m_pTaskCallbackFunc = 0;
	m_taskResult = Tu::Status::SUCCESS;
	m_started = true;
}

void Progress::End()
{
	m_progress = 100.f;
	m_started = false;
}

void Progress::StartThread()
{
	m_cancelled			= false;
	m_progress			= 0;

	m_hThread = CreateThread(0, 0, WorkThreadStartFunc, this, 0, 0);
}

void Progress::WaitForThreadToFinish()
{
	WaitForSingleObject(m_hThread, INFINITE);
}

void Progress::SetTaskCallbackFunc(Tu::Status (*pTaskCallbackFunc)(void *pThis, const StringArray& arguments))
{
	m_pTaskCallbackFunc = pTaskCallbackFunc;
}

void Progress::SetTaskArguments(const StringArray& arguments)
{
	m_taskArguments = arguments;
}

void Progress::SetTaskResult(Tu::Status result)
{
	m_taskResult = result;
}

bool Progress::Cancelled() const
{
	return m_cancelled;
}

Tu::Status Progress::TaskResult() const
{
	return m_taskResult;
}

DWORD CALLBACK Progress::WorkThreadStartFunc(void* pThis)
{
	Progress* pThisProgress = (Progress*)pThis;
	pThisProgress->m_taskResult = pThisProgress->m_pTaskCallbackFunc(pThis, pThisProgress->m_taskArguments);

	return 0;
}

}