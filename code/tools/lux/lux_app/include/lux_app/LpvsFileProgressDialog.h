#pragma once


// LpvsFileProgress dialog

class LpvsFileProgressDialog : public CDialog, LPVS::MultiProgressNotification
{
	DECLARE_DYNAMIC(LpvsFileProgressDialog)

public:
	LpvsFileProgressDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~LpvsFileProgressDialog();
	
// Dialog Data
	enum { IDD = IDD_FILE_PROGRESS };

	//Returning false will eventually abort the current operation. For the moment this is however
	//not implemented.
	virtual bool OnProgress(F32 percent);
	//Callback function for printing output to the user while they wait.
	virtual void OnPrintProgress(const std::string& text);
	virtual void OnPrintWarning(const std::string& text);
	virtual void OnPrintError(const std::string& text);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};
