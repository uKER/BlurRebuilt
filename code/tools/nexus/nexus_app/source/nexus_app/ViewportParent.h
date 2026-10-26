#ifndef NEXUS_VIEWPORTPARENT_H
#define NEXUS_VIEWPORTPARENT_H

#define NEXUS_NUM_VIEWPORTS 4

namespace Nexus
{
	class C_ViewportParent : public CView
	{
	public:
		C_Viewport* GetViewportPtr(U32 index);

	protected: // create from serialization only
		C_ViewportParent();
		DECLARE_DYNCREATE(C_ViewportParent)

	// Operations
	public:
		C_DisplayManager* GetDisplayManagerPtr();

	protected:
	// Overrides
		public:
		virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
		BOOL DataObjectToList(COleDataObject* const pDataObject);
		BOOL ProcessDroppedFile( HDROP hDrop );

	// Implementation
	public:
		virtual ~C_ViewportParent();
	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
	#endif

	// Generated message map functions
	protected:
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		DECLARE_MESSAGE_MAP()
	public:
		virtual void OnInitialUpdate();
		virtual void OnFinalRelease();
		afx_msg void OnPaint();
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

	protected:
		virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	public:
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);

		bool m_initialPaint;

		C_DisplayManager* m_pDisplayManager;

		C_Viewport*	m_viewports[NEXUS_NUM_VIEWPORTS];
	};
}

#endif