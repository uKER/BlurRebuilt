#pragma once

#include "lux_app/CustomControls/DarkDialog.h"


// CPickerDlg dialog

class CPickerDlg : public CDarkDialog
{
	DECLARE_DYNAMIC(CPickerDlg)

public:
	CPickerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPickerDlg();

// Dialog Data
	enum { IDD = IDD_TESTDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// index of the selected polygon
	int m_iPolygon;
	// chart that is associated with the polygon
	int m_iChart;
	char *m_pszNodeName;
	CString m_csNodeName;
	CSliderCtrl m_slider1, m_slider2;
	int m_iSlider1, m_iSlider2;
	float m_fSlider1, m_fSlider2;

	void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	void CalculateLightDirSH();
};
