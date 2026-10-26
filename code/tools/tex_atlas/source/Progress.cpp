#include "stdafx.h"

using namespace TexAtlas;

Tu::Status Progress::m_taskResult = Tu::Status::FAILURE;

Progress::Progress()
{
	m_taskResult = Tu::Status::SUCCESS;
	m_pTaskCallbackFunc = 0;
	m_cancelled			= false;
	m_scale = 1.f;
	m_offset = 0;
}

Progress::~Progress()
{
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

void Progress::SetScale(F32 scale)
{
	m_scale = scale;
}

void Progress::SetOffset(F32 percent)
{
	m_offset = percent;
}

bool Progress::Cancelled() const
{
	return m_cancelled;
}

Tu::Status Progress::TaskResult() const
{
	return m_taskResult;
}

// main lightmapping thread entry point
unsigned long __stdcall Progress::WorkThreadStartFunc(void* pThis)
{
	Progress* pThisProgress = (Progress*)pThis;
	pThisProgress->m_taskResult = pThisProgress->m_pTaskCallbackFunc(pThis, pThisProgress->m_taskArguments);

	return 0;
}
