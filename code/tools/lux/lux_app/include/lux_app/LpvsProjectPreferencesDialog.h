#pragma once


// ProjectPreferencesDialog dialog

class LpvsProjectPreferencesDialog : public CDialog
{
	DECLARE_DYNAMIC(LpvsProjectPreferencesDialog)

public:
	LpvsProjectPreferencesDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~LpvsProjectPreferencesDialog();

// Dialog Data
	enum { IDD = IDD_PROJECT_PREFERENCES_DIALOG };

protected:
	void Reset();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	bool m_settingsChanged;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSetLightingModelFile();
	afx_msg void OnBnClickedOk();
	CString m_sceneFileName;
protected:
	CString m_lightingModelFileName;
public:
	afx_msg void OnBnClickedSetSceneFile();
	virtual BOOL OnInitDialog();
protected:
	CString m_sceneOutputFileName;
public:
	afx_msg void OnBnClickedSetSceneFileOutput();
};
