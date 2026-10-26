#ifndef NEXUS_CMD_PROGRESS_H
#define NEXUS_CMD_PROGRESS_H

namespace NexusCmd
{
	class ConsoleProgress : public Nexus::Progress
	{
	public:
		ConsoleProgress();

		Tu::Status ConsoleProgress::ExecuteTask(
			Tu::Status (*pTaskCallbackFunc)(void *pThis, const Nexus::StringArray& arguments),
			const Nexus::StringArray& arguments);

		~ConsoleProgress();

		void SetTitle(const std::string& text);
	    
	protected:
		bool OnProgress(F32 percent);

		virtual void OnPreExecuteTask();

		virtual void OnPostExecuteTask();

		void OnPrintProgress(const std::string& text);

		std::string m_text;
		F32 m_lastProgress;
		F32 m_progressSinceLastUpdate;
	};
}
#endif
