#include "stdafx.h"

#include <lux_common/MultiProgressNotification.h>
#include <lux_common/LpvsAftkProgressNotification.h>

using namespace LPVS;

LpvsAftkProgressNotification::LpvsAftkProgressNotification(
	U32 subProcess,
	MultiProgressNotification* pLpvsFileProgress)
{
	m_subProcess = subProcess;
	m_pLpvsFileProgress = pLpvsFileProgress;
}

bool LpvsAftkProgressNotification::OnProgress(F32 percent)
{
	if (m_pLpvsFileProgress != 0)
	{
		m_pLpvsFileProgress->OnProgress(percent);
	}
	return false;
}

void LpvsAftkProgressNotification::OnPrintProgress(const std::string& text)
{
	if (m_pLpvsFileProgress != 0)
	{
		m_pLpvsFileProgress->OnPrintProgress(text);
	}
}

void LpvsAftkProgressNotification::OnPrintWarning(const std::string& text)
{
	if (m_pLpvsFileProgress != 0)
	{
		m_pLpvsFileProgress->OnPrintWarning(text);
	}
}

void LpvsAftkProgressNotification::OnPrintError(const std::string& text)
{
	if (m_pLpvsFileProgress != 0)
	{
		m_pLpvsFileProgress->OnPrintError(text);
	}
}
