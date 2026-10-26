#ifndef LPVS_AFTK_PROGRESS_NOTIFICATION_H
#define LPVS_AFTK_PROGRESS_NOTIFICATION_H

namespace LPVS
{
	class LpvsAftkProgressNotification : public Aftk::I_Progress
	{
	public:
		LpvsAftkProgressNotification(U32 subProcess, MultiProgressNotification* pLpvsFileProgress);

	protected:
		bool OnProgress(F32 percent);

		void OnPrintProgress(const std::string& text);
		void OnPrintWarning(const std::string& text);
		void OnPrintError(const std::string& text);

		U32							m_subProcess;
		MultiProgressNotification*	m_pLpvsFileProgress;
	};
}

#endif
