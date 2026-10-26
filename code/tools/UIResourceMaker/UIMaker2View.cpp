// UIMaker2View.cpp : implementation of the CUIMaker2View class
//

#include "stdafx.h"
#include "UIMaker2.h"

#include "UIMaker2Doc.h"
#include "UIMaker2View.h"

#include "CommandLine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUIMaker2View

IMPLEMENT_DYNCREATE(CUIMaker2View, CScrollView)

BEGIN_MESSAGE_MAP(CUIMaker2View, CScrollView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CUIMaker2View construction/destruction

CUIMaker2View::CUIMaker2View()
{
	// TODO: add construction code here

}

CUIMaker2View::~CUIMaker2View()
{
}

BOOL CUIMaker2View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	BOOL Ret = CScrollView::PreCreateWindow(cs);

	SetScrollSizes(MM_TEXT, CSize(0, 2000));

	return Ret;
}

// CUIMaker2View drawing

void CUIMaker2View::OnDraw(CDC* cdc)
{
	CUIMaker2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here

	gCommandLine.Render(cdc);

//	cdc->Rectangle(0,0,640,480);

	SetScrollSizes(MM_TEXT, CSize(0, gCommandLine.GetSize()));
}


// CUIMaker2View printing

BOOL CUIMaker2View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CUIMaker2View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CUIMaker2View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CUIMaker2View diagnostics

#ifdef _DEBUG
void CUIMaker2View::AssertValid() const
{
	CScrollView::AssertValid();
}

void CUIMaker2View::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CUIMaker2Doc* CUIMaker2View::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CUIMaker2Doc)));
	return (CUIMaker2Doc*)m_pDocument;
}
#endif //_DEBUG


// CUIMaker2View message handlers
