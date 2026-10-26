#include "stdafx.h"

#define NEXUS_PROGRESS_VERBOSE

namespace NexusCmd
{
ConsoleProgress::ConsoleProgress()
:
Progress()
{
}

ConsoleProgress::~ConsoleProgress()
{
}

Tu::Status ConsoleProgress::ExecuteTask(
	Tu::Status (*pTaskCallbackFunc)(void *pThis, const Nexus::StringArray& arguments),
	const Nexus::StringArray& arguments)
{
	m_lastProgress = 0;
	m_progressSinceLastUpdate = 0;
	Tu::Status status;
	OnPreExecuteTask();
	pTaskCallbackFunc(0, arguments);
//	SetTaskCallbackFunc(pTaskCallbackFunc);
//	SetTaskArguments(arguments);
//	StartThread();
//	WaitForThreadToFinish();
	OnPostExecuteTask();
	return TaskResult();
}

void ConsoleProgress::SetTitle(const std::string& text)
{
	std::cout << text << "\n" << std::flush;
}

bool ConsoleProgress::OnProgress(F32 percent)
{
	if (percent == 0)
	{
		m_progressSinceLastUpdate = 0;
		m_lastProgress = 0;
	}
	static const F32 cutoff = 0.1f;
	if (!(fabs(percent - 100.f) < 1e-2) && (percent < (100.f-cutoff)))
	{
		m_progressSinceLastUpdate += percent - m_lastProgress;
		if (m_progressSinceLastUpdate < cutoff)
		{
			m_lastProgress = percent;
			return false;
		}
	}

#ifndef NEXUS_PROGRESS_VERBOSE
	printf("\r%3.2f%%", percent);
#else
	printf("\r%3.2f%% %s", percent, m_text.c_str());
	if (m_text.length() < 70)
	{
		for (U32 i = (U32)m_text.length(); i < 70; i++)
		{
			printf(" ");
		}
	}
#endif
	if (percent == 100.f)
	{
		m_progressSinceLastUpdate = 0;
		m_lastProgress = 0;
	}
	else
	{
		m_lastProgress = percent;
		m_progressSinceLastUpdate = 0;
	}
	return false;
}

void ConsoleProgress::OnPrintProgress(const std::string& text)
{
#ifdef NEXUS_PROGRESS_VERBOSE
	m_text = text;
	if (m_text.length() > 70)
	{
		m_text = m_text.substr(0, 70);
	}
#endif
}

void ConsoleProgress::OnPreExecuteTask()
{
	std::cout << std::endl;
}

void ConsoleProgress::OnPostExecuteTask()
{
}

}