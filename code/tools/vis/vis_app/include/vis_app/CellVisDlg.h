#pragma once


// CCellVisDlg dialog

class CCellVisDlg : public CDialog
{
	DECLARE_DYNAMIC(CCellVisDlg)

public:
	CCellVisDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCellVisDlg();

// Dialog Data
	enum { IDD = IDD_CELLVISDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// Cell index number
	int m_iCell;
};
