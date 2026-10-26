#ifndef __SYSTEM_LOG_EXECUTER_FEEDBACK_H__
#define __SYSTEM_LOG_EXECUTER_FEEDBACK_H__

#include <system/misc/progress.h>

namespace Debug
{

/// helper class that sends progress messages to executer via the debug logging system
class ExecuterFeedback : public Progress::I_ProgressBar
{
public:
				/// constructor. specify the title of the task and the number of progress bars
				ExecuterFeedback( const char *pszTitle);
				/// construct with a parent progress bar
				ExecuterFeedback( const char *pTitle, I_ProgressBar *m_pChild );

				/// set the title of a particular progress bar
	void		SetProgressBarTitle(int iBar, const char *pszTitle);
				/// set the percentage completed for a particular progress bar (floating point value from 0 to 1)
	void		SetProgressBarValue(int iBar, float fComplete);
				/// set the feedback text box contents
	void		SetFeedbackText(const char *pszText);
				/// append text to the contents of the feedback text box
	void		AppendFeedbackText(const char *pszText);

				/// receive progress notification from child
	bool		ProgressNotify( int i, const F32 percent, const Util::C_String &description );

private:
				/// send a progress update command to the farm
	void		Set(int i, const Util::C_String &text);

	Ut::Vector<Util::C_String> m_Vars;
};

}

#endif
