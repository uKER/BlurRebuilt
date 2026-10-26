#pragma once


// LpvsRenderView view

class LpvsRenderView : public CView
{
	DECLARE_DYNCREATE(LpvsRenderView)

protected:
	LpvsRenderView();           // protected constructor used by dynamic creation
	virtual ~LpvsRenderView();
	bool		Render2D();
	bool		Render3D();

	//Is this the first time we've handled a paint message?
	bool		m_initialPaint;
	bool		m_bInitialized;

public:
//	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual void OnInitialUpdate();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
//	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
protected:
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	virtual void OnActivateFrame(UINT nState, CFrameWnd* pDeactivateFrame);
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
protected:
	virtual void OnDraw(CDC* /*pDC*/);
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};


