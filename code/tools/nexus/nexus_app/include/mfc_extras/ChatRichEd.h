/*
 *	$Header: $
 *
 *	$History: $
 */
#if !defined(AFX_RICHED_H__F97C5D2C_3A7E_11D3_A8FE_00403398CC79__INCLUDED_)
#define AFX_RICHED_H__F97C5D2C_3A7E_11D3_A8FE_00403398CC79__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class _AFX_RICHEDITEX_STATE
{
public:
					_AFX_RICHEDITEX_STATE();
    virtual			~_AFX_RICHEDITEX_STATE();

    HINSTANCE		m_hInstRichEdit20 ;
};

BOOL PASCAL AfxInitRichEditEx();

/////////////////////////////////////////////////////////////////////////////
// CRichEd window
class CChatRichEd : public CRichEditCtrl
{
	DECLARE_DYNAMIC(CChatRichEd);
// Construction
public:
					CChatRichEd();
	virtual			~CChatRichEd();

// Attributes
public:
	CHARFORMAT&		CharFormat()	{ return m_cfDefault; }

// Operations
public:
	BOOL			Create(DWORD dwStyle,  const RECT& rcRect, CWnd* pParentWnd, UINT nID);
	void			AppendText(LPCTSTR szText);
	BOOL			SaveToFile(CFile *pFile);
	void			Freeze();
	void			Thaw();
	void			Clear();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRichEd)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	void			InternalAppendText(LPCTSTR szText);
	static DWORD CALLBACK	StreamCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);

	int				m_iLineCount,
					m_iLastLineCount;
	CStringList		m_cslDeferredText;
	BOOL			m_bFrozen;
	CHARFORMAT		m_cfDefault;

	//{{AFX_MSG(CRichEd)
	afx_msg void	OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg BOOL	OnLink(NMHDR *in_pNotifyHeader, LRESULT *out_pResult);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RICHED_H__F97C5D2C_3A7E_11D3_A8FE_00403398CC79__INCLUDED_)
