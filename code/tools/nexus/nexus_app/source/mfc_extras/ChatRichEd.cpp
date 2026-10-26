
#include <nexus_app/stdafx.h>
#include <mfc_extras/ChatRichEd.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

_AFX_RICHEDITEX_STATE::_AFX_RICHEDITEX_STATE()
{
    m_hInstRichEdit20 = NULL ;
}

_AFX_RICHEDITEX_STATE::~_AFX_RICHEDITEX_STATE()
{
    if (m_hInstRichEdit20 != NULL)
        ::FreeLibrary(m_hInstRichEdit20) ;
}

_AFX_RICHEDITEX_STATE _afxRichEditStateEx;

BOOL PASCAL AfxInitRichEditEx()
{
    _AFX_RICHEDITEX_STATE *l_pState = &_afxRichEditStateEx;
    
    if (l_pState->m_hInstRichEdit20 == NULL)
        l_pState->m_hInstRichEdit20 = LoadLibraryA(_T("RICHED20.DLL"));
    
    return l_pState->m_hInstRichEdit20 != NULL ;
}

/////////////////////////////////////////////////////////////////////////////
// CChatRichEd
IMPLEMENT_DYNAMIC(CChatRichEd, CRichEditCtrl);

CChatRichEd::CChatRichEd() : CRichEditCtrl()
{
	m_iLineCount = m_iLastLineCount = 0;
	m_bFrozen = FALSE;

	memset(&m_cfDefault, 0, sizeof(m_cfDefault));
	m_cfDefault.cbSize = sizeof(CHARFORMAT);
	m_cfDefault.dwMask = CFM_BOLD | CFM_COLOR | CFM_FACE | CFM_ITALIC | CFM_SIZE | CFM_STRIKEOUT | CFM_UNDERLINE;
	m_cfDefault.crTextColor = RGB(0, 0, 0);
	_tcscpy(m_cfDefault.szFaceName, _T("MS Sans Serif"));
	m_cfDefault.bPitchAndFamily = FF_SWISS;
	m_cfDefault.yHeight = -12;
}

CChatRichEd::~CChatRichEd()
{
}

BOOL CChatRichEd::Create(DWORD dwStyle,  const RECT& rcRect, CWnd* pParentWnd, UINT nID)
{
    if (!::AfxInitRichEditEx())
        return FALSE ;
    
    CWnd* l_pWnd = this ;
    return l_pWnd->Create(_T("RichEdit20A"), NULL, dwStyle, rcRect, pParentWnd, nID);
}

void CChatRichEd::Freeze()
{
	m_bFrozen = TRUE;
	SetBackgroundColor(FALSE, RGB(64, 64, 64));
}

void CChatRichEd::Thaw()
{
	CString text;

	while (!m_cslDeferredText.IsEmpty())
	{
		text = m_cslDeferredText.RemoveTail();
		InternalAppendText(text);
	}

	m_bFrozen = FALSE;
	SetBackgroundColor(TRUE, RGB(64, 64, 64));
}

void CChatRichEd::Clear()
{
	int iTotalTextLength = GetWindowTextLength();

	SetSel(0, iTotalTextLength);
	ReplaceSel(_T(""));
}

//	This is the public interface for appending text to the control.  It
//	either appends it directly to the control or adds it to a string list
//	if the control is frozen.  When the control is thawed the strings 
//	are taken off the list and added to the control.
void CChatRichEd::AppendText(LPCTSTR szText)
{
	if (m_bFrozen)
		m_cslDeferredText.AddHead(szText);
	else
		InternalAppendText(szText);
}

void CChatRichEd::InternalAppendText(LPCTSTR szText)
{
	int len;

	ASSERT(szText);
	ASSERT(AfxIsValidString(szText));

	int	 iTotalTextLength = GetWindowTextLength();
	CWnd *focusWnd = GetFocus();

	if (iTotalTextLength > 125000)
	{
		//	The control's starting to get full so trim off the first 
		//	50,000 bytes....
		SetSel(0, 50000);
		ReplaceSel(_T(""));
		SetSel(iTotalTextLength, iTotalTextLength);
	}

	//	Hide any selection and select the end of text marker.
	HideSelection(TRUE, TRUE);
	SetSel(iTotalTextLength, iTotalTextLength);

	//	Now set the character format
	SetSelectionCharFormat(m_cfDefault);
	//	And put the text into the selection
	ReplaceSel(szText);

	HideSelection(FALSE, TRUE);
	SendMessage(EM_SCROLLCARET, 0, 0);

	if (focusWnd != (CWnd *) NULL)
		focusWnd->SetFocus();
}

BOOL CChatRichEd::SaveToFile(CFile *pFile)
{
	ASSERT(pFile);
	ASSERT_KINDOF(CFile, pFile);

	EDITSTREAM es;

	#pragma warning(push)
	#pragma warning(disable: 4311)
	es.dwCookie = (DWORD) pFile;
	#pragma warning(pop)
	es.pfnCallback = StreamCallback;

	StreamOut(SF_RTF, es);
	return TRUE;
}

DWORD CALLBACK CChatRichEd::StreamCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	#pragma warning(push)
	#pragma warning(disable: 4312)
	CFile *pFile = (CFile *) dwCookie;
	#pragma warning(pop)

	ASSERT(pFile);
	ASSERT_KINDOF(CFile, pFile);

	ASSERT(pbBuff);
	ASSERT(AfxIsValidAddress(pbBuff, cb, FALSE));

	try
	{
		pFile->Write(pbBuff, cb);
	}
	catch(CFileException *fe)
	{
		fe->Delete();
	}

	*pcb = cb;
	return FALSE;
}

BEGIN_MESSAGE_MAP(CChatRichEd, CRichEditCtrl)
	//{{AFX_MSG_MAP(CChatRichEd)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT_EX(EN_LINK, OnLink)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChatRichEd message handlers
BOOL CChatRichEd::OnLink(NMHDR *pNotifyHeader, LRESULT *pResult)
{
	ENLINK	  *pENLink = (ENLINK *) pNotifyHeader;
	CString	  URL ;
	CHARRANGE CharRange ;

	*pResult = 0;

	switch (pNotifyHeader->code)
	{
	case EN_LINK:
		pENLink = (ENLINK *) pNotifyHeader;
	
		switch (pENLink->msg)
		{
		case WM_LBUTTONDOWN:
			GetSel(CharRange);
			SetSel(pENLink->chrg);
			URL = GetSelText();
			SetSel(CharRange);

			{
				CWaitCursor WaitCursor;

				ShellExecute(GetSafeHwnd(), _T("open"), URL, NULL, NULL, SW_SHOWNORMAL);
				*pResult = 1;
			}
			
			break;

		case WM_LBUTTONUP:
			*pResult = 1;
			break ;
		}
		
		break;
	}
	
	return TRUE;
}

//	We need to handle the OnSize message to ensure that the most recent
//	line of text is visible.
void CChatRichEd::OnSize(UINT nType, int cx, int cy) 
{
	CRichEditCtrl::OnSize(nType, cx, cy);

	if (GetSafeHwnd() != (HWND) NULL)
	{
		CDC		   *dc = GetDC();
		TEXTMETRIC tm;
		int		   topLine = GetFirstVisibleLine(),
				   bottomLine,
				   textLength = GetTextLength();
		CHARRANGE  oldSel;

		GetSel(oldSel);
		SetSel(textLength, textLength);
		dc->GetTextMetrics(&tm);
		ReleaseDC(dc);
		m_iLineCount = cy / tm.tmAscent;

		if (m_iLastLineCount != m_iLineCount)
		{
			m_iLastLineCount = m_iLineCount;
			bottomLine = LineFromChar(-1);

			if (GetLineCount() > m_iLineCount && topLine != bottomLine - m_iLineCount - 1)
				LineScroll(topLine - bottomLine - m_iLineCount - 1);
		}

		SetSel(oldSel);
		SendMessage(EM_SCROLLCARET, 0, 0);
	}
}
