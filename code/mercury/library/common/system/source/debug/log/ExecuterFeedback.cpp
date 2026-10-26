#include "SystemPch.h"

#include "System\DebugLog.h"
#include "System\Debug\Log\ExecuterFeedback.h"

namespace Debug
{
/// reference to executer logger
static Debug::Logger &g_ExecuterLog = Debug::GetLogger("executer");

/// constructor. specify the title of the task
ExecuterFeedback::ExecuterFeedback(const char *pszTitle) :
	I_ProgressBar(NULL)
{
	Util::C_String out, tmp;

	// send the string
	Set(0, pszTitle);
}

/// constructor. specify the title of the task and a progress bar to capture updates from
ExecuterFeedback::ExecuterFeedback(const char *pszTitle, Progress::I_ProgressBar *pChild ) :
	I_ProgressBar(NULL)
{
	Util::C_String out, tmp;

	// send the string
	Set(0, pszTitle);

	pChild->m_pParent = this;
}

/// set the title of a particular progress bar
void ExecuterFeedback::SetProgressBarTitle(int iBar, const char *pszTitle)
{
	Set(3+2*iBar, pszTitle);
}

/// set the percentage completed for a particular progress bar (floating point value from 0 to 1)
void ExecuterFeedback::SetProgressBarValue(int iBar, float fComplete)
{
	assert(fComplete <= 1.f);
	//Clamp to prevent executer assertions.
	if (fComplete > 1.f)
		fComplete = 1.f;
	if (fComplete < 0)
		fComplete = 0;

	Util::C_String tmp;
	tmp.Printf("%d", (int)(fComplete*100));
	Set(4+2*iBar, tmp);
}

/// set the feedback text box contents
void ExecuterFeedback::SetFeedbackText(const char *pszText)
{
	Set(1, pszText);
}

/// append text to the contents of the feedback text box
void ExecuterFeedback::AppendFeedbackText(const char *pszText)
{
	LOGINFO( g_ExecuterLog, "APPEND 1 %s", pszText);
}

/// receive progress notification from child
bool ExecuterFeedback::ProgressNotify( int iHierarchyDepth, const F32 percent, const Util::C_String &description )
{
	if (iHierarchyDepth<2)
	{
		SetProgressBarValue(iHierarchyDepth, percent/100.f);
		SetProgressBarTitle(iHierarchyDepth, description.Chars());
	}
	return true;
}

/// send progress command to the farm. checks for changes and doesn't send if hasn't changed
void ExecuterFeedback::Set(int i, const Util::C_String &text)
{
	if (i>=m_Vars.Size())
		m_Vars.Resize(i+1);

	if (m_Vars[i] != text)
	{
		m_Vars[i] = text;
		LOGINFO( g_ExecuterLog, "SET %d %s", i, text.Chars());
	}
}

} // namespace Debug
