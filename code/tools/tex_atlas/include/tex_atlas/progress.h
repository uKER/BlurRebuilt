#ifndef CS_GEN_PROGRESS_H
#define CS_GEN_PROGRESS_H

namespace TexAtlas
{

typedef Tu::Status(*TaskFunc)(void *pThis, const Ut::UtVector<Ut::DynamicVector< std::string> >& arguments);

class Progress : public Aftk::I_Progress
{
public:
	Progress();

	virtual ~Progress();

	virtual Tu::Status ExecuteTask(TaskFunc) = 0;

	void SetTaskResult(Tu::Status result);

	virtual void SetTitle(const std::string& text) = 0;

	//Returning false will eventually abort the current operation. For the moment this is however
	//not implemented.
	virtual bool OnProgress(F32 percent) = 0;

	void SetOffset(F32 percent);

	void SetScale(F32 percent);

	//Callback function for printing output to the user while they wait.
	virtual void OnPrintProgress(const std::string& text) = 0;

	bool Cancelled() const;

	Tu::Status TaskResult() const;

protected:
	static unsigned long __stdcall WorkThreadStartFunc(void* pThis);

	virtual void OnPreExecuteTask() = 0;

	virtual void OnPostExecuteTask() = 0;

	void SetTaskCallbackFunc(TaskFunc);
	
	void SetTaskArguments(const Ut::UtVector<Ut::DynamicVector< std::string> >& arguments);

	void StartThread();

	void WaitForThreadToFinish();

	bool				m_cancelled;
	F32					m_progress;
	void*				m_hThread;
	F32					m_scale;
	F32					m_offset;
	Tu::Status			(*m_pTaskCallbackFunc)(void *pThis, const Ut::UtVector<Ut::DynamicVector< std::string> >& arguments);
	Ut::UtVector<Ut::DynamicVector< std::string> >			m_taskArguments;
	static Tu::Status	m_taskResult;
};
}
#endif
