#if !defined(AFX_PROGRESSTIMETOCOMPLETE_H__770907FF_A66F_11D4_85BA_00A0CC5F2805__INCLUDED_)
#define AFX_PROGRESSTIMETOCOMPLETE_H__770907FF_A66F_11D4_85BA_00A0CC5F2805__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class TProgressTimeToComplete : public CProgressCtrl
{
  private:
    double  m_lfPercent;
    clock_t m_start;
    CFont   m_fountHorz;
    CFont   m_fountVert;

  protected:
    //{{AFX_VIRTUAL(TProgressTimeToComplete)
    //}}AFX_VIRTUAL
    //{{AFX_MSG(TProgressTimeToComplete)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

  protected:
    virtual CString GetRemainingText(double lfPercent, double lfSecsRemaining);

  public:
    TProgressTimeToComplete();
    virtual ~TProgressTimeToComplete();
    void ResetStartTime(void);

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESSTIMETOCOMPLETE_H__770907FF_A66F_11D4_85BA_00A0CC5F2805__INCLUDED_)
