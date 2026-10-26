#pragma once


class NexusProgress : public SRD::IProgress
{
public:
	NexusProgress();
	virtual ~NexusProgress();
	
	void SetParent(CWnd* pParent);
// Dialog Data
	void Show();

	void Hide();

	void SetTitle(const std::string& text);

	//Returning false will eventually abort the current operation. For the moment this is however
	//not implemented.
	bool OnProgress(F32 percent);

	//Callback function for printing output to the user while they wait.
	void OnPrintProgress(const std::string& text);

private:
	CWnd*						m_pParent;
	
};
