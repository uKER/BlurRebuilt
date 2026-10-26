// XMLEditView.cpp : implementation file
//

#include "stdafx.h"
#include <TOM.h>
#include "UIMaker2.h"
#include "XMLEditView.h"

#include "ui.h"
#include "CommandLine.h"
#include "AddTextDlg.h"
#include "TextManager.h"
#include ".\xmleditview.h"


// CXMLEditView

IMPLEMENT_DYNCREATE(CXMLEditView, CRichEditView)

CXMLEditView::CXMLEditView()
{
	AfxInitRichEdit2();

	changed=false;
}

CXMLEditView::~CXMLEditView()
{
}

BEGIN_MESSAGE_MAP(CXMLEditView, CRichEditView)
	ON_WM_KEYDOWN()
	ON_CONTROL_REFLECT(EN_CHANGE, OnEnChange)
	ON_WM_RBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_COMMAND(ID_ADD_TEXT_KEY, OnAddTextKey)
END_MESSAGE_MAP()


// CXMLEditView diagnostics

#ifdef _DEBUG
void CXMLEditView::AssertValid() const
{
	CRichEditView::AssertValid();
}

void CXMLEditView::Dump(CDumpContext& dc) const
{
	CRichEditView::Dump(dc);
}
#endif //_DEBUG


// CXMLEditView message handlers

#define DEFINE_GUIDXXX(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID CDECL name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

DEFINE_GUIDXXX(IID_ITextDocument,0x8CC497C0,0xA1DF,0x11CE,0x80,0x98,0x00,0xAA,0x00,0x47,0xBE,0x5D);

void CXMLEditView::OnEnChange()
{

}

void CXMLEditView::OnInitialUpdate()
{
	CRichEditView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class

	long mask = CRichEditView::GetRichEditCtrl().GetEventMask();
	CRichEditView::GetRichEditCtrl().SetEventMask(mask |= ENM_CHANGE );
}


void CXMLEditView::OnRButtonDown(UINT nFlags, CPoint point)
{

}


void CXMLEditView::OnMButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	
	//CRichEditView::OnMButtonDown(nFlags, point);
}

void CXMLEditView::OnKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags)
{	

	CRichEditView::OnKeyDown(nChar,nRepCnt,nFlags);
}

void CXMLEditView::OnAddTextKey()
{

}
