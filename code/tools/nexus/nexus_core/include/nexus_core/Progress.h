#ifndef NEXUS_CORE_PROGRESS_H
#define NEXUS_CORE_PROGRESS_H

namespace Nexus
{

class Progress : public Aftk::I_Progress
{
public:
	Progress();

	virtual ~Progress();

	virtual void Begin();

	virtual void End();

	virtual Tu::Status ExecuteTask(
		Tu::Status (*pTaskCallbackFunc)(void *pThis, const StringArray& arguments),
		const StringArray& arguments) = 0;

	void SetTaskResult(Tu::Status result);

	virtual void SetTitle(const std::string& text) = 0;

	//Returning false will eventually abort the current operation. For the moment this is however
	//not implemented.
	virtual bool OnProgress(F32 percent) = 0;

	//Callback function for printing output to the user while they wait.
	virtual void OnPrintProgress(const std::string& text) = 0;

	bool Cancelled() const;

	Tu::Status TaskResult() const;

protected:
	static DWORD CALLBACK WorkThreadStartFunc(void* pThis);

	virtual void OnPreExecuteTask() = 0;

	virtual void OnPostExecuteTask() = 0;

	void SetTaskCallbackFunc(Tu::Status (*pTaskCallbackFunc)(void *pThis, const StringArray& arguments));
	
	void SetTaskArguments(const StringArray& arguments);

	void StartThread();

	void WaitForThreadToFinish();

	bool				m_started;
	bool				m_cancelled;
	F32					m_progress;
	HANDLE				m_hThread;
	Tu::Status			(*m_pTaskCallbackFunc)(void *pThis, const StringArray& arguments);
	StringArray			m_taskArguments;
	static Tu::Status	m_taskResult;
};
}
#endif
