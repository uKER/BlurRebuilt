#ifndef MULTI_PROGRESS_NOTIFICATION_H
#define MULTI_PROGRESS_NOTIFICATION_H

namespace LPVS
{
	// Interface for progress notifications.
	class MultiProgressNotification
	{
	public:
		void SetNumSubProcesses(U32 numSubProcesses)
		{
			m_subProcessProgress.Clear();
			m_subProcessProgress.Reserve(numSubProcesses);
			for (U32 i = 0; i < numSubProcesses; i++)
			{
				F32 zero = 0;
				m_subProcessProgress.PushBack(zero);
			}

			m_currentSubProcess = 0;
		}

		void SetSubProcess(U32 subProcess)
		{
			m_currentSubProcess = subProcess;
		}
		//Returning false will eventually abort the current operation. For the moment this is however
		//not implemented.
		virtual bool OnProgress(F32 percent) = 0;
		//Callback function for printing output to the user while they wait.
		virtual void OnPrintProgress(const std::string& text) = 0;
		virtual void OnPrintWarning(const std::string& text) = 0;
		virtual void OnPrintError(const std::string& text) = 0;

		U32 GetNumSubProcesses() const
		{
			return (U32)m_subProcessProgress.Size();
		}

		F32 GetSubProcessProgress(U32 process) const
		{
			return m_subProcessProgress[process];
		}

	protected:
		U32										m_currentSubProcess;
		Ut::UtVector< Ut::DynamicVector<F32> >	m_subProcessProgress;
	};
}

#endif
