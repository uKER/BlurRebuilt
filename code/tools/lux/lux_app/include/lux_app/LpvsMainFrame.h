// LpvsMainFrm.h : interface of the LpvsMainFrame class
//


#pragma once

#include "lux_app/CustomControls/WndSlider.h"
#include "lux_app/CustomControls/FixedWidthToolbar.h"
#include "lux_app/AppSettings.h"

class CPickerDlg;
class CLayerDlg;

class LpvsMainFrame : public CFrameWnd
{

protected: // create from serialization only
	LpvsMainFrame();
	DECLARE_DYNCREATE(LpvsMainFrame)

// Operations
public:

// Overrides
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~LpvsMainFrame();

	void SetDocumentPtr(CDocument* pDocument);

	CDocument* GetDocumentPtr();

	void KeyboardEvent(UINT nChar, UINT nRepCnt, UINT nFlags);
	void MouseMoveEvent(UINT nFlags, CPoint point);
	void MouseButtonEvent(int buttonId, LPVS::LpvsGuiMouseButtonActionEnum, UINT nFlags, CPoint point);
	void Pick();
	void CreateSlidingPaneToolbar();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void Reset();
	void OnMouseButtonStateChanged(int buttonId);
	void SetCmdIcon( UINT nCmd, UINT nIconID );

	S32								m_mouseXOld;
	S32								m_mouseYOld;

	bool							m_keyboardState[256];
	bool							m_mouseButtonState[LPVS::LPVS_MOUSE_BUTTON_COUNT];

	LPVS::LpvsGuiViewportStateEnum	m_viewportState;


public:  // control bar embedded members
	CStatusBar						m_wndStatusBar;
	CExtToolControlBar				m_wndToolBar;

	S32								m_totalRenderViewWidth;
	S32								m_totalRenderViewHeight;

	// prof-uis stuff
	WINDOWPLACEMENT					m_dataFrameWP;
	CExtMenuControlBar				m_wndMenuBar;


	CExtControlBar					m_wndResizableBar1;		// toolbar
	CExtWFF< CListBox >				m_wndInBarListBox;		// output text toolbar

	CFixedWidthToolbar				m_wndResizableBar3;		// toolbar
	CExtWRB< CWndSliderView >		m_wndInBarDlg;			// sliding-panes toolbar

	CPickerDlg						*m_pPickerDlg;
	CLayerDlg						*m_pLayerDlg;

	CAppSettings	m_appSettings;

	void	CreateListToolbar();

	void	SetLightmapMode(int type);
	int		GetLightmapMode();
// Generated message map functions
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLightingLight();
	virtual BOOL DestroyWindow();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
protected:
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
public:
	afx_msg void OnProjectProjectsettings();
	afx_msg void OnUpdateViewRealtimelighting(CCmdUI *pCmdUI);
	afx_msg void OnViewRealtimelighting();
	afx_msg void OnViewBouncePaths();
	afx_msg void OnUpdateViewBouncePaths(CCmdUI *pCmdUI);
	afx_msg void OnLightingDebug();
	afx_msg void OnLightmaptypeFullcolour();
	afx_msg void OnUpdateLightmaptypeFullcolour(CCmdUI *pCmdUI);
	afx_msg void OnLightmaptype2();
	afx_msg void OnUpdateLightmaptype2(CCmdUI *pCmdUI);
	afx_msg void OnLightmaptypeBumpy();
	afx_msg void OnUpdateLightmaptypeBumpy(CCmdUI *pCmdUI);
	afx_msg void OnLightmaptype3();
	afx_msg void OnUpdateLightmaptype3(CCmdUI *pCmdUI);
	afx_msg void OnLightmaptypeMultiplied();
	afx_msg void OnUpdateLightmaptypeMultiplied(CCmdUI *pCmdUI);
	afx_msg void OnLightingUsenetworkrendering();
	afx_msg void OnUpdateLightingUsenetworkrendering(CCmdUI *pCmdUI);
	afx_msg void OnViewShowsamplepoints();
	afx_msg void OnUpdateViewShowsamplepoints(CCmdUI *pCmdUI);
	afx_msg void OnLightmaptypeDebugUnwrapping();
	afx_msg void OnUpdateLightmaptypeDebugUnwrapping(CCmdUI *pCmdUI);
	afx_msg void OnLightmaptypeVertexSH();
	afx_msg void OnUpdateLightmaptypeVertexSH(CCmdUI *pCmdUI);
};

extern LpvsMainFrame *gpMainFrm;;

