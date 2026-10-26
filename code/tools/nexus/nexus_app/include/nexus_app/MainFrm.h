#ifndef NEXUS_MAIN_FRM_H
#define NEXUS_MAIN_FRM_H

namespace Nexus
{
	class DockingPanelRight;
	class DockingPanelBottom;

	class MainFrame : public CFrameWnd
	{
	protected: // create from serialization only
		MainFrame();
		DECLARE_DYNCREATE(MainFrame)

	// Attributes
	public:

	// Operations
	public:

	// Overrides
	public:
		virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	// Implementation
	public:
		virtual ~MainFrame();
	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;

	#endif

		void Update();

		void OnCriticalError(Tu::Status status);

		void OnError(Tu::Status status);

		void OnMessage(Tu::Status status);

		DockingPanelRight* GetDockingPanelRightPtr();

		DockingPanelBottom* GetDockingPanelBottomPtr();

		const DockingPanelRight* GetDockingPanelRightPtr() const;

		const DockingPanelBottom* GetDockingPanelBottomPtr() const;

	public:
		CExtMenuControlBar				m_wndMenuBar;
		CStatusBar						m_wndStatusBar;
		WINDOWPLACEMENT					m_dataFrameWP;
		CExtToolControlBar				m_wndToolBar;
		DockingPanelRight*				m_pDockingPanelRight;
		DockingPanelBottom*				m_pDockingPanelBottom;


	// Generated message map functions
	protected:
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		DECLARE_MESSAGE_MAP()
	public:
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		afx_msg void OnInitMenu(CMenu* pMenu);
		afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
		afx_msg void OnFileImport();
		afx_msg void OnUpdateShadingSmoothshadeall(CCmdUI *pCmdUI);
		afx_msg void OnUpdateShadingWireframeall(CCmdUI *pCmdUI);
		afx_msg void OnShadingPointsall();
		afx_msg void OnUpdateShadingBackfaceculling(CCmdUI *pCmdUI);
		afx_msg void OnShadingTexturing();
		afx_msg void OnUpdateShadingTexturing(CCmdUI *pCmdUI);
		afx_msg void OnShadingWireframeall();
		afx_msg void OnShadingSmoothshadeall();
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
		afx_msg void OnShadingShowuv0();
		afx_msg void OnUpdateShadingShowuv0(CCmdUI *pCmdUI);
		afx_msg void OnShadingShowuv1();
		afx_msg void OnUpdateShadingShowuv1(CCmdUI *pCmdUI);
		afx_msg void OnViewSceneboundingbox();
		afx_msg void OnUpdateViewSceneboundingbox(CCmdUI *pCmdUI);
		afx_msg void OnViewScenegraph();
		afx_msg void OnUpdateViewScenegraph(CCmdUI *pCmdUI);
		afx_msg void OnRenderingVertextangentspace();
		afx_msg void OnUpdateRenderingVertextangentspace(CCmdUI *pCmdUI);
		afx_msg void OnTasksOutputscenebounds();
		afx_msg void OnTasksrepairscene();
		afx_msg void OnTasksCalculatevertextangentspaceforscene();
		afx_msg void OnTasksWeldmeshcomponentsforscene();
		afx_msg void OnTasksWeldmeshpolygonverticesforscene();
		afx_msg void OnUpdateTasksWeldmeshpolygonverticesforscene(CCmdUI *pCmdUI);
		afx_msg void OnUpdateTasksWeldmeshcomponentsforscene(CCmdUI *pCmdUI);
		afx_msg void OnUpdateTasksCalculatevertextangentspaceforscene(CCmdUI *pCmdUI);
		afx_msg void OnUpdateTasksrepairscene(CCmdUI *pCmdUI);
		afx_msg void OnUpdateTasksOutputscenebounds(CCmdUI *pCmdUI);
		afx_msg void OnTasksWeldmesh();
		afx_msg void OnUpdateTasksWeldmesh(CCmdUI *pCmdUI);
		afx_msg void OnTangentspaceUvset0();
		afx_msg void OnUpdateTangentspaceUvset0(CCmdUI *pCmdUI);
		afx_msg void OnTangentspaceUvset1();
		afx_msg void OnUpdateTangentspaceUvset1(CCmdUI *pCmdUI);
		afx_msg void OnTangentspaceUvset2();
		afx_msg void OnUpdateTangentspaceUvset2(CCmdUI *pCmdUI);
		afx_msg void OnTangentspaceUvset3();
		afx_msg void OnUpdateTangentspaceUvset3(CCmdUI *pCmdUI);
		afx_msg void OnTangentspaceUvset4();
		afx_msg void OnUpdateTangentspaceUvset4(CCmdUI *pCmdUI);
		afx_msg void OnViewAnimationdetails();
		afx_msg void OnUpdateViewAnimationdetails(CCmdUI *pCmdUI);
		afx_msg void OnViewPolycount();
		afx_msg void OnUpdateViewPolycount(CCmdUI *pCmdUI);
		afx_msg void OnShadingCropsheet();
		afx_msg void OnUpdateShadingCropsheet(CCmdUI *pCmdUI);
		afx_msg void OnTasksOutputselectionstats();
		afx_msg void OnHeadsupdisplayCameradetails();
		afx_msg void OnTasksRemoveduplicatemeshes();
		afx_msg void OnTasksCreateinstances();
		afx_msg void OnShowArealights();
		afx_msg void OnUpdateShowArealights(CCmdUI *pCmdUI);
		afx_msg void OnUpdateHeadsupdisplayCameradetails(CCmdUI *pCmdUI);
		afx_msg void OnShowLights();
		afx_msg void OnUpdateShowLights(CCmdUI *pCmdUI);
		afx_msg void OnSelectionModeObject();
		afx_msg void OnUpdateSelectionModeObject(CCmdUI *pCmdUI);
		afx_msg void OnSelectionModeFace();
		afx_msg void OnUpdateSelectionModeFace(CCmdUI *pCmdUI);
		afx_msg void OnSelectionModePoint();
		afx_msg void OnUpdateSelectionModePoint(CCmdUI *pCmdUI);
		afx_msg void OnUpdateShowFaceInfoOnSelection(CCmdUI* pCmdUI);
		afx_msg void OnShowFaceInfoOnSelection();
		afx_msg void OnUpdateShowObjectInfoOnSelection(CCmdUI* pCmdUI);
		afx_msg void OnShowObjectInfoOnSelection();

		afx_msg LRESULT UpdateTime(WPARAM wParam, LPARAM lParam);

		void KeyboardEvent(UINT nChar, UINT nRepCnt, UINT nFlags, bool pressed);

	protected:
		virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	public:
		virtual BOOL DestroyWindow();
		virtual BOOL Create(
			LPCTSTR lpszClassName,
			LPCTSTR lpszWindowName, 
			DWORD dwStyle = WS_OVERLAPPEDWINDOW, 
			const RECT& rect = rectDefault, 
			CWnd* pParentWnd = NULL,
			LPCTSTR lpszMenuName = NULL,
			DWORD dwExStyle = 0, 
			CCreateContext* pContext = NULL);
	};

}

#endif
