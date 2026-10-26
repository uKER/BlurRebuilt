// NexusView.h : interface of the NexusView class
//


#pragma once

namespace Nexus
{
	enum NexusMouseButtonEnum
	{
		NEXUS_MOUSE_BUTTON_LEFT = 0,
		NEXUS_MOUSE_BUTTON_RIGHT = 1,
		NEXUS_MOUSE_BUTTON_MIDDLE = 2,

		NEXUS_MOUSE_BUTTON_COUNT
	};

	enum NexusMouseButtonActionEnum
	{
		NEXUS_MOUSE_BUTTON_ACTION_DOWN,
		NEXUS_MOUSE_BUTTON_ACTION_UP,
		NEXUS_MOUSE_BUTTON_ACTION_DOUBLE_CLICK,

		NEXUS_MOUSE_BUTTON_ACTION_COUNT
	};

	enum NexusViewportStateEnum
	{
		NEXUS_VIEWPORT_STATE_NO_ACTION,
		NEXUS_VIEWPORT_STATE_VIEW_TRANSLATE,
		NEXUS_VIEWPORT_STATE_VIEW_ROTATE,
		NEXUS_VIEWPORT_STATE_VIEW_DOLLY,

		NEXUS_VIEWPORT_STATE_COUNT
	};
};

class NexusView : public CView
{
protected: // create from serialization only
	NexusView();
	DECLARE_DYNCREATE(NexusView)

// Attributes
public:
	NexusDoc* GetDocument() const;

// Operations
public:

protected:
// Overrides
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~NexusView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	afx_msg LRESULT UpdateTime(WPARAM wParam, LPARAM lParam);

protected:
	void KeyboardEvent(UINT nChar, UINT nRepCnt, UINT nFlags, bool pressed);
	void MouseMoveEvent(UINT nFlags, CPoint point);
	void MouseButtonEvent(
		int buttonId, 
		Nexus::NexusMouseButtonActionEnum action, 
		UINT nFlags, 
		CPoint point);
	void MouseButtonStateChanged(int buttonId);

	S32	m_mouseXOld;
	S32	m_mouseYOld;

	bool m_keyboardState[256];
	bool m_mouseButtonState[Nexus::NEXUS_MOUSE_BUTTON_COUNT];
	bool m_initialPaint;

	Nexus::NexusViewportStateEnum	m_viewportState;

	Nexus::NexusRender m_nexusRender;
//	NexusProgress m_nexusProgress;

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	virtual void OnFinalRelease();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnUpdateShadingSmoothshadeall(CCmdUI *pCmdUI);
	afx_msg void OnUpdateShadingWireframeall(CCmdUI *pCmdUI);
	afx_msg void OnUpdateShadingFlatshadeall(CCmdUI *pCmdUI);
	afx_msg void OnShadingPointsall();
	afx_msg void OnUpdateShadingBackfaceculling(CCmdUI *pCmdUI);
	afx_msg void OnShadingTexturing();
	afx_msg void OnUpdateShadingTexturing(CCmdUI *pCmdUI);
	afx_msg void OnShadingWireframeall();
	afx_msg void OnShadingSmoothshadeall();
	afx_msg void OnShadingFlatshadeall();
	afx_msg void OnUpdateShadingPointsall(CCmdUI *pCmdUI);
	afx_msg void OnShadingBackfaceculling();
	afx_msg void OnUpdateRenderingVertexnormals(CCmdUI *pCmdUI);
	afx_msg void OnRenderingVertexnormals();
	afx_msg void OnRenderingAxes();
	afx_msg void OnUpdateRenderingAxes(CCmdUI *pCmdUI);
	afx_msg void OnLightingDefaultlighting();
	afx_msg void OnUpdateLightingDefaultlighting(CCmdUI *pCmdUI);
	afx_msg void OnLightmaptypeFullcolour();
	afx_msg void OnUpdateLightmaptypeFullcolour(CCmdUI *pCmdUI);
	afx_msg void OnLightmaptypeMultiplied();
	afx_msg void OnUpdateLightmaptypeMultiplied(CCmdUI *pCmdUI);
	afx_msg void OnUpdateLightmaptype3(CCmdUI *pCmdUI);
	afx_msg void OnLightmaptype3();
	afx_msg void OnLightmaptype2();
	afx_msg void OnUpdateLightmaptype2(CCmdUI *pCmdUI);
	afx_msg void OnShowNulls();
	afx_msg void OnUpdateShowNulls(CCmdUI *pCmdUI);
	afx_msg void OnShowMeshes();
	afx_msg void OnUpdateShowMeshes(CCmdUI *pCmdUI);
	afx_msg void OnShowSplines();
	afx_msg void OnUpdateShowSplines(CCmdUI *pCmdUI);
	afx_msg void OnShowInstances();
	afx_msg void OnUpdateShowInstances(CCmdUI *pCmdUI);
	afx_msg void OnShowPointlights();
	afx_msg void OnUpdateShowPointlights(CCmdUI *pCmdUI);
	afx_msg void OnShowSpotlights();
	afx_msg void OnUpdateShowSpotlights(CCmdUI *pCmdUI);
	afx_msg void OnShowDirectionallights();
	afx_msg void OnUpdateShowDirectionallights(CCmdUI *pCmdUI);
	afx_msg void OnShowJoints();
	afx_msg void OnUpdateShowJoints(CCmdUI *pCmdUI);
	afx_msg void OnViewShowgrid();
	afx_msg void OnUpdateViewShowgrid(CCmdUI *pCmdUI);
	afx_msg void OnShadingShowweighting();
	afx_msg void OnUpdateShadingShowweighting(CCmdUI *pCmdUI);
	afx_msg void OnDebugOutputscenegraph();
	afx_msg void OnShadingFixedfunctionskinning();
	afx_msg void OnUpdateShadingFixedfunctionskinning(CCmdUI *pCmdUI);
	afx_msg void OnShadingShaderskinning();
	afx_msg void OnUpdateShadingShaderskinning(CCmdUI *pCmdUI);
};

#ifndef _DEBUG  // debug version in NexusView.cpp
inline NexusDoc* NexusView::GetDocument() const
   { return reinterpret_cast<NexusDoc*>(m_pDocument); }

#endif

