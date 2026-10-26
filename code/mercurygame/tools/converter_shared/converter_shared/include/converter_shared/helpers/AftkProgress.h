#ifndef CONVERTER_SHARED_AFTK_PROGRESS_H
#define CONVERTER_SHARED_AFTK_PROGRESS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/misc/Progress.h>

class C_AftkProgress : public Aftk::I_Progress
{
public:
	C_AftkProgress(Progress::I_ProgressBar* pProgressBar) : m_ProgressBar(pProgressBar, "AftkProgress")
	{
	}

	virtual ~C_AftkProgress()
	{
	}

	// Inherited from AftkProgressNotification
	virtual bool OnProgress(F32 percent)
	{
		//m_pProgressBar->ProgressNotify( 0, percent, "AftkProgress" );
		m_ProgressBar.SetProgress(percent);
		return false;
	}

	//Callback function for printing output to the user while they wait.
	virtual void OnPrintProgress(const std::string& text) 
	{
		LOGDEBUG( Tools::g_log, "%s\n", text.c_str());
	}

	virtual void OnPrintWarning(const std::string& text)
	{
		LOGWARN( Tools::g_log, "%s\n", text.c_str());
	}

	virtual void OnPrintError(const std::string& text)
	{
		LOGFATAL( Tools::g_log, "%s\n", text.c_str());
	}

private:
	Progress::UpdateSimple			m_ProgressBar;
};

#endif
