// LpvsRenderView.cpp : implementation file
//

#include "stdafx.h"
#include <lux_app/LpvsApp.h>
#include <lux_app/LpvsFileProgressDialog.h>
#include <lux_app/LpvsRenderView.h>
#include <lux_app/LpvsMainFrame.h>
#include "..\..\include\lux_app\lpvsrenderview.h"
#include "lux_app/AppSettings.h"

// LpvsRenderView

using namespace LPVS;

IMPLEMENT_DYNCREATE(LpvsRenderView, CView)

LpvsRenderView::LpvsRenderView()
{
	m_initialPaint = true;
	m_bInitialized = false;
}

LpvsRenderView::~LpvsRenderView()
{
}

BEGIN_MESSAGE_MAP(LpvsRenderView, CView)
	ON_WM_PAINT()
	ON_WM_SIZE()
//	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYUP()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// LpvsRenderView diagnostics

#ifdef _DEBUG
void LpvsRenderView::AssertValid() const
{
	CView::AssertValid();
}

void LpvsRenderView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

void LpvsRenderView::OnInitialUpdate()
{
	LPVS::LPVSCORE.Create(m_hWnd);
	m_bInitialized = true;

	LPVSCORE.GetSceneRenderer()->SetSettings(gpMainFrm->m_appSettings.m_RendererSettings);
}

// LpvsRenderView message handlers

void LpvsRenderView::OnPaint()
{
	if (!m_bInitialized) return;

	//CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CView::OnPaint() for painting messages

	if (LPVS::LPVSCORE.GetState() == LPVS::APP_MAIN_STATE_IDLE)
	{
		//Painting is permitted.
		RECT r;
		this->GetClientRect(&r);
		if (m_initialPaint == true)
		{
			LPVS::LPVSCORE.Resize(r.right - r.left, r.bottom - r.top);
			m_initialPaint = false;
		}

		LPVS::LPVSCORE.SetViewportRectangle(r);
		LPVS::LPVSCORE.RenderScene();	
	}

	CView::OnPaint();
}

void LpvsRenderView::OnSize(UINT nType, int cx, int cy)
{
	if (!m_bInitialized) return;
	if (LPVS::LpvsCore::m_bShuttingDown) return;

	CView::OnSize(nType, cx, cy);
	LPVS::LPVSCORE.Resize(cx, cy); // TODO: fix - not have singleton be reallocted when we're shutting down!

	// TODO: Add your message handler code here
}

//void LpvsRenderView::OnTimer(UINT nIDEvent)
//{
//	// TODO: Add your message handler code here and/or call default
//
//	CView::OnTimer(nIDEvent);
//}

void LpvsRenderView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	LpvsMainFrame* pLpvsMainFrame = (LpvsMainFrame*)GetParentFrame();
	pLpvsMainFrame->KeyboardEvent(nChar, nRepCnt, nFlags);

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void LpvsRenderView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	RECT r;
	this->GetClientRect(&r);
	LPVS::LPVSCORE.SetViewportRectangle(r);
	LPVS::LPVSCORE.RenderScene();

	CView::OnUpdate(pSender, lHint, pHint);
}

void LpvsRenderView::OnActivateFrame(UINT nState, CFrameWnd* pDeactivateFrame)
{
	CView::OnActivateFrame(nState, pDeactivateFrame);
}

void LpvsRenderView::OnMouseMove(UINT nFlags, CPoint point)
{
	LpvsMainFrame* pLpvsMainFrame = (LpvsMainFrame*)GetParentFrame();
	pLpvsMainFrame->MouseMoveEvent(nFlags, point);

	CView::OnMouseMove(nFlags, point);
}

void LpvsRenderView::OnRButtonDown(UINT nFlags, CPoint point)
{
	LpvsMainFrame* pLpvsMainFrame = (LpvsMainFrame*)GetParentFrame();
	pLpvsMainFrame->MouseButtonEvent(LPVS_MOUSE_BUTTON_RIGHT, LPVS_MOUSE_BUTTON_ACTION_DOWN, nFlags, point);

	CView::OnRButtonDown(nFlags, point);
}

void LpvsRenderView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	LpvsMainFrame* pLpvsMainFrame = (LpvsMainFrame*)GetParentFrame();
	pLpvsMainFrame->KeyboardEvent(nChar, nRepCnt, nFlags);

	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}

void LpvsRenderView::OnRButtonUp(UINT nFlags, CPoint point)
{
	LpvsMainFrame* pLpvsMainFrame = (LpvsMainFrame*)GetParentFrame();
	pLpvsMainFrame->MouseButtonEvent(LPVS_MOUSE_BUTTON_RIGHT, LPVS_MOUSE_BUTTON_ACTION_UP, nFlags, point);

	CView::OnRButtonUp(nFlags, point);
}

void LpvsRenderView::OnLButtonDown(UINT nFlags, CPoint point)
{
	LpvsMainFrame* pLpvsMainFrame = (LpvsMainFrame*)GetParentFrame();
	pLpvsMainFrame->MouseButtonEvent(LPVS_MOUSE_BUTTON_LEFT, LPVS_MOUSE_BUTTON_ACTION_DOWN, nFlags, point);

	CView::OnLButtonDown(nFlags, point);
}

void LpvsRenderView::OnLButtonUp(UINT nFlags, CPoint point)
{
	LpvsMainFrame* pLpvsMainFrame = (LpvsMainFrame*)GetParentFrame();
	pLpvsMainFrame->MouseButtonEvent(LPVS_MOUSE_BUTTON_LEFT, LPVS_MOUSE_BUTTON_ACTION_UP, nFlags, point);

	CView::OnLButtonUp(nFlags, point);
}

void LpvsRenderView::OnMButtonDown(UINT nFlags, CPoint point)
{
	LpvsMainFrame* pLpvsMainFrame = (LpvsMainFrame*)GetParentFrame();
	pLpvsMainFrame->MouseButtonEvent(LPVS_MOUSE_BUTTON_MIDDLE, LPVS_MOUSE_BUTTON_ACTION_DOWN, nFlags, point);

	CView::OnMButtonDown(nFlags, point);
}

void LpvsRenderView::OnMButtonUp(UINT nFlags, CPoint point)
{
	LpvsMainFrame* pLpvsMainFrame = (LpvsMainFrame*)GetParentFrame();
	pLpvsMainFrame->MouseButtonEvent(LPVS_MOUSE_BUTTON_MIDDLE, LPVS_MOUSE_BUTTON_ACTION_UP, nFlags, point);

	CView::OnMButtonUp(nFlags, point);
}

void LpvsRenderView::OnDraw(CDC* pDC)
{
	LPVS::LPVSCORE.RenderScene();
}

BOOL LpvsRenderView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	//return CView::OnEraseBkgnd(pDC);
	return true;
}
