#pragma once


// CFixedWidthToolbar

class CFixedWidthToolbar : public CExtControlBar
{
public:
	CFixedWidthToolbar();

	DECLARE_DYNAMIC(CFixedWidthToolbar)

// Attributes
public:


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFixedWidthToolbar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFixedWidthToolbar();

protected:

	virtual CSize CFixedWidthToolbar::CalcFixedLayout(BOOL bStretch, BOOL bHorz);



// Generated message map functions
protected:
	//{{AFX_MSG(CFixedWidthToolbar)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
//	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
}; // class CFixedWidthToolbar
