#pragma once

// create a dialog with a different background colour

class CDarkDialog : public CDialog
{
public:
	CDarkDialog( UINT i, CWnd* pParentWnd = NULL) :
	  CDialog(i, pParentWnd)
	  {
		   m_HollowBrush.CreateStockObject(HOLLOW_BRUSH);
	  };

	 CDarkDialog() : CDialog() {};

//    BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM Param,
  //      LRESULT* pLResult);
protected:
    static CBrush m_brush;
    static CBrush m_darkbrush;
	static CBrush m_HollowBrush;
public:
	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

